#include "string_hash.h"
#include "stdbool.h"
#include "stdint.h"
#include "string.h"

static string_bucket_node buckets[256];
static bool intialised = false;  
static size_t last_id = 0;
static char** reverse_map = NULL;
static size_t reverse_map_size = 0;

void intialise_string_map() {
    for (size_t i = 0; i < (sizeof(buckets) / sizeof(buckets[0])); i++)
    {
        buckets[i] = (string_bucket_node) {
            .content = NULL,
            .id = 0,
            .next = NULL
        };
    }
    reverse_map_size = 256;
    reverse_map = malloc(reverse_map_size);
    intialised = true;
}

static uint8_t hash_string(const char *str)
{
    uint32_t hash = 2166136261u;

    while (*str) {
        hash ^= (unsigned char)*str++;
        hash *= 16777619u;
    }

    return (uint8_t)hash;
}

void reverse_map_grow() {
    if(last_id >= reverse_map_size) {
        reverse_map_size*=2;
        reverse_map = realloc(reverse_map, reverse_map_size);
    }
}

size_t get_string_id(const char* string) {
    if(string == NULL) return 0;

    if(!intialised) {
        intialise_string_map();
    }

    size_t bucket = hash_string(string);

    string_bucket_node* node = &buckets[bucket];
    if(node->content == NULL) {
        last_id++;
        node->content = strdup(string);
        node->next = NULL;
        node->id = last_id;
        reverse_map_grow();
        reverse_map[last_id] = node->content;
        return node->id;
    }

    if(strcmp(node->content, string) == 0) {
        return node->id;
    }

    while (node->next != NULL)
    {
        if(strcmp(node->content, string) == 0) {
            return node->id;
        }
        node = node->next;
    }

    string_bucket_node* new_node = malloc(sizeof(string_bucket_node));
    node->next = new_node;
    last_id++;
    new_node->content = strdup(string);
    new_node->next = NULL;
    new_node->id = last_id;
    reverse_map_grow();
    reverse_map[last_id] = node->content;
    return node->id;
}

char* content_of_id(size_t id) {
    if(id == 0 || id > last_id) {
        return NULL;
    }
    return reverse_map[id];
}