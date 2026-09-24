#pragma once

#include "tokenizer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    TYPE_VOID,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_CHAR,
    TYPE_BOOL,
    TYPE_STR,
    TYPE_INFER,
    TYPE_ARRAY,
    TYPE_WEAK,
    TYPE_STRUCT,
    TYPE_CLASS
} Type;

void send_token(Token t);
void build_lines();

#ifdef DEBUG
int num_of_tokens();
#endif

#ifdef __cplusplus
}
#endif