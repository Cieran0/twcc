#include "pre_token_arena.h"
#include "string.h"

void grow(pta* this) {
    const size_t new_cap = this->capacity * 2;
    void* new_start = malloc(new_cap);
    memcpy(new_start, this->start, this->capacity);

    this->start = new_start;
    this->capacity = new_cap;
}

void add_string(pta* this, char* string, int size) {
    const int null_terminated = (string[size-1] == 0);
    if(null_terminated) { size--; }

    void* string_start = NULL;

    if(this->current_size + size+1 <= this->capacity) {
        string_start = this->start + this->current_size;
        memcpy(string_start, string, size);
        *((char*)string_start+size) = 0;

         this->strings_stored++;
         this->current_size += size+1;
    } else {
        grow(this);
        add_string(this, string, size);
    }

}

char* get_string(pta* this, int index) {
    if (index < 0 || index >= this->strings_stored) {
        return NULL;
    }
    
    char* current = (char*)this->start;
    for (int i = 0; i < index; i++) {
        while (*current != '\0') {
            current++;
        }
        current++;
    }
    
    return current;
}

pta new_pre_token_arena(size_t capacity) {
    void* start = malloc(capacity);
    return (pta) {
        start,
        capacity,
        0,
        0,
        add_string,
        get_string
    };
}