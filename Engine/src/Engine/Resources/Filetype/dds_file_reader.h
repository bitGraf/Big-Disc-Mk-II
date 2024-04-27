#pragma once

#include "Engine/Resources/Resource_Types.h"

enum class dds_file_result : int32 {
    error = -1,
    success,
};

RHAPI unsigned char* dds_load_from_memory(uint8 const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels);