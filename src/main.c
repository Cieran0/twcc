#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "stdbool.h"

#include "pre_token_arena.h"
#include "token.h"
#include "abstract_syntax_tree.h"
#include "types.h"
#include "symbol_table.h"
#include "file.h"
#include "vector_token.h"
#include "tokenise.h"
#include "function.h"
#include "string_builder.h"
#include "generate_code.h"

#include "assert.h"

enum error_no {
    SUCCESS = 0,
    NO_INPUT_FILE,
    INVALID_INPUT_FILE,
};


int main(int argc, const char** argv) {

    if(argc < 2) {
        printf("No Input Given!\n");
        return NO_INPUT_FILE;
    }

    const char* output_name = "out.s";
    if (argc >= 3)
    {
        output_name = argv[2];
    }

    char* file = load_file(argv[1]);
    if(!file) {
        printf("Invalid Input File: %s\n", argv[1]);
        return INVALID_INPUT_FILE;
    }
    printf("%s\n", file);

    vector_token tokens = tokenise_string(file);

    free(file);

    // for (size_t i = 0; i < tokens.size; i++)
    // {
    //     printf("%s", token_type_names[tokens.data[i].type]);
    //     if (tokens.data[i].content != NULL) {
    //         printf(": %s", tokens.data[i].content);
    //     }
    //     printf("\n");
    // }

    function* func_ptr = extract_function(&tokens);
    string_builder sb = string_builder_new(1024);
    string_builder_append(&sb, ".intel_syntax noprefix\n");

    int func_count = 0;

    while (func_ptr != NULL)
    {
        func_count++;
        function f = *func_ptr;

        printf("Name: %s\n", f.signature.name);
        printf("Return Type: int\n");
        
        printf("Arguments:\n");
        for (size_t i = 0; i < f.argc; i++)
        {
            printf("Name: %s\n", f.arguments[i].name);
            printf("Type: int\n");
        }
        printf("---------\n");

        for (size_t i = 0; i < f.code.size; i++)
        {
            printf("%s", token_type_names[f.code.data[i].type]);
            if (f.code.data[i].content != NULL) {
                printf(": %s", f.code.data[i].content);
            }
            printf("\n");
        }
        
        abstract_syntax_tree ast = parse_ast(&f);

        print_ast(&ast);

        symbol_table st = symbol_table_new(64);

        for (size_t i = 0; i < f.argc; i++)
        {
            name_type_pair arg = f.arguments[i];
            symbol_table_add(&st, arg.name, arg.type);
        }
        
        int invalid_nodes = 0;
        for (size_t i = 0; i < ast.statements_count; i++)
        {
            invalid_nodes += analyse_ast_node(ast.statements[i], &st);
        }

        if(invalid_nodes) {
            printf("Found %d invalid nodes\n", invalid_nodes);
            return INVALID_INPUT_FILE;
        }
        
        printf("Semantic type check passed\n");

        printf("Function Code\n--------------\n");
        char* function_code = generate_asm_from_function(f, ast, st);
        printf("%s", function_code);
        printf("--------------\n");

        string_builder_append(&sb, function_code);
        free(function_code);

        func_ptr = extract_function(&tokens);
    }

    if(func_count == 0) {
        printf("No functions found!\n");
        return INVALID_INPUT_FILE;
    }

    char* generated_code = string_builder_build(&sb);

    write_to_file(output_name, generated_code);

    free(generated_code);
    string_builder_destroy(&sb);

    return SUCCESS;
}