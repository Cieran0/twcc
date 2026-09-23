#include "vector_token.h"
#include "string.h"

void vector_token_grow(vector_token* this, size_t grow_by_min) {
    size_t new_cap = this->capacity == 0? 1 : this->capacity * 2;
    while (new_cap < this->capacity+grow_by_min)
    {
        new_cap*=2;
    }
    
    token* new_data = (token*)malloc(sizeof(token) * new_cap);

    if(new_data == NULL) {
        return;
    }

    memcpy(new_data, this->data, sizeof(token) * this->size);

    free(this->data);

    this->data = new_data;
    this->capacity = new_cap;
}

void vector_token_push(vector_token* this, token t) {
    if(this->size + 1 >= this->capacity) {
        vector_token_grow(this, 1);
    }

    if(this->size + 1 >= this->capacity) {
        return;
    }

    this->data[this->size] = t;
    this->size++; 
}

void vector_token_pop(vector_token* this) {
    if(this->size <= 0)
        return;
    
    this->size--;
    char* to_free = this->data[this->size].content;
    
    if(to_free == NULL)
        return;

    free(to_free);
    this->data[this->size].content = NULL;
}


vector_token vector_token_new(size_t capacity) {
    token* data = NULL;
    if(capacity > 0) data = (token*)malloc(sizeof(token) * capacity);
    return (vector_token) {
        data,
        0,
        capacity
    };
}

//Remove [start, end] from the vector
void vector_token_remove(vector_token* this, size_t start, size_t end) {
    if (this->size == 0) return;
    if (start >= this->size) return;
    
    if (end >= this->size) {
        end = this->size - 1; 
    }
    if (start > end) return;

    size_t removal_size = (end - start) + 1;

    for (size_t i = start; i <= end; i++) {
        if (this->data[i].content != NULL) {
            free(this->data[i].content);
            this->data[i].content = NULL;
        }
    }

    if (removal_size >= this->size) {
        this->size = 0;
        return;
    }

    size_t elements_to_move = this->size - (end + 1);
    for (size_t i = 0; i < elements_to_move; i++) {
        this->data[start + i] = this->data[end + 1 + i]; 
    }
    
    this->size -= removal_size;
}