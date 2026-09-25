#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Assuming 4KB pages from VirtualAlloc / mmap, 1MB
static const int STACK_SIZE = 256;

typedef uint8_t opcode;

typedef union
{
    int64_t i; // also char and bool
    double f;
    void* ptr;
} FValue;

typedef enum {
    NONE
} CErrCode;

typedef struct {
                    // ra and fp should be stored at beginning of any new frame / function call
    size_t fp;      // start of current frame
    opcode* ip;      // instruction pointer
    CErrCode err;
    FValue a0, a1, addr;  // two argument registers, and one specifically for pointers
    FValue *stack;
} FriedeVM;

typedef struct {
    FriedeVM *caller;
    FriedeVM coroutine;
} FriedeCoro;

#ifdef __cplusplus
}
#endif