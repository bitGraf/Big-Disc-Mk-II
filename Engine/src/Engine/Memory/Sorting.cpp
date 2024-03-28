#include "Sorting.h"

#include "Engine/Memory/Memory.h"

void swap(uint8* a, uint8* b, uint8* tmp, uint32 stride);

int32 partition(uint8* A, uint8* tmp, uint32 stride, int32 lo, int32 hi, less_than_func_t less_than);

void quicksort_in_place(uint8* A, uint8* tmp, uint32 stride, int32 lo, int32 hi, less_than_func_t less_than) {
    if (lo >= hi || lo < 0) return;
    if (hi - lo == 1) { // two element array
        if (!less_than((A+(stride*lo)),(A+(stride*hi)))) {
            // swap them
            swap((A + (stride * lo)), (A + (stride * hi)), tmp, stride);
        }
        return;
    }

    int32 p = partition(A, tmp, stride, lo, hi, less_than);
    quicksort_in_place(A, tmp, stride, lo, p - 1, less_than);
    quicksort_in_place(A, tmp, stride, p+1, hi, less_than);
}

void swap(uint8* a, uint8* b, uint8* tmp, uint32 stride) {
    memory_copy(tmp, a, stride);
    memory_copy(a, b, stride);
    memory_copy(b, tmp, stride);
}

int32 partition(uint8* A, uint8* tmp, uint32 stride, int32 lo, int32 hi,
                less_than_func_t less_than) {
    uint8* pivot = A + (hi*stride);

    // tmp pivot index
    int32 i = lo - 1;
    for (int32 j = lo; j < hi; j++) {
        if (less_than(A + (j*stride), pivot)) {
            // move tmp pivot index
            i++;
            // swap current element with element at tmp pivot index

            swap(A + (i * stride), A + (j * stride), tmp, stride);
        }
    }

    i++;
    swap(A + (i * stride), A + (hi * stride), tmp, stride);

    return i;
}