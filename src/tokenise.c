#include "tokenise.h"
#include "vector_token.h"
#include "pre_token_arena.h"
#include "stdio.h"
#include "string.h"

int is_special_char(char c) {
    const char special_chars[] = "(){},;+*-/";
    const size_t len = sizeof(special_chars) - 1;
    for (size_t i = 0; i < len; i++)
    {
        if (c == special_chars[i]) {
            return 1;
        }
    }
    return 0;
}

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
    case '*':
        return (token) {TOKEN_STAR, NULL};
    case '-':
        return (token) {TOKEN_MINUS, NULL};
    case '/':
        return (token) {TOKEN_DIV, NULL};
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

vector_token tokenise_string(char* file) {
    pta preta = pre_tokenise(file);

    printf("Size: %llu, Cap: %llu\n", preta.current_size, preta.capacity);
    vector_token tokens = vector_token_new(preta.strings_stored);
    
    for (size_t i = 0; i < preta.strings_stored; i++)
    {
        char* string = preta.get_string(&preta, i);
        vector_token_push(&tokens, tokenise(string));
        printf("%s\n", string);
    }
    
    free(preta.start);

    return tokens;
}

builtin_type type_from_type_token(token t) {
    //TODO: actually implement this
    return INT;
}

const bool is_binary_operation(token_type type) {
    switch (type)
    {
    case TOKEN_PLUS:
    case TOKEN_MINUS:
    case TOKEN_STAR:
    case TOKEN_DIV:
        return true;
    
    default:
        return false;
    }
    
}