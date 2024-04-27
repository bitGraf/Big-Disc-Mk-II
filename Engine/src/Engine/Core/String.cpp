#include "String.h"

#include "Engine/Core/Asserts.h"
#include "Engine/Memory/Memory_Arena.h"
#include "Engine/Memory/Memory.h"

// todo: STD library
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8* AdvanceBufferSize_(uint8** Buffer, uint64 Size, uint8* End) {
    AssertMsg((*Buffer + Size) <= End, "Reached the end of the buffer!");
    uint8* Result = *Buffer;
    *Buffer += Size;
    return Result;
}

uint64 string_copy(char* dst_buffer, uint64 dst_buf_size, char* src_buffer) {
    uint64 num_copied = 0;
    for (uint64 n = 0; n < dst_buf_size; n++) {
        dst_buffer[n] = src_buffer[n];
        num_copied++;
        
        // check after, so we still copy the null-terminator
        if (src_buffer[n] == 0) break;
    }
    return num_copied;
}

uint64 string_length(const char* buffer) {
    uint64 len = 0;
    for (const char* scan = buffer; *scan; scan++) {
        len++;
    }

    return len;
}

int string_build(char* buffer, uint64 buf_size, char* fmt, ...) {
    // todo: STD library
    va_list args;
    va_start(args, fmt);

    int res = vsnprintf(buffer, buf_size, fmt, args);

    va_end(args);

    return res;
}

int string_compare(const char* str1, const char* str2) {
    // todo: STD library
    return strcmp(str1, str2);
}

int string_compare(const char* str1, const char* str2, uint64 num_chars) {
    // todo: STD library
    return strncmp(str1, str2, num_chars);
}

char* copy_string_to_arena(const char* str, memory_arena* arena) {
    size_t len = strlen(str);
    char* new_str = PushArray(arena, char, len + 1);
    memory_copy(new_str, str, len);
    new_str[len] = 0;
    return new_str;
}

char* get_next_line(char* buffer, char* end, uint64* line_length) {
    for (char* scan = buffer; (scan<end) && (*scan); scan++) {
        if (*scan == '\n') {
            while ((scan < end) && (*scan == '\n')) {
                scan++;
            }
            *line_length = string_find_first(scan, end, '\n');
            return scan;
        }
    }

    return nullptr;
}

void string_replace(char* buffer, uint64 buf_size, char replace_this, char with_this) {
    for (uint64 n = 0; (n < buf_size) && (buffer[n]); n++) {
        if (buffer[n] == replace_this)
            buffer[n] = with_this;
    }
}

uint64 string_find_first(char* buffer, char* end, char token) {
    for (char* scan = buffer; (scan < end) && (*scan); scan++) {
        if (*scan == token) {
            return (scan - buffer);
        }
    }
    
    return (end-buffer);
}

uint64 string_find_last(char* buffer, char* end, char token) {
    uint64 last = 0;
    for (char* scan = buffer; (scan < end) && (*scan); scan++) {
        if (*scan == token) {
            last = (scan - buffer);
        }
    }
    
    return last;
}

char* string_skip_whitespace(char* buffer, char* end) {
    for (char* scan = buffer; (scan < end) && (*scan); scan++) {
        if (*scan != ' ') {
            return scan;
        }
    }

    return end;
}

bool32 string_is_numeric(char* buffer, char* end) {
    for (char* scan = buffer; (scan < end) && (*scan); scan++) {
        if ((*scan < 46) || (*scan > 57) || (*scan == 47)) {
            // numbers [0,9] are ASCII [48,57]
            // '.' is 46 and '/' is 47
            // ' ' is 32
            if (*scan == 32) continue;

            return false;
        }
    }

    return true;
}
bool32 string_contains(char* buffer, char* end, char token) {
    for (char* scan = buffer; (scan < end) && (*scan); scan++) {
        if (*scan == token) {
            return true;
        }
    }

    return false;
}

bool32 string_only_whitespace(char* buffer, char* end) {
    for (char* scan = buffer; (scan < end) && (*scan); scan++) {
        if (*scan != ' ') {
            return false;
        }
    }

    return true;
}

char string_first_non_whitespace_char(char* buffer, char* end) {
    for (char* scan = buffer; (scan < end) && (*scan); scan++) {
        if (*scan != ' ') {
            return *scan;
        }
    }

    return *end;
}

real32 string_to_float(char* buffer, uint64 length) {
    return (real32)atof(buffer);
}
int32  string_to_int(char* buffer, uint64 length) {
    return atoi(buffer);
}