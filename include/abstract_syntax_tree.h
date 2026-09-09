#pragma once
#include "token.h"

typedef struct abstract_syntax_tree_node ast_node;

typedef struct abstract_syntax_tree_node {
    token self;
    ast_node** children;
    size_t children_count;
} ast_node;

typedef struct abstract_syntax_tree {
    ast_node** statements;
    size_t statements_count;
} abstract_syntax_tree;