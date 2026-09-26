#include "tokenizer.h"
#include "parsemitter.h"
#include <stdio.h>
#include <stdlib.h>

// TODO: Replace return 1 with graceful error handling for missed allocations / file not found
int main(int argc, char *argv[]) {

    #ifdef DEBUG
    if (!generate_tokens("input.txt")) return 1;
    build_all_exprs();
    #else
    if (argc != 2) {
        printf("Error: Expected 1 filepath argument, received %d", argc - 1);
        return 1;
    }
    if (!generate_tokens(argv[1])) return 1;
    build_all_exprs();
    #endif

    return 0;
}