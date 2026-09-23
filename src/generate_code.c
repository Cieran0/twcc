#include "generate_code.h"
#include "stdio.h"
#include "assert.h"
#include "stdbool.h"
#include "string.h"
#include "tokenise.h"

typedef struct local_var {
    const char* name;
    int stack_offset;
    char* location;
} local_var;

typedef struct local_vars {
    local_var* vars;
    size_t size;
    size_t capacity;
} local_vars;

const char* registers[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

const char* add_local_var(const char* name, local_vars* vars) {
    if(vars->capacity == vars->size) return NULL;

    int stack_offset = (vars->size + 1) * 8;
    local_var var = (local_var) {
        .name = name,
        .stack_offset = stack_offset,
        .location = (char*)malloc(32),
    };

    snprintf(var.location, 32, "[rbp-%d]", stack_offset);

    vars->vars[vars->size] = var;
    vars->size++;
    return var.location;
}

const char* get_var_location(const char* name, function *f, local_vars* vars) {
    for (size_t i = 0; i < f->argc; i++) {
        if (strcmp(name, f->arguments[i].name) == 0) {
            if( i >= 6 ) 
                assert(false);

            if (i < 6) return registers[i];
        }
    }

    for (size_t i = 0; i < vars->size; i++)
    {
        if (strcmp(name, vars->vars[i].name) == 0) {
            return vars->vars[i].location;
        }
    }
    
    const char* location = add_local_var(name, vars);
    assert(location != NULL);
    return location;
}

char* generate_asm_from_expression(ast_node* node, function *f, local_vars* vars) {
    if (node == NULL) {
        return NULL;
    }

    string_builder sb = string_builder_new(64);
    token_type type = node->self.type;
    
    if (type == TOKEN_NAME) {
        string_builder_append(&sb, "\tmov rax, ");
        string_builder_append(&sb, get_var_location(node->self.content, f, vars));
        string_builder_append(&sb, "\n");
    } 
    else if (is_binary_operation(type)) {
        
        //Put left hand side in RAX
        char* left_side = generate_asm_from_expression(node->children[0], f, vars);
        string_builder_append(&sb, left_side);
        free(left_side);
        
        //Save left side to stack
        string_builder_append(&sb, "\tpush rax\n");
        
        //Put right hand side in RAX
        char* right_side = generate_asm_from_expression(node->children[1], f, vars);
        string_builder_append(&sb, right_side);
        free(right_side);
        
        //Pop the left side into rcx
        string_builder_append(&sb, "\tmov rcx, rax\n");
        string_builder_append(&sb, "\tpop rax\n");
        
        //Add them together
        if(type == TOKEN_PLUS) {
            string_builder_append(&sb, "\tadd rax, rcx\n");
        } else if (type == TOKEN_STAR) {
            string_builder_append(&sb, "\timul rax, rcx\n");
        } else if (type == TOKEN_MINUS) {
            string_builder_append(&sb, "\tsub rax, rcx\n");
        } else if (type == TOKEN_DIV) {
            string_builder_append(&sb, "\tcqo\n");
            string_builder_append(&sb, "\tidiv rcx\n");
        } else {
            assert(false && type);
        }
    } else if (type == TOKEN_FUNCTION_CALL) {
        size_t argc = node->children_count;

        for (int i = argc - 1 ; i >= 0; i--)
        {
            char* argument_asm = generate_asm_from_expression(node->children[i], f, vars);
            string_builder_append(&sb, argument_asm);
            free(argument_asm);

            string_builder_append(&sb, "\tpush rax\n");
        }
        
        for (size_t i = 0; i < argc; i++)
        {
            string_builder_append(&sb, "\tpop ");
            string_builder_append(&sb, registers[i]);
            string_builder_append(&sb, "\n");
        }
        
        string_builder_append(&sb, "\tcall ");
        string_builder_append(&sb, node->self.content);
        string_builder_append(&sb, "\n");

    } else if (type == TOKEN_NUM) {
        string_builder_append(&sb, "\tmov rax, ");
        string_builder_append(&sb, node->self.content);
        string_builder_append(&sb, "\n");
    } else if (type == TOKEN_TYPE) {
        //Assign name to location 
        ast_node* name_node = node->children[0];
        const char* var_location = get_var_location(name_node->self.content, f, vars);
        if(node->children_count > 1) {
            ast_node* equals_node = node->children[1];
            assert(equals_node->self.type == TOKEN_EQUALS);
            char* expr_asm = generate_asm_from_expression(equals_node->children[0], f, vars);
            string_builder_append(&sb, expr_asm);
            free(expr_asm);
            string_builder_append(&sb, "\tmov ");
            string_builder_append(&sb, var_location);
            string_builder_append(&sb, ", rax\n");
        }
    }
    else {
        printf("Encountered unexpected Token of type: %s\n", token_type_names[type]);
        assert(false);
        return NULL;
    }

    char* string = string_builder_build(&sb);
    string_builder_destroy(&sb);
    return string;
}



char* generate_asm_from_function(function func, abstract_syntax_tree ast, symbol_table st) {
    if(func.code.size == 0) {
        return NULL;
    }

    string_builder sb = string_builder_new(1024);

    //Add function label
    string_builder_append(&sb, ".global ");
    string_builder_append(&sb, func.signature.name);
    string_builder_append(&sb, "\n");
    string_builder_append(&sb, func.signature.name);
    string_builder_append(&sb, ":\n");

    // Set up stack frame
    string_builder_append(&sb, "\tpush rbp\n");
    string_builder_append(&sb, "\tmov rbp, rsp\n");

    int local_variable_count = st.size - func.argc;
    int space_to_reserve = local_variable_count*8;
    char space_to_reserve_str[32];

    if( local_variable_count > 0 ) {
        string_builder_append(&sb, "\tsub rsp, ");
        snprintf(space_to_reserve_str, 256, "%d", space_to_reserve);
        string_builder_append(&sb, space_to_reserve_str);
        string_builder_append(&sb, "\n");
    } 


    local_vars vars = (local_vars) {
        .vars = malloc(sizeof(local_var) * local_variable_count),
        .size = 0,
        .capacity = local_variable_count
    };

    for (size_t i = 0; i < ast.statements_count; i++)
    {
        ast_node* statement = ast.statements[i];

        if(statement->self.type == TOKEN_RETURN) {

            if(statement->children_count > 0) {
                char* expression_asm = generate_asm_from_expression(statement->children[0], &func, &vars);
                string_builder_append(&sb, expression_asm);
                free(expression_asm);
            } else {
                string_builder_append(&sb, "\txor rax,rax\n");
            }

        } else {
            char* expression_asm = generate_asm_from_expression(statement, &func, &vars);
            string_builder_append(&sb, expression_asm);
            free(expression_asm);
        } 
    }
    

    // Tear down stack frame
    if( local_variable_count > 0 ) {
        string_builder_append(&sb, "\tadd rsp, ");
        string_builder_append(&sb, space_to_reserve_str);
        string_builder_append(&sb, "\n");
    }
     
    string_builder_append(&sb, "\tpop rbp\n");
    string_builder_append(&sb, "\tret\n");

    char* function_asm = string_builder_build(&sb);
    string_builder_destroy(&sb);
    return function_asm;
}