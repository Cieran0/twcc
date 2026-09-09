#pragma once
#include "stdlib.h"

#define TOKEN_LIST \
    X(TOKEN_TYPE) \
    X(TOKEN_NAME) \
    X(TOKEN_OPEN_BRACE) \
    X(TOKEN_CLOSE_BRACE) \
    X(TOKEN_COMMA) \
    X(TOKEN_OPEN_CURLY_BRACE) \
    X(TOKEN_CLOSE_CURLY_BRACE) \
    X(TOKEN_RETURN) \
    X(TOKEN_PLUS) \
    X(TOKEN_SEMI_COLON)

// Generate the Enum
typedef enum token_type {
    #define X(name) name,
    TOKEN_LIST
    #undef X
} token_type;

// Generate the String Array
static const char* token_type_names[] = {
    #define X(name) #name,
    TOKEN_LIST
    #undef X
};

typedef struct token
{
    size_t type;
    char* content;
} token;
