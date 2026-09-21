#pragma once
#include "token.h"

typedef struct vector_token {
    token* data;
    size_t size;
    size_t capacity;
} vector_token;

void vector_token_grow(vector_token* this, size_t grow_by_min);
void vector_token_push(vector_token* this, token t);
void vector_token_pop(vector_token* this);
vector_token vector_token_new(size_t capacity);
void vector_token_remove(vector_token* this, size_t start, size_t end);