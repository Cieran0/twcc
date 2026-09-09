#pragma once
#include "types.h"
#include "stdlib.h"

typedef struct symbol
{
    char* name;
    builtin_type type;
} symbol;

typedef struct symbol_table symbol_table;

struct symbol_table
{
    symbol* table;
    size_t size;
    size_t capacity;
    symbol_table* parent;
};

symbol_table symbol_table_new(size_t capacity);

void symbol_table_add(symbol_table* table, const char* name, builtin_type type);

symbol* symbol_table_lookup(symbol_table* table, const char* name);