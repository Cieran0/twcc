#include "string_builder.h"
#include "string.h"

char* clone_str(const char* string) {
    if(string == NULL)
        return NULL;

    size_t size = strlen(string);
    char* cloned = (char*)malloc(size+1);
    memcpy(cloned, string, size+1);
    return cloned;
}

string_builder string_builder_new(size_t capacity) {
    char* string = capacity <= 0? NULL : (char*)malloc(capacity);

    return (string_builder){
        .string = string,
        .size = 0,
        .capacity = capacity
    };
}

void string_builder_grow(string_builder* sb, size_t grow_by_min) {
    if(grow_by_min <= 0) return;
    if(sb == NULL) return;
    
    size_t new_capacity = sb->capacity > 0 ? sb->capacity*2 : 1;
    while (new_capacity < sb->capacity + grow_by_min)
    {
        new_capacity *= 2;
    }
    
    char* new_string = (char*)malloc(new_capacity);
    memcpy(new_string, sb->string, sb->size);
    free(sb->string);
    sb->string = new_string;
    sb->capacity = new_capacity;
}

void string_builder_append(string_builder* sb, const char* to_append) {
    if(to_append == NULL) return;

    size_t size = strlen(to_append);
    
    if (sb->size + size + 1 > sb->capacity) {
        string_builder_grow(sb, size + 1);
    }

    memcpy(sb->string + sb->size, to_append, size);
    sb->size += size;
}

char* string_builder_build(string_builder* sb) {
    char* string = (char*)malloc(sb->size+1);
    memcpy(string, sb->string, sb->size);
    string[sb->size] = '\0';
    return string;
}

void string_builder_destroy(string_builder* sb) {
    sb->capacity = 0;
    sb->size = 0;
    free(sb->string);
    sb->string = NULL;
}