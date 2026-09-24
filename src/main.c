#include "tokenizer.h"
#include "parsemitter.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    char* code;

    #ifdef DEBUG
    code = generate_tokens("input.txt");
    build_lines();
    #else
    if (argc != 2) {
        printf("Error: Expected 1 filepath argument, received %d", argc - 1);
        return 1;
    }
    code = generate_tokens(argv[1]);
    #endif

    free(code);
    return 0;
}