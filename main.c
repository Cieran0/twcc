#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "pre_token_arena.h"
#include "token.h"

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
    const char* type;
    char* name;
} name_type_pair;

typedef struct function {
    name_type_pair signature;
    size_t argc;
    name_type_pair* arguments;
    vector_token code;
} function;

const char* function_return_types[] = {
    "int"
};

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
    
    //TODO: init function


    return NULL;
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
    size_t token_count = preta.strings_stored;
    vector_token tokens = vector_token_new(preta.strings_stored);

    for (size_t i = 0; i < preta.strings_stored; i++)
    {
        char* string = preta.get_string(&preta, i);
        vector_token_push(&tokens, tokenise(string));
        printf("%s\n", string);
    }

    free(preta.start);

    for (size_t i = 0; i < token_count; i++)
    {
        printf("%s", token_type_names[tokens.data[i].type]);
        if (tokens.data[i].content != NULL) {
            printf(": %s", tokens.data[i].content);
        }
        printf("\n");
    }

    function* func_ptr = extract_function(&tokens);
    if(func_ptr == NULL) {
        printf("No Functions Found!\n");
        return INVALID_INPUT_FILE;
    }

    function f = *func_ptr;
    
    return SUCCESS;
}