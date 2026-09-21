#pragma once

#include "string_builder.h"
#include "token.h"
#include "function.h"
#include "abstract_syntax_tree.h"

char* generate_asm_from_function(function func, abstract_syntax_tree ast, symbol_table st);