#pragma once

#include "Engine/Defines.h"
#include "Engine/Renderer/Render_Types.h"

struct render_packet;

struct RohinAppArgs {
    bool32 create_console;
    //const char* data_path;

    // raw
    int argc;
    const char** argv;
};

#define MAX_APP_NAME 128
struct RohinAppConfig {
    char application_name[MAX_APP_NAME];
    int32 start_x;
    int32 start_y;
    int32 start_width;
    int32 start_height;

    uint64 requested_permanant_memory;
    uint64 requested_transient_memory;

    log_level app_log_level;
    renderer_api_type backend_api;

    RohinAppArgs args;
};

struct RohinMemory {
    bool32 IsInitialized;

    // Mainly used by RohinApp
    uint64 AppStorageSize;
    void*  AppStorage; // NOTE: REQUIRED to be cleared to zero at startup!!

    // To be used in Game
    uint64 GameStorageSize;
    void*  GameStorage; // NOTE: REQUIRED to be cleared to zero at startup!!
};

struct RohinApp {
    RohinAppConfig app_config;

    // function pointers to game code
    bool32 (*startup)(RohinApp* app);
    bool32 (*initialize)(RohinApp* app);
    bool32 (*update_and_render)(RohinApp* app, render_packet* packet, real32 delta_time);
    void (*on_resize)(RohinApp* app, uint32 new_width, uint32 new_height);
    void (*shutdown)(RohinApp* app);

    RohinMemory memory;
};