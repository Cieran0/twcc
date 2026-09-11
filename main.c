#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "pre_token_arena.h"
#include "token.h"
#include "abstract_syntax_tree.h"
#include "types.h"
#include "symbol_table.h"

enum error_no {
    SUCCESS = 0,
    NO_INPUT_FILE,
    INVALID_INPUT_FILE,
};

char* load_file(const char* filename) {
    FILE *f = fopen(filename, "rb");
    if(!f) return NULL;

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *string = malloc(fsize + 1);
    fread(string, fsize, 1, f);
    fclose(f);

    return string;
}

int is_special_char(char c) {
    const char special_chars[] = "(){},;+";
    const size_t len = sizeof(special_chars) - 1;
    for (size_t i = 0; i < len; i++)
    {
        if (c == special_chars[i]) {
            return 1;
        }
    }
    return 0;
}

pta pre_tokenise(char* file) {
    pta preta = new_pre_token_arena(1024);
    char name_buffer[1024] = {0};
    int buffer_index = 0;

    char c;
    while ((c = *(file++)) != 0 && buffer_index < sizeof(name_buffer))
    {
        if (c == ' ' || c == '\n' || c == '\t') {
            if(name_buffer[0] == 0) continue;
            preta.add_string(&preta, name_buffer, buffer_index);
            memset(name_buffer, 0, buffer_index+1);
            buffer_index = 0;
        } else if (is_special_char(c)) {
            if(name_buffer[0] != 0) {
                preta.add_string(&preta, name_buffer, buffer_index);
                memset(name_buffer, 0, buffer_index+1);
            }
            preta.add_string(&preta, &c, 1);
            buffer_index = 0;
        } else {
            name_buffer[buffer_index] = c;
            buffer_index++;
        }
    }

    if(name_buffer[0] != 0) {
        preta.add_string(&preta, name_buffer, buffer_index);
        memset(name_buffer, 0, buffer_index+1);
    }
    
    return preta;
}

token tokenise_char(char c) {
    switch (c)
    {
    case '(':
        return (token) {TOKEN_OPEN_BRACE, NULL};
    case ')':
        return (token) {TOKEN_CLOSE_BRACE, NULL};
    case ',':
        return (token) {TOKEN_COMMA, NULL};
    case '{':
        return (token) {TOKEN_OPEN_CURLY_BRACE, NULL};
    case '}':
        return (token) {TOKEN_CLOSE_CURLY_BRACE, NULL};
    case '+':
        return (token) {TOKEN_PLUS, NULL};
    case ';':
        return (token) {TOKEN_SEMI_COLON, NULL};
    }

    char* content = malloc(2);
    content[0] = c;
    content[1] = 0;

    return (token){
        TOKEN_NAME,
        content
    };
}

token tokenise(const char* string) {
    size_t len = strlen(string);
    if(len == 1) {
        return tokenise_char(string[0]);
    }

    char* content = malloc(len);
    memcpy(content, string, len + 1);

    size_t type = TOKEN_NAME;

    if(strcmp("int", string) == 0) {
        type = TOKEN_TYPE;
    } else if (strcmp("return", string) == 0) {
        type = TOKEN_RETURN;
    }

    return (token){
        type,
        content
    };
}

typedef struct vector_token {
    token* data;
    size_t size;
    size_t capacity;
} vector_token;

token token_clone(token t) {
    if (t.content == NULL) {
        return t;
    }

    size_t content_size = strlen(t.content);
    token cloned = t;
    
    cloned.content = (char*)malloc(content_size + 1);
    if (cloned.content != NULL) {
        memcpy(cloned.content, t.content, content_size + 1);
    }

    return cloned;
}

void vector_token_grow(vector_token* this, size_t grow_by_min) {
    size_t new_cap = this->capacity == 0? 1 : this->capacity * 2;
    while (new_cap < this->capacity+grow_by_min)
    {
        new_cap*=2;
    }
    
    token* new_data = (token*)malloc(sizeof(token) * new_cap);

    if(new_data == NULL) {
        return;
    }

    memcpy(new_data, this->data, sizeof(token) * this->size);

    free(this->data);

    this->data = new_data;
    this->capacity = new_cap;
}

void vector_token_push(vector_token* this, token t) {
    if(this->size + 1 >= this->capacity) {
        vector_token_grow(this, 1);
    }

    if(this->size + 1 >= this->capacity) {
        return;
    }

    this->data[this->size] = t;
    this->size++; 
}

void vector_token_pop(vector_token* this) {
    if(this->size <= 0)
        return;
    
    this->size--;
    char* to_free = this->data[this->size].content;
    
    if(to_free == NULL)
        return;

    free(to_free);
    this->data[this->size].content = NULL;
}


vector_token vector_token_new(size_t capacity) {
    token* data = (token*)malloc(sizeof(token) * capacity);
    return (vector_token) {
        data,
        0,
        capacity
    };
}

//Remove [start, end] from the vector
void vector_token_remove(vector_token* this, size_t start, size_t end) {
    if (this->size == 0) return;
    if (start >= this->size) return;
    
    if (end >= this->size) {
        end = this->size - 1; 
    }
    if (start > end) return;

    size_t removal_size = (end - start) + 1;

    for (size_t i = start; i <= end; i++) {
        if (this->data[i].content != NULL) {
            free(this->data[i].content);
            this->data[i].content = NULL;
        }
    }

    if (removal_size >= this->size) {
        this->size = 0;
        return;
    }

    size_t elements_to_move = this->size - (end + 1);
    for (size_t i = 0; i < elements_to_move; i++) {
        this->data[start + i] = this->data[end + 1 + i]; 
    }
    
    this->size -= removal_size;
}

typedef struct name_type_pair {
    builtin_type type;
    char* name;
} name_type_pair;

typedef struct function {
    name_type_pair signature;
    size_t argc;
    name_type_pair* arguments;
    vector_token code;
} function;

char* clone_str(const char* string) {
    if(string == NULL)
        return NULL;

    size_t size = strlen(string);
    char* cloned = (char*)malloc(size+1);
    memcpy(cloned, string, size+1);
    return cloned;
}

builtin_type type_from_type_token(token t) {
    //TODO: actually implement this
    return INT;
}

function* extract_function(vector_token* tokens) {

    size_t signature_index = 0;
    size_t argument_start = 0;
    size_t argument_end = 0;
    size_t code_start = 0;
    size_t code_end = 0;
    int found = 0;

    for (size_t i = 0; i < tokens->size -2; i++)
    {
        if(tokens->data[i].type != TOKEN_TYPE) continue;
        if(tokens->data[i+1].type != TOKEN_NAME) continue;
        if(tokens->data[i+2].type != TOKEN_OPEN_BRACE) continue;
        found = 1;
        signature_index = i;
        break;
    }

    if(!found) return NULL;
    argument_start = signature_index+2;
    
    for (size_t i = argument_start+1; i < tokens->size; i++)
    {
        found = 0;
        if(tokens->data[i].type == TOKEN_CLOSE_BRACE) {
            found = 1;
            argument_end = i;
            break;
        }
    }
    
    if(!found) return NULL;
    code_start = argument_end + 1;
    if(code_start >= tokens->size || tokens->data[code_start].type != TOKEN_OPEN_CURLY_BRACE)
        return NULL;

    size_t open_counter = 0;
    for (size_t i = code_start + 1; i < tokens->size; i++)
    {
        found = 0;
        if(tokens->data[i].type == TOKEN_OPEN_CURLY_BRACE) {
            open_counter++;
        } else if (tokens->data[i].type != TOKEN_CLOSE_CURLY_BRACE) {
            continue;
        }

        if (open_counter == 0) {
            code_end = i;
            found = 1;
            break;
        }

        open_counter--;
    }

    if (!found) {
        return NULL;
    }
    

    name_type_pair function_signature = {
        type_from_type_token(tokens->data[signature_index]),
        clone_str(tokens->data[signature_index+1].content)
    };

    size_t argc = 0;
    name_type_pair* args = NULL;
    
    size_t current_idx = argument_start + 1; 
    
    while (current_idx < argument_end) {
        if (tokens->data[current_idx].type != TOKEN_TYPE) break;
        
        if (current_idx + 1 >= argument_end) break;
        if (tokens->data[current_idx + 1].type != TOKEN_NAME) break;
        
        argc++;
        current_idx += 2;
        
        if (current_idx < argument_end) {
            if (tokens->data[current_idx].type == TOKEN_COMMA) {
                current_idx++;
            } else {
                found = 0;
                break;
            }
        }
    }
    
    if(!found) {
        free(function_signature.name);
        return NULL;
    }

    if(argc > 0 ) {
        args = (name_type_pair*)malloc(sizeof(name_type_pair) * argc);
        for (size_t i = 0; i < argc; i++)
        {
            size_t arg_index =  (argument_start+1)+i*3;
            args[i] = (name_type_pair) {
                type_from_type_token(tokens->data[arg_index]),
                clone_str(tokens->data[arg_index+1].content)
            };
        }
        
    }

    vector_token code = vector_token_new((code_end-code_start)+1);

    for (size_t i = code_start+1; i < code_end; i++)
    {
        vector_token_push(&code, token_clone(tokens->data[i]));
    }
    
    function* f = (function*)malloc(sizeof(function));

    *f = (function) {
        function_signature,
        argc,
        args,
        code
    };

    vector_token_remove(tokens, signature_index, code_end);

    return f;
}

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
    if (*index >= end) return NULL;

    token current = code->data[*index];

    if (current.type == TOKEN_NAME) {
        (*index)++;
        return create_node(current);
    }

    // TODO: Add support for stuff other than names
    return NULL;
}

//TODO: multiple expression could happen at once? a+b+c
ast_node* parse_expression(vector_token* code, size_t* index, size_t end) {
    ast_node* left = parse_primary(code, index, end);
    if (!left) return NULL;

    while (*index < end) {
        token op = code->data[*index];

        if (op.type == TOKEN_PLUS) {
            (*index)++;

            ast_node* right = parse_primary(code, index, end);
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
    } 
    else if (current.type == TOKEN_NAME) {
        return parse_expression(code, index, end);
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
    if(node == NULL) return 1;
    
    token self = node->self;
    if(self.type == TOKEN_NAME) {
        symbol* sy = symbol_table_lookup(scope, self.content);
        if(sy == NULL) {
            printf("Undeclared variable: %s found\n", self.content);
            return 1;
        }
    }
    else if (self.type == TOKEN_RETURN) {
        //Check return type matches function return type
    } else if (self.type == TOKEN_PLUS) {
        //Check both are numeric
    }

    int invalid_children = 0;
    for (size_t i = 0; i < node->children_count; i++)
    {
        invalid_children += analyse_ast_node(node->children[i], scope);
    }
    

    return invalid_children;
}

int main(int argc, const char** argv) {

    if(argc < 2) {
        printf("No Input Given!\n");
        return NO_INPUT_FILE;
    }

    char* file = load_file(argv[1]);
    if(!file) {
        printf("Invalid Input File: %s\n", argv[1]);
        return INVALID_INPUT_FILE;
    }
    printf("%s\n", file);

    pta preta = pre_tokenise(file);

    free(file);

    printf("Size: %llu, Cap: %llu\n", preta.current_size, preta.capacity);
    vector_token tokens = vector_token_new(preta.strings_stored);

    for (size_t i = 0; i < preta.strings_stored; i++)
    {
        char* string = preta.get_string(&preta, i);
        vector_token_push(&tokens, tokenise(string));
        printf("%s\n", string);
    }

    free(preta.start);

    // for (size_t i = 0; i < tokens.size; i++)
    // {
    //     printf("%s", token_type_names[tokens.data[i].type]);
    //     if (tokens.data[i].content != NULL) {
    //         printf(": %s", tokens.data[i].content);
    //     }
    //     printf("\n");
    // }

    function* func_ptr = extract_function(&tokens);
    if(func_ptr == NULL) {
        printf("No Functions Found!\n");
        return INVALID_INPUT_FILE;
    }
    function f = *func_ptr;

    printf("Name: %s\n", f.signature.name);
    printf("Return Type: int\n");
    
    printf("Arguments:\n");
    for (size_t i = 0; i < f.argc; i++)
    {
        printf("Name: %s\n", f.arguments[i].name);
        printf("Type: int\n");
    }
    printf("---------\n");

    for (size_t i = 0; i < f.code.size; i++)
    {
        printf("%s", token_type_names[f.code.data[i].type]);
        if (f.code.data[i].content != NULL) {
            printf(": %s", f.code.data[i].content);
        }
        printf("\n");
    }
    
    abstract_syntax_tree ast = parse_ast(&f);

    symbol_table st = symbol_table_new(64);

    for (size_t i = 0; i < f.argc; i++)
    {
        name_type_pair arg = f.arguments[i];
        symbol_table_add(&st, arg.name, arg.type);
    }
    
    int invalid_nodes = 0;
    for (size_t i = 0; i < ast.statements_count; i++)
    {
        invalid_nodes += analyse_ast_node(ast.statements[i], &st);
    }

    if(invalid_nodes) {
        printf("Found %d invalid nodes\n", invalid_nodes);
        return INVALID_INPUT_FILE;
    }
    
    printf("Semantic type check passed\n");

    return SUCCESS;
}