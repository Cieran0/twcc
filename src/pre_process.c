#include "pre_process.h"
#include "file.h"
#include "string_builder.h"
#include "string.h"
#include "stdio.h"

char* load_file_with_include_paths(const char* filename, const char** includes, size_t includes_len) {
    if(filename == NULL) return NULL;
    
    char* file = load_file(filename);
    if(file != NULL) {
        return file;
    }

    for (size_t i = 0; i < includes_len; i++)
    {
        string_builder sb = string_builder_new(32);
        string_builder_append(&sb, includes[i]);
        string_builder_append(&sb, "/");
        string_builder_append(&sb, filename);
        char* path = string_builder_build(&sb);
        file = load_file(path);
        string_builder_destroy(&sb);
        if (file != NULL) {
            free(path);
            return file;
        }
    }
    

    return NULL;
}

char* pre_process(const char* file, const char** includes, size_t includes_len) {
    char* pre_processed = clone_str(file);
    size_t index = 0;
    char c;
    
    while ((c = pre_processed[index]) != '\0')
    {
        if(c == '\t' || c == ' ') {
            index++;
            continue;
        }

        if (c != '#') {
            while ((c = pre_processed[index]) != '\n' && c != '\0')
            {
                index++;
            }
            if (c == '\n') index++; 
            continue;
        }

        // Check if it's #include
        if(strncmp(pre_processed+index, "#include", 8) != 0) {
            printf("Some issue occured\n");
            free(pre_processed);
            return NULL;
        } 

        int start_of_include_line = index;
        int end_of_include_line = index + 8;
        while ((c = pre_processed[end_of_include_line]) != '\n' && c != '\0')
        {
            end_of_include_line++;
        }

        char end_char = 0;
        int file_name_include_start = 0;
        int file_name_include_end = 0;
        
        for (size_t i = index + 8; i < end_of_include_line; i++)
        {
            if(pre_processed[i] == ' ' || pre_processed[i] == '\t') {
                continue;
            }

            if(pre_processed[i] == '\"' && end_char == 0) {
                file_name_include_start = i + 1;
                end_char = '\"';
            } else if (pre_processed[i] == '<' && end_char == 0) {
                file_name_include_start = i + 1;
                end_char = '>';
            } else if (end_char != 0 && pre_processed[i] == end_char) {
                file_name_include_end = i;
                break; 
            }
            else if (end_char == 0) {
                printf("Some issue occured 2\n");
                free(pre_processed);
                return NULL;
            }
        }
        
        if (end_char == 0 || file_name_include_end == 0) {
            printf("Matching delimiter not found\n");
            free(pre_processed);
            return NULL;
        }
        
        int include_name_size = file_name_include_end - file_name_include_start + 1;
        char* include_name = malloc(include_name_size);
        memcpy(include_name, pre_processed + file_name_include_start, include_name_size - 1);
        include_name[include_name_size - 1] = '\0';
        printf("Include name: %s\n", include_name);

        char* included_file = load_file_with_include_paths(include_name, includes, includes_len);
        free(include_name);
        
        if(included_file == NULL) {
            free(pre_processed);
            return NULL;
        }
        
        char* pre_processed_include = pre_process(included_file, includes, includes_len);
        free(included_file);
        
        if (pre_processed_include == NULL) {
            free(pre_processed);
            return NULL;
        }
        
        size_t before_len = start_of_include_line;
        size_t include_len = strlen(pre_processed_include);
        size_t after_len = strlen(pre_processed + end_of_include_line);
        size_t new_size = before_len + include_len + after_len + 1;
        
        char* new_file = malloc(new_size);
        if (new_file == NULL) {
            free(pre_processed_include);
            free(pre_processed);
            return NULL;
        }
        
        memcpy(new_file, pre_processed, before_len);
        memcpy(new_file + before_len, pre_processed_include, include_len);
        memcpy(new_file + before_len + include_len, 
               pre_processed + end_of_include_line, 
               after_len);
        new_file[new_size - 1] = '\0';
        
        free(pre_processed);
        free(pre_processed_include);
        pre_processed = new_file;

        index = before_len + include_len;
    }
    
    return pre_processed;
}