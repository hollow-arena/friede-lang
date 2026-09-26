#pragma once

#include "tokenizer.h"

#ifdef __cplusplus
extern "C" {
#endif

void push_one();
void send_token(Token t);
void build_all_exprs();

#ifdef DEBUG
int num_of_tokens();
#endif

#ifdef __cplusplus
}
#endif