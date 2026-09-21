#include "stdio.h"
#include "stdlib.h"

char* load_file(const char* filename) {
    FILE *f = fopen(filename, "rb");
    if(!f) return NULL;

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *string = malloc(fsize + 1);
    fread(string, fsize, 1, f);
    fclose(f);

    return string;
}

void write_to_file(const char* filename, const char* string) {
    FILE *f;

    f = fopen(filename, "w");
    fprintf(f, string);
    fclose(f); 

}