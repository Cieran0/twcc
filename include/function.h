#pragma once
#include "vector_token.h"
#include "types.h"


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

function* extract_function(vector_token* tokens);