#pragma once
#include "token.h"
#include "vector_token.h"
#include "types.h"
#include "stdbool.h"

token token_clone(token t);
vector_token tokenise_string(char* file);
builtin_type type_from_type_token(token t);
const bool is_binary_operation(token_type type);