#pragma once

#include "Engine/Defines.h"

struct AppConfig {
    const char* application_name;
    int32 start_x;
    int32 start_y;
    int32 start_width;
    int32 start_height;

    uint64 requested_memory;
};

/*
 * Platform Layer:
 *   provides services to higher layers
 * 
 * exported functions:
 *   startup()
 *   shutdown()
 *   process_messages()
 *   platform_alloc() <= returns zeroed memory :)
 *   platform_free()
 *   write to console
 *   get_time()
 *   sleep()
 * 
 * File I/O
 * */

void platform_init_logging(bool32 create_console);
RHAPI bool32 platform_setup_paths();
bool32 platform_startup(AppConfig* config);
RHAPI void platform_shutdown();
bool32 platform_process_messages();
void* platform_alloc(uint64 size, uint64 base_address);
void platform_free(void* memory);
bool32 platform_assert_message(const char* fmt, ...);
void platform_console_write_error(const char* Message, uint8 Color);
void platform_console_write(const char* Message, uint8 Color);
void platform_console_set_title(const char* Message, ...);
int64 platform_get_wall_clock();
real64 platform_get_seconds_elapsed(int64 start, int64 end);
RHAPI void platform_sleep(uint64 ms);
void platform_update_mouse();
void* platform_get_window_handle();

// rendering stuff
void platform_swap_buffers();

// file i/o
struct file_handle {
    uint64 num_bytes;
    uint8* data;
};
RHAPI size_t platform_get_full_resource_path(char* buffer, size_t buffer_length, const char* resource_path);
RHAPI file_handle platform_read_entire_file(const char* full_path);
RHAPI void platform_free_file_data(file_handle* handle);

struct file_info {
    uint64 file_attributes;
    uint64 creation_time;
    uint64 last_access_time;
    uint64 last_write_time;
    uint64 file_size;
};
RHAPI size_t platform_get_full_library_path(char* buffer, size_t buffer_length, const char* library_path);
RHAPI bool32 platform_get_file_attributes(const char* full_path, file_info* info);
RHAPI bool32 platform_copy_file(const char* src_path, const char* dst_path);

RHAPI void* platform_load_shared_library(const char* lib_path);
RHAPI void* platform_get_func_from_lib(void* shared_lib, const char* func_name);
RHAPI void  platform_unload_shared_library(void* shared_lib);
RHAPI void  platform_filetime_to_systime(uint64 file_time, char* buffer, uint64 buf_size);

// this is bad and dumb :)
#if RH_PLATFORM_LINUX
void* platform_get_raw_handle();
uint32 platform_get_window_id();
#endif