#ifndef JSON_HPP
#define JSON_HPP

#include <jsmn.h>

namespace json {

struct Context {
    char* data;
    int data_size;

    jsmntok_t* tokens;
    int num_tokens;
};

bool parse(Context* ctx, char* data, int size);

int recursive_size(Context* ctx, int index);

// Returns index of value token for the given key, or -1 if not found.
int object_find(Context* ctx, int index, const char* key);

void parse_string(Context* ctx, char* dest_buffer, int dest_buffer_len, int index);

int parse_int(Context* ctx, int index);

float parse_float(Context* ctx, int index);

}

#endif