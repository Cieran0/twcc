#include "tokenise.h"
#include "vector_token.h"
#include "pre_token_arena.h"
#include "stdio.h"
#include "string.h"

int is_special_char(char c) {
    const char special_chars[] = "(){},;+*-/=";
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

bool is_digit(char c) {
    return !(c < '0' || c > '9');
}

static const token_type char_to_token[256] = {
    ['('] = TOKEN_OPEN_BRACE,
    [')'] = TOKEN_CLOSE_BRACE,
    [','] = TOKEN_COMMA,
    ['{'] = TOKEN_OPEN_CURLY_BRACE,
    ['}'] = TOKEN_CLOSE_CURLY_BRACE,
    ['+'] = TOKEN_PLUS,
    [';'] = TOKEN_SEMI_COLON,
    ['*'] = TOKEN_STAR,
    ['-'] = TOKEN_MINUS,
    ['/'] = TOKEN_DIV,
    ['='] = TOKEN_EQUALS,
};

token tokenise_char(char c) {
    unsigned char uc = (unsigned char)c;
    token_type type;
    
    if (char_to_token[uc] != 0) {
        type = char_to_token[uc];
    }
    else if (is_digit(c)) {
        type = TOKEN_NUM;
    }
    else {
        type = TOKEN_NAME;
    }

    char* content = malloc(2);
    content[0] = c;
    content[1] = '\0';

    return (token){type, content};
}

bool is_number(const char* string, size_t len) {
    if (!(string[0] == '-' || is_digit(string[0]))) {
        return false;
    }

    for (size_t i = 1; i < len; i++)
    {
        if(!is_digit(string[i])) return false;
    }

    return true;
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
    } else if (is_number(string, len)) {
        type = TOKEN_NUM;
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