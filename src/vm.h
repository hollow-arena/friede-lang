#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Assuming 4KB pages from VirtualAlloc / mmap, 1MB
static const int STACK_SIZE = 256;

typedef union
{
    int64_t i;
    double f;
    char c;
    bool b;
    void* ptr;
} FValue;

typedef struct {
                    // ra and fp should be stored at beginning of any new frame / function call
    size_t ra;      // return address after finishing current frame
    size_t fp;      // start of current frame
    size_t ip;      // instruction pointer
    FValue a0, a1;  // two argument registers
    FValue *stack;
} FriedeVM;

typedef struct {
    FriedeVM *caller;
    FriedeVM coroutine;
} FriedeCoro;

#ifdef __cplusplus
}
#endif