#include "function.h"
#include "tokenise.h"
#include "string_builder.h"
#include "stdbool.h"
#include "stdio.h"

function* extract_function(vector_token* tokens) {

    if(tokens->size <= 4) return NULL;

    size_t signature_index = 0;
    size_t argument_start = 0;
    size_t argument_end = 0;
    size_t code_start = 0;
    size_t code_end = 0;
    bool found = false;

    for (size_t i = 0; i < tokens->size -2; i++)
    {
        if(tokens->data[i].type != TOKEN_TYPE) continue;
        if(tokens->data[i+1].type != TOKEN_NAME) continue;
        if(tokens->data[i+2].type != TOKEN_OPEN_BRACE) continue;
        found = true;
        signature_index = i;
        break;
    }

    if(!found) {
        printf("No signature found\n");
        return NULL;
    }
    argument_start = signature_index+2;
    
    for (size_t i = argument_start+1; i < tokens->size; i++)
    {
        found = 0;
        if(tokens->data[i].type == TOKEN_CLOSE_BRACE) {
            found = 1;
            argument_end = i;
            break;
        }
    }


    if(!found) {
        printf("Arguments malformed\n");
        return NULL;
    }
    code_start = argument_end + 1;

    bool is_definition = false;

    if(code_start >= tokens->size || tokens->data[code_start].type != TOKEN_OPEN_CURLY_BRACE) {
        if(tokens->data[code_start].type != TOKEN_SEMI_COLON) {
            return NULL;
        }
        is_definition = true;
        code_end = code_start;
    }

    size_t open_counter = 0;
    if(is_definition) {
        goto skip_getting_code;
    }

    for (size_t i = code_start + 1; i < tokens->size; i++)
    {
        found = 0;
        if(tokens->data[i].type == TOKEN_OPEN_CURLY_BRACE) {
            open_counter++;
        } else if (tokens->data[i].type != TOKEN_CLOSE_CURLY_BRACE) {
            continue;
        }

        if (open_counter == 0) {
            code_end = i;
            found = 1;
            break;
        }

        open_counter--;
    }

    if (!found) {
        return NULL;
    }
    
skip_getting_code:

    name_type_pair function_signature = {
        type_from_type_token(tokens->data[signature_index]),
        clone_str(tokens->data[signature_index+1].content)
    };

    size_t argc = 0;
    name_type_pair* args = NULL;
    
    size_t current_idx = argument_start + 1; 
    
    while (current_idx < argument_end) {
        if (tokens->data[current_idx].type != TOKEN_TYPE) break;
        
        if (current_idx + 1 >= argument_end) break;
        if (tokens->data[current_idx + 1].type != TOKEN_NAME) break;
        
        argc++;
        current_idx += 2;
        
        if (current_idx < argument_end) {
            if (tokens->data[current_idx].type == TOKEN_COMMA) {
                current_idx++;
            } else {
                found = 0;
                break;
            }
        }
    }
    
    if(!found) {
        free(function_signature.name);
        return NULL;
    }

    if(argc > 0 ) {
        args = (name_type_pair*)malloc(sizeof(name_type_pair) * argc);
        for (size_t i = 0; i < argc; i++)
        {
            size_t arg_index =  (argument_start+1)+i*3;
            args[i] = (name_type_pair) {
                type_from_type_token(tokens->data[arg_index]),
                clone_str(tokens->data[arg_index+1].content)
            };
        }
        
    }
    vector_token code = vector_token_new(0);
    if(is_definition) {
        goto return_function;
    }
    code = vector_token_new((code_end-code_start)+1);

    for (size_t i = code_start+1; i < code_end; i++)
    {
        vector_token_push(&code, token_clone(tokens->data[i]));
    }
    
return_function:
    function* f = (function*)malloc(sizeof(function));

    *f = (function) {
        function_signature,
        argc,
        args,
        code
    };

    vector_token_remove(tokens, signature_index, code_end);

    return f;
}
