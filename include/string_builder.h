#pragma once
#include "stdlib.h"

typedef struct string_builder
{
    char* string;
    size_t size;
    size_t capacity;
} string_builder;

char* clone_str(const char* string);
string_builder string_builder_new(size_t capacity);
void string_builder_grow(string_builder* sb, size_t grow_by_min);
void string_builder_append(string_builder* sb, const char* to_append);
char* string_builder_build(string_builder* sb);
void string_builder_destroy(string_builder* sb);