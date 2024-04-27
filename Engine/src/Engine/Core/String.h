#pragma once

#include "Engine/Defines.h"

struct memory_arena;

// buffer reading utils
#define AdvanceBufferArray(Buffer, Type, Count, End) (Type*)AdvanceBufferSize_(Buffer, (Count)*sizeof(Type), End)
#define AdvanceBuffer(Buffer, Type, End) (Type*)AdvanceBufferSize_(Buffer, sizeof(Type), End)
RHAPI uint8* AdvanceBufferSize_(uint8** Buffer, uint64 Size, uint8* End);

RHAPI uint64 string_copy(char* dst_buffer, uint64 dst_buf_size, char* src_buffer);
RHAPI uint64 string_length(const char* buffer);
RHAPI int string_build(char* buffer, uint64 buf_size, char* fmt, ...);
RHAPI int string_compare(const char* str1, const char* str2);
RHAPI int string_compare(const char* str1, const char* str2, uint64 num_chars);
RHAPI char* copy_string_to_arena(const char* str, memory_arena* arena);

RHAPI char* get_next_line(char* buffer, char* end, uint64* line_length);
RHAPI void string_replace(char* buffer, uint64 buf_size, char replace_this, char with_this);
RHAPI char* string_skip_whitespace(char* buffer, char* end);

RHAPI uint64 string_find_last(char* buffer, char* end, char token);
RHAPI uint64 string_find_first(char* buffer, char* end, char token);
RHAPI bool32 string_is_numeric(char* buffer, char* end);
RHAPI bool32 string_contains(char* buffer, char* end, char token);
RHAPI bool32 string_only_whitespace(char* buffer, char* end);
RHAPI char string_first_non_whitespace_char(char* buffer, char* end);

RHAPI real32 string_to_float(char* buffer, uint64 length);
RHAPI int32  string_to_int(char* buffer, uint64 length);
