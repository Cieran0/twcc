#include "abstract_syntax_tree.h"
#include "vector_token.h"
#include "function.h"
#include "tokenise.h"
#include "stdio.h"
#include "stdbool.h"
#include "assert.h"

ast_node* create_node(token t) {
    ast_node* node = (ast_node*)malloc(sizeof(ast_node));
    node->self = token_clone(t);
    node->children = NULL;
    node->children_count = 0;
    return node;
}

void add_child(ast_node* parent, ast_node* child) {
    if (!parent || !child) return;
    parent->children_count++;
    parent->children = (ast_node**)realloc(parent->children, sizeof(ast_node*) * parent->children_count);
    parent->children[parent->children_count - 1] = child;
}

ast_node* parse_primary(vector_token* code, size_t* index, size_t end) {
    
    if (*index >= end) {
        return NULL;
    }

    token current = code->data[*index];


    if (current.type == TOKEN_NAME) {
        (*index)++;
        
        if(*index < end) {
            if(code->data[*index].type == TOKEN_OPEN_BRACE) {
                current.type = TOKEN_FUNCTION_CALL;            
            }
        }

        return create_node(current);
    } else if (current.type == TOKEN_NUM) {
        (*index)++;
        return create_node(current);
    }

    printf("Encountered Unexpected Type: %s\n", token_type_names[current.type]);
    assert(false);
    return NULL;
}

void print_ast_node(ast_node* node, size_t depth) {
    for (size_t i = 0; i < depth; i++)
    {
        printf("  ");
    }
    if(depth > 0) {
        printf("|-");
    }
    printf("%s\n", node->self.type == TOKEN_NAME ? node->self.content : token_type_names[node->self.type]);
    for (size_t i = 0; i < node->children_count; i++)
    {
        print_ast_node(node->children[i], depth+1);
    }
    
}

void print_ast(abstract_syntax_tree* ast) {
    printf("AST Statements\n-----------\n");
    for (size_t i = 0; i < ast->statements_count; i++)
    {
        printf("Statement %d:\n-----------\n", i+1);
        print_ast_node(ast->statements[i], 0);
        printf("-----------\n");
    }
}

ast_node* parse_expression(vector_token* code, size_t* index, size_t end) {
    
    ast_node* left = parse_primary(code, index, end);
    if (!left) {
        return NULL;
    }

    if (left->self.type == TOKEN_FUNCTION_CALL) {
        while (*index < end)
        {
            token tk = code->data[*index];
            
            if(tk.type == TOKEN_OPEN_BRACE) {
                (*index)++;
                continue;
            } else if (tk.type == TOKEN_CLOSE_BRACE) {
                (*index)++;
                break;
            } else if (tk.type == TOKEN_COMMA) {
                (*index)++;
                continue;
            }
            ast_node* child = parse_expression(code, index, end);
            add_child(left, child);
        }
    }

    while (*index < end) {
        token op = code->data[*index];

        if (is_binary_operation(op.type)) {
            (*index)++;

            ast_node* right = parse_expression(code, index, end);
            if (!right) break;

            ast_node* bin_op = create_node(op);
            add_child(bin_op, left);
            add_child(bin_op, right);

            left = bin_op;
        } else {
            //Left is a primary?
            break;
        }
    }

    return left;
}

ast_node* parse_statement(vector_token* code, size_t* index, size_t end) {
    if (*index >= end) return NULL;

    token current = code->data[*index];

    if (current.type == TOKEN_RETURN) {
        ast_node* return_node = create_node(current);
        (*index)++;

        if (*index < end && code->data[*index].type != TOKEN_SEMI_COLON) {
            ast_node* expr = parse_expression(code, index, end);
            if (expr) {
                add_child(return_node, expr);
            }
        }
        return return_node;
    }  else if (current.type == TOKEN_TYPE) {
        //dec variable
        ast_node* declare_node = create_node(current);
        (*index)++;

        assert(code->data[*index].type == TOKEN_NAME);
        add_child(declare_node, create_node(code->data[*index]));

        (*index)++;
        if (*index < end && code->data[*index].type != TOKEN_SEMI_COLON) {
            assert(code->data[*index].type == TOKEN_EQUALS);

            ast_node* assignment = create_node(code->data[*index]);
            (*index)++;

            ast_node* expr = parse_expression(code, index, end);
            if (expr) {
                add_child(assignment, expr);
            }
            
            add_child(declare_node, assignment);
        }
        return declare_node;
    } else if (current.type == TOKEN_NAME) {
        ast_node* expr = parse_expression(code, index, end);
        return expr;
    }

    //Fallback
    (*index)++;
    return create_node(current);
}


abstract_syntax_tree parse_ast(function* f) {
    abstract_syntax_tree ast = {0};
    ast.statements = NULL;
    ast.statements_count = 0;

    vector_token* code = &(f->code);
    size_t index = 0;
    size_t end = code->size;

    while (index < end) {
        if (code->data[index].type == TOKEN_SEMI_COLON) {
            index++;
            continue;
        }

        ast_node* stmt = parse_statement(code, &index, end);
        
        if (stmt) {
            ast.statements_count++;
            ast.statements = (ast_node**)realloc(ast.statements, sizeof(ast_node*) * ast.statements_count);
            ast.statements[ast.statements_count - 1] = stmt;
        }

        // Consume the semicolon that ends the statement
        if (index < end && code->data[index].type == TOKEN_SEMI_COLON) {
            index++;
        }
    }

    return ast;
}

int analyse_ast_node(ast_node* node, symbol_table* scope) {
    if(node == NULL) return 0;
    
    token self = node->self;
    if(self.type == TOKEN_NAME) {
        symbol* sy = symbol_table_lookup(scope, self.content);
        if(sy == NULL) {
            printf("Undeclared variable: %s found\n", self.content);
            return 1;
        }
    } else if (self.type == TOKEN_FUNCTION_CALL) {
        //Check if signature is correct
        symbol* sy = symbol_table_lookup(scope, self.content);
        if(sy == NULL) {
            printf("Undeclared function: %s found\n", self.content);
            return 1;
        }
    }
    else if (self.type == TOKEN_RETURN) {
        //Check return type matches function return type
    } else if (self.type == TOKEN_PLUS) {
        //Check both are numeric
    } else if (self.type == TOKEN_TYPE) {
        if (node->children_count == 0) {
            printf("Variable declaration without name\n");
            return 1;
        }
        ast_node* child = node->children[0];
        if(child->self.type != TOKEN_NAME) {
            printf("Variable declaration without name\n");
            return 1;
        }

        symbol* sy = symbol_table_lookup(scope, child->self.content);
        if(sy != NULL) {
            printf("Redeclaration of variable %s\n", child->self.content);
            return 1;
        }

        symbol_table_add(scope, child->self.content, INT);
    }

    int invalid_children = 0;
    for (size_t i = 0; i < node->children_count; i++)
    {
        invalid_children += analyse_ast_node(node->children[i], scope);
    }
    

    return invalid_children;
}