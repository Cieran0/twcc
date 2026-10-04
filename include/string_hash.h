#pragma once
#include "stdlib.h"

typedef struct string_bucket_node string_bucket_node;

typedef struct string_bucket_node {
    string_bucket_node* next;
    char* content;
    size_t id;
} string_bucket_node;

size_t get_string_id(const char* string);
char* content_of_id(size_t id);