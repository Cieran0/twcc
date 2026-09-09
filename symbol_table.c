#include "symbol_table.h"
#include "string.h"

symbol_table symbol_table_new(size_t capacity) {
    return (symbol_table) {
        .table = capacity > 0? malloc(sizeof(symbol)*capacity) : NULL,
        .size = 0,
        .capacity = capacity,
        .parent = NULL
    };
}

void symbol_table_grow(symbol_table* table, size_t grow_by_min) {
    if(table == NULL) return;
    if(grow_by_min <= 0) return;
    size_t new_capacity = table->capacity > 0 ? table->capacity*2 : 1;

    while (new_capacity < (table->capacity + grow_by_min))
    {
        new_capacity*=2;
    }

    symbol* old_table = table->table;
    
    table->capacity = new_capacity;
    table->table = malloc(sizeof(symbol)*new_capacity);

    if(old_table == NULL) {
        return;
    }

    memcpy(table->table, old_table, sizeof(symbol) * table->size);
    free(old_table);
}

char* clone_name(const char* name) {
    if(name == NULL) return NULL;

    size_t size = strlen(name)+1;
    char* cloned = (char*)malloc(size);
    memcpy(cloned, name, size);
    return cloned;
}

void symbol_table_add(symbol_table* table, const char* name, builtin_type type) {
    if(table == NULL) return;

    if(table->size + 1 > table->capacity) {
        symbol_table_grow(table, 1);
    }

    table->table[table->size] = (symbol) {
        .name = clone_name(name),
        .type = type
    };

    table->size++;
}

symbol* symbol_table_lookup(symbol_table* table, const char* name) {
    if(table == NULL) return NULL;
    
    for (size_t i = 0; i < table->size; i++)
    {
        if(strcmp(table->table[i].name, name) == 0) {
            return &(table->table[i]);
        }
    }

    if(table->parent == NULL) {
        return NULL;
    }
    
    return symbol_table_lookup(table->parent, name);
}