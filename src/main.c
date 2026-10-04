#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "stdbool.h"
#include "assert.h"

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
#include "pre_process.h"
#include "string_hash.h"


typedef enum error_no {
    SUCCESS = 0,
    NO_INPUT_FILE,
    INVALID_INPUT_FILE,
    MALFORMED_FLAG,
} error_no;

typedef struct arguments {
    const char** inputs;
    size_t inputs_size;
    const char* output;
    const char** includes;
    size_t includes_size;
    error_no err;
} arguments;

arguments parse_args(int argc, const char** argv) {

    arguments a = (arguments) {
        .inputs = malloc(sizeof(const char*) * argc),
        .inputs_size = 0,
        .output = NULL,
        .includes = malloc(sizeof(const char*) * argc),
        .includes_size = 0,
        .err = SUCCESS
    };

    for (size_t i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-o") == 0)
        {
            if(a.output != NULL) {
                printf("Flag '-o' is duplicated\n");
                a.err = MALFORMED_FLAG;
                return a;
            }

            if(i+1 >= argc) {
                printf("Flag '-o' expects filename aftewards\n");
                a.err = MALFORMED_FLAG;
                return a;
            }
            a.output = argv[i+1];
            i++;
            continue;
        }

        if (strncmp(argv[i], "-I", 2) == 0) {
            if(strlen(argv[i]) == 2) {
                if(i+1 >= argc) {
                    printf("Flag '-I' expects directory aftewards\n");
                    a.err = MALFORMED_FLAG;
                    return a;
                }
                a.includes[a.includes_size] = argv[i+1];
                a.includes_size++;
                i++;
                continue;
            }

            const char* directory = argv[i]+2;
            a.includes[a.includes_size] = directory;
            a.includes_size++;
        }
        
        a.inputs[a.inputs_size] = argv[i];
        a.inputs_size++; 
    }
    return a;
}


char un_escape_char(char c) {
    
    switch (c)
    {
    case '\a': 
        return 'a'; 
    case '\b': 
        return 'b'; 
    case '\f': 
        return 'f'; 
    case '\n': 
        return 'n'; 
    case '\r': 
        return 'r'; 
    case '\t': 
        return 't'; 
    case '\v': 
        return 'v'; 
    case '\"': 
        return '\"'; 
    case '\'': 
        return '\''; 
    case '\?': 
        return '\?'; 
    case '\\':
        return '\\';
    
    default:
        break;
    }
    return 0;
}

char* un_escape_string(const char* string) {

    size_t len = strlen(string);
    char* new = malloc(len*2);
    size_t j = 0;
    for (size_t i = 0; i < len; i++)
    {
        char c = string[i];
        char un_e_c = un_escape_char(c);
        if(un_e_c == 0) {
            new[j] = string[i];
            j++;
        } else {
            new[j] = '\\';
            new[j+1] = un_e_c;
            j+=2;
        }
    }
    new[j] = 0;
    return new;
}

int main(int argc, const char** argv) {

    arguments args = parse_args(argc, argv);

    if(args.err != SUCCESS) {
        free(args.includes);
        free(args.inputs);
        return args.err;
    }

    if(args.inputs_size == 0) {
        free(args.includes);
        free(args.inputs);
        printf("No Input Given!\n");
        return NO_INPUT_FILE;
    }

    if(args.output == NULL) {
        args.output = "out.s";
    }

    char* file = load_file(args.inputs[0]);
    if(!file) {
        printf("Invalid Input File: %s\n", args.inputs[0]);
        free(args.includes);
        free(args.inputs);
        return INVALID_INPUT_FILE;
    }
    printf("%s\n", file);

    char* pre_processed_file = pre_process(file, args.includes, args.includes_size);
    free(file);

    if(pre_processed_file == NULL) {
        return INVALID_INPUT_FILE;
    }

    vector_token tokens = tokenise_string(pre_processed_file);
    free(pre_processed_file);

    function* func_ptr = extract_function(&tokens);
    string_builder sb = string_builder_new(1024);
    string_builder_append(&sb, ".intel_syntax noprefix\n\n.section .text\n\n");

    symbol_table global = symbol_table_new(128, NULL);

    int func_count = 0;

    while (func_ptr != NULL)
    {
        func_count++;
        function f = *func_ptr;

        symbol_table_add(&global, f.signature.name, f.signature.type);

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

        symbol_table st = symbol_table_new(64, &global);
        

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
        free(args.includes);
        free(args.inputs);
        return INVALID_INPUT_FILE;
    }

    string_builder_append(&sb, "\n.section .rodata\n\n");
    size_t id = 1;
    char* content;
    while ((content = content_of_id(id)) != NULL)
    {
        string_builder_append(&sb, ".LC");
        char buff[32];
        snprintf(buff, 32, "%zu", id);
        string_builder_append(&sb, buff);
        string_builder_append(&sb, ":\n\t.string \"");
        char* unescaped_content = un_escape_string(content);
        string_builder_append(&sb, unescaped_content);
        string_builder_append(&sb, "\"\n\n");
        free(unescaped_content);
        id++;
    }
    
    

    char* generated_code = string_builder_build(&sb);

    write_to_file(args.output, generated_code);
    printf("----------------ASM------------------\n");
    printf("%s", generated_code);
    printf("----------------ASM------------------\n");

    free(generated_code);
    string_builder_destroy(&sb);

    free(args.includes);
    free(args.inputs);
    return SUCCESS;
}