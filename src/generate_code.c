#include "generate_code.h"
#include "stdio.h"
#include "assert.h"
#include "stdbool.h"
#include "string.h"
#include "tokenise.h"

const char* get_var_register(const char* name, function *f) {
    const char* registers[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    for (size_t i = 0; i < f->argc; i++) {
        if (strcmp(name, f->arguments[i].name) == 0) {
            if (i < 6) return registers[i];
        }
    }

    //TODO: support more than 6 input variables
    assert(false);
    return "UNKNOWN_VAR";
}

char* generate_asm_from_expression(ast_node* node, function *f) {
    if (node == NULL) {
        return NULL;
    }

    string_builder sb = string_builder_new(64);
    token_type type = node->self.type;
    
    if (type == TOKEN_NAME) {
        string_builder_append(&sb, "\tmov rax, ");
        string_builder_append(&sb, get_var_register(node->self.content, f));
        string_builder_append(&sb, "\n");
    } 
    else if (is_binary_operation(type)) {
        
        //Put left hand side in RAX
        char* left_side = generate_asm_from_expression(node->children[0], f);
        string_builder_append(&sb, left_side);
        free(left_side);
        
        //Save left side to stack
        string_builder_append(&sb, "\tpush rax\n");
        
        //Put right hand side in RAX
        char* right_side = generate_asm_from_expression(node->children[1], f);
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
    } else {
        printf("Encountered unexpected Token of type: %s\n", token_type_names[type]);
        assert(false);
        return NULL;
    }

    char* string = string_builder_build(&sb);
    string_builder_destroy(&sb);
    return string;
}

char* generate_asm_from_function(function func, abstract_syntax_tree ast, symbol_table st) {
    string_builder sb = string_builder_new(1024);

    //Add function label
    string_builder_append(&sb, ".intel_syntax noprefix\n");
    string_builder_append(&sb, ".global ");
    string_builder_append(&sb, func.signature.name);
    string_builder_append(&sb, "\n");
    string_builder_append(&sb, func.signature.name);
    string_builder_append(&sb, ":\n");

    // Set up stack frame
    string_builder_append(&sb, "\tpush rbp\n");
    string_builder_append(&sb, "\tmov rbp, rsp\n");
    
    for (size_t i = 0; i < ast.statements_count; i++)
    {
        ast_node* statement = ast.statements[i];

        if(statement->self.type == TOKEN_RETURN) {

            if(statement->children_count > 0) {
                char* expression_asm = generate_asm_from_expression(statement->children[0], &func);
                string_builder_append(&sb, expression_asm);
                free(expression_asm);
            } else {
                string_builder_append(&sb, "\txor rax,rax\n");
            }

        } else {
            //TODO: handle different types here
            printf("generate_asm_from_function failed due to not being TOKEN_RETURN");
            assert(false);
        }
    }
    

    // Tear down stack frame
    string_builder_append(&sb, "\tpop rbp\n");
    string_builder_append(&sb, "\tret\n");

    char* function_asm = string_builder_build(&sb);
    string_builder_destroy(&sb);
    return function_asm;
}