#pragma once

#include "Engine/Resources/Resource_Types.h"

#include "Engine/Platform/DXGI_Format.h"

enum class dds_file_result : int32 {
    error = -1,
    success,
};

RHAPI unsigned char* dds_load_from_memory(uint8 const *buffer, int buf_len, 
                                          int *width, int *height, Texture_Format* format,
                                          int32* is_cube, int32* mip_levels);
size_t BitsPerPixel(DXGI_FORMAT fmt) noexcept;