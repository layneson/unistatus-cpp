#include <cstdlib>
#include <cstring>
#include <cassert>

#include "json.hpp"

namespace json {

bool parse(Context* ctx, char* data, int size) {
    jsmn_parser json_parser;

    jsmn_init(&json_parser);
    ctx->num_tokens = jsmn_parse(&json_parser, data, size, NULL, 0);

    assert(ctx->num_tokens > 0 && ctx->num_tokens < 5000000); // Some arbitrarily large number.
    ctx->tokens = new jsmntok_t[ctx->num_tokens];

    jsmn_init(&json_parser);
    int num_tokens_parsed = jsmn_parse(&json_parser, data, size, ctx->tokens, ctx->num_tokens);

    assert(ctx->num_tokens == num_tokens_parsed);

    ctx->data = data;
    ctx->data_size = size;

    return true;
}

int recursive_size(Context* ctx, int index) {
    jsmntok_t* tokens = ctx->tokens;

    if (tokens[index].type == JSMN_OBJECT) {
        int rsize = 1; // 1 for the object token itself.

        int num_children = tokens[index].size;

        index += 2; // Skip object token and first key.
        for (int i = 0; i < num_children; i++) {
            // index is currently pointing to a value.
            int child_size = recursive_size(ctx, index);

            rsize += child_size + 1; // + 1 for the key.

            index += child_size + 1; // Skip over the value and the next key to the following value.
        }

        return rsize;
    } else if (tokens[index].type == JSMN_ARRAY) {
        int rsize = 1; // 1 for the array itself.

        int num_children = tokens[index].size;

        index += 1; // Skip array.
        for (int i = 0; i < num_children; i++) {
            int child_size = recursive_size(ctx, index);

            rsize += child_size;
            index += child_size;
        }

        return rsize;
    }

    // This is a terminal.
    return 1;
}

// Returns index of value token for the given key, or -1 if not found.
int object_find(Context* ctx, int index, const char* key) {
    char* buffer = ctx->data;
    jsmntok_t* tokens = ctx->tokens;

    assert(tokens[index].type == JSMN_OBJECT);

    int key_size = strlen(key);

    int num_children = tokens[index].size;

    index += 1; // Align to key.

    for (int i = 0; i < num_children; i++) {
        assert(tokens[index].type == JSMN_STRING);

        if (key_size == tokens[index].end - tokens[index].start) {
            if (strncmp(key, buffer + tokens[index].start, key_size) == 0) {
                return index + 1; // Give the value, not the key.
            }
        }

        // Advance past next value.
        // We are on key, so next value is + 1.
        index += recursive_size(ctx, index + 1) + 1; // + 1 to move to following key.
    }

    return -1;
}

void parse_string(Context* ctx, char* dest_buffer, int dest_buffer_len, int index) {
    jsmntok_t* token = ctx->tokens + index;
    char* json_buffer = ctx->data;

    int token_len = token->end - token->start;
    if (token_len > dest_buffer_len - 1) { // - 1 accounts for null terminator.
        token_len = dest_buffer_len - 1;
    }

    memcpy(dest_buffer, json_buffer + token->start, token_len);
    dest_buffer[token_len] = 0;
}

int parse_int(Context* ctx, int index) {
    char int_buffer[300];

    parse_string(ctx, int_buffer, 300, index);

    return atoi(int_buffer);
}

float parse_float(Context* ctx, int index) {
    char float_buffer[300];

    parse_string(ctx, float_buffer, 300, index);

    return strtof(float_buffer, NULL);
}

}