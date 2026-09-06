#pragma once
#include "stdlib.h"

typedef struct pre_token_arena {
    void* start;
    size_t capacity;
    size_t current_size;
    size_t strings_stored;
    void (*add_string)(struct pre_token_arena*, char* string, int size);
    char* (*get_string)(struct pre_token_arena*, int index);
} pta;

pta new_pre_token_arena(size_t capacity);