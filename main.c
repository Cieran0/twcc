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

//TODO: implement vector of tokens
void vector_token_push();
void vector_token_pop();
void vector_token_new();
//Remove multiple from anywhere in vector
void vector_token_remove();
//Add multiple to anywhere in vector
void vector_token_insert();

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
    token* tokens = malloc(sizeof(token)*token_count);

    for (size_t i = 0; i < preta.strings_stored; i++)
    {
        char* string = preta.get_string(&preta, i);
        tokens[i] = tokenise(string);
        printf("%s\n", string);
    }

    free(preta.start);

    for (size_t i = 0; i < token_count; i++)
    {
        printf("%s", token_type_names[tokens[i].type]);
        if (tokens[i].content != NULL) {
            printf(": %s", tokens[i].content);
        }
        printf("\n");
    }
    
    return SUCCESS;
}