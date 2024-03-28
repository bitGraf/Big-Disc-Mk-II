#pragma once

#include "Engine/Defines.h"

// general purpose in-place quicksort implementation
typedef bool32 (*less_than_func_t)(uint8*, uint8*);
RHAPI void quicksort_in_place(uint8* A, uint8* tmp, uint32 stride, int32 lo, int32 hi, less_than_func_t less_than);