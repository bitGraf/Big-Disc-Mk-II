#include "Engine.h"

#include "Engine/Core/Logger.h"
#include "Engine/Core/Asserts.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Memory/Memory_Arena.h"
#include "Engine/Core/Event.h"
#include "Engine/Core/Input.h"
#include "Engine/Core/String.h"

#include "Engine/Renderer/Renderer.h"
#include "Engine/Resources/Resource_Manager.h"

#if defined(RH_IMGUI)
#include "Imgui.h"
#endif

struct RohinEngine {
    RohinApp* app;

    real32 target_frame_time;
    real32 last_frame_time;
    bool32 is_running;
    bool32 is_paused;
    bool32 lock_framerate;

    uint8* engine_memory;
    uint64 engine_memory_size;
    memory_arena engine_arena;
    memory_arena resource_arena;
    memory_arena frame_render_arena;

    bool32 debug_mode;

    // tmp, should probably be pulled out into a 'scene' representation
    real32 view_vert_fov;
    laml::Mat4 projection_matrix;
};
global_variable RohinEngine engine;

bool32 engine_on_resize(uint16 code, void* sender, void* listener, event_context context) {
    real32 width  = (real32)context.u32[0];
    real32 height = (real32)context.u32[1];
    laml::transform::create_projection_perspective(engine.projection_matrix, engine.view_vert_fov, width/height, 0.1f, 100.0f);

    renderer_resized(context.u32[0], context.u32[1]);
    engine.app->on_resize(engine.app, context.u32[0], context.u32[1]);

    return false;
}

bool32 engine_on_event(uint16 code, void* sender, void* listener, event_context context) {
    switch (code) {
        case EVENT_CODE_APPLICATION_QUIT:
            engine.is_running = false;
            return true;
        case EVENT_CODE_KEY_PRESSED:
            if (context.u16[0] == KEY_ESCAPE) {
                event_context no_data = {};
                event_fire(EVENT_CODE_APPLICATION_QUIT, 0, no_data);
            } else if (context.u16[0] == KEY_F1) {
                engine.debug_mode = !engine.debug_mode;
                RH_INFO("Debug Mode: %s", engine.debug_mode ? "Enabled" : "Disabled");
            } else {
                keyboard_keys key = (keyboard_keys)context.u16[0];
                //RH_INFO("Key Pressed: [%s]", input_get_key_string(key));
            }
    }

    if ((code != EVENT_CODE_KEY_PRESSED)) {
        RH_TRACE("Engine[0x%016llX] recieved event code %d \n         "
                 "Sender=[0x%016llX] \n         "
                 "Listener=[0x%016llX] \n         "
                 "Data=[%llu], [%u,%u], [%hu,%hu,%hu,%hu]",
                 (uintptr_t)(&engine), code, (uintptr_t)sender, (uintptr_t)listener,
                 context.u64,
                 context.u32[0], context.u32[1],
                 context.u16[0], context.u16[1], context.u16[2], context.u16[3]);
    }

    renderer_on_event(code, sender, listener, context);

    return false;
}

const char* getCmdOption(const char** begin, const char** end, const char* option) {
    for (const char** opt = begin; opt < end; opt++) {
        if (string_compare(*opt, option) == 0) {
            if (opt + 1 < end) {
                return *(opt + 1);
            }
        }
    }

    return 0;
}
bool32 cmdOptionExists(const char** begin, const char** end, const char* option) {
    for (const char** opt = begin; opt < end; opt++) {
        if (string_compare(*opt, option) == 0) {
            return true;
        }
    }

    return false;
}
void parseArguments(RohinAppArgs* args, int argc, const char** argv) {
    /*
    const char* data_path = getCmdOption(argv, argv + argc, "-r");
    if (data_path) {
        args->data_path = data_path;
    } else {
        args->data_path = nullptr;
    }

    */
    args->create_console = !cmdOptionExists(argv, argv + argc, "--no-console");
}
bool32 start_rohin_engine(RohinApp* app) {
    engine.app = app;
    
    // get memory for the application
    // initialize all systems we need to
    parseArguments(&app->app_config.args, app->app_config.args.argc, app->app_config.args.argv);
    InitLogging(app->app_config.args.create_console, app->app_config.app_log_level);

    AppConfig config;
    config.application_name = app->app_config.application_name;
    config.start_x = app->app_config.start_x;
    config.start_y = app->app_config.start_y;
    config.start_width = app->app_config.start_width;
    config.start_height = app->app_config.start_height;
    config.requested_memory = app->app_config.requested_permanant_memory + 
                              app->app_config.requested_transient_memory;
    if (!platform_startup(&config)) {
        RH_FATAL("Failed on platform startup!");
        return false;
    }

    RH_INFO("Rohin Engine v0.0.1");

#if RH_INTERNAL
    uint64 base_address = Terabytes(2);
#else
    uint64 base_address = 0;
#endif

    engine.debug_mode = false;
    engine.engine_memory_size = Megabytes(64);
    engine.engine_memory = (uint8*)platform_alloc(engine.engine_memory_size, 0);
    CreateArena(&engine.frame_render_arena, Megabytes(16), engine.engine_memory);
    CreateArena(&engine.engine_arena,       Megabytes(16), engine.engine_memory + Megabytes(16));
    CreateArena(&engine.resource_arena,     Megabytes(32), engine.engine_memory + Megabytes(32));

    uint32 monitor_refresh_hz = 60;
    uint32 target_framerate = 60;

    // renderer startup
    if (!renderer_initialize(&engine.engine_arena, app->app_config.backend_api, config.application_name, nullptr)) {
        RH_FATAL("Failed to initialize renderer!");
        return false;
    }
    // tmp: remove this!
    renderer_resized(config.start_width, config.start_height);

    // resource system startup
    if (!resource_init(&engine.resource_arena)) {
        RH_FATAL("Failed to initialize resource manager!");
        return false;
    }

    // after resource system is setup
    if (!renderer_create_pipeline()) {
        RH_FATAL("Failed to create render pipeline!");
        return false;
    }

    engine.target_frame_time = 1.0f / ((real32)target_framerate);
    engine.last_frame_time = engine.target_frame_time;
    engine.lock_framerate = true;

    // Initialize some systems
    event_init(&engine.engine_arena);
    event_register(EVENT_CODE_APPLICATION_QUIT, 0, engine_on_event);
    event_register(EVENT_CODE_KEY_PRESSED, 0, engine_on_event);
    //event_register(EVENT_CODE_MOUSE_MOVED, 0, engine_on_event);
    event_register(EVENT_CODE_RESIZED, 0, engine_on_resize);

    input_init(&engine.engine_arena);

    void* memory = platform_alloc(config.requested_memory, base_address);
    if (memory) {
        engine.app->memory.AppStorage = memory;
        engine.app->memory.AppStorageSize = engine.app->app_config.requested_permanant_memory;
        engine.app->memory.GameStorage = ((uint8*)memory + engine.app->memory.AppStorageSize);
        engine.app->memory.GameStorageSize = engine.app->app_config.requested_transient_memory;

        engine.is_running = true;

        if (!engine.app->startup(engine.app)) {
            engine.is_running = false;
        }

        uint64 LastCounter = platform_get_wall_clock();
        uint64 FlipWallClock = platform_get_wall_clock();
        //uint64 LastCycleCount = __rdtsc();

        real32 AR = (real32)config.start_width / (real32)config.start_height;
        engine.view_vert_fov = 75.0f;
        laml::transform::create_projection_perspective(engine.projection_matrix, engine.view_vert_fov, AR, 0.1f, 100.0f);

        engine.app->initialize(engine.app);

        // Game Loop!
        RH_INFO("------ Starting Main Loop ----------------------");
        while(engine.is_running) {
            if (!platform_process_messages()) {
                engine.is_running = false;
            }

            if (!engine.is_paused) {
                // construct render packet with all the things that are visible this frame
                ResetArena(&engine.frame_render_arena);
                render_packet* packet = PushStruct(&engine.frame_render_arena, render_packet);
                packet->arena = &engine.frame_render_arena;

                packet->delta_time = engine.last_frame_time;
                packet->projection_matrix = engine.projection_matrix;

                // Start capturing ImGui commands here, so update_and_render can add to the UI
                // TODO: possible idea: call ImGui_Begin_Frame() immediatly after drawing the frame, 
                //       so that -everywhere- else can freely add to the UI. First need to make sure 
                //       that nothing causes any issues with that method.
                renderer_debug_UI_begin_frame();

                engine.app->update_and_render(engine.app, packet, engine.last_frame_time);

                // draw Engine debug stats
#if defined(RH_IMGUI)
                ImGui::Begin("Engine");
                ImGui::Text("Total Memory:   %llu Mb (%llu Kb used)", (engine.engine_memory_size)/(1024*1024), (engine.frame_render_arena.Used+engine.engine_arena.Used+engine.resource_arena.Used)/1024);
                ImGui::Text("  engine_arena:    %2llu Mb (%llu Kb used)",   (engine.engine_arena.Size)/(1024*1024),       (engine.engine_arena.Used)/1024);
                ImGui::Text("  resource_arena:  %2llu Mb (%llu Kb used)",   (engine.resource_arena.Size)/(1024*1024),     (engine.resource_arena.Used)/1024);
                ImGui::Text("  frame_arena:     %2llu Mb (%llu byte used)", (engine.frame_render_arena.Size)/(1024*1024), (engine.frame_render_arena.Used));
                ImGui::Separator();
                ImGui::Text("Application Memory: %llu Mb (%llu Kb used)", (app->memory.AppStorageSize + app->memory.GameStorageSize) / (1024 * 1024), 0);
                ImGui::Separator();
                ImGui::Text("%d cmds | %d skeletons", packet->num_commands, packet->num_skeletons);
                ImGui::End();
#endif

                // sort packet commands into static/skinned command lists
                packet->_num_skinned = packet->num_skeletons;
                packet->_num_static  = packet->num_commands - packet->_num_skinned;
                packet->_static_cmds  = PushArray(&engine.frame_render_arena, render_command, packet->_num_static);
                packet->_skinned_cmds = PushArray(&engine.frame_render_arena, render_command, packet->_num_skinned);
                uint32 static_idx = 0, skinned_idx = 0;
                for (uint32 n = 0; n < packet->num_commands; n++) {
                    const render_command& cmd = packet->commands[n];

                    if (cmd.skeleton_idx) {
                        packet->_skinned_cmds[skinned_idx++] = cmd;
                    } else {
                        packet->_static_cmds[static_idx++] = cmd;
                    }
                }

                renderer_draw_frame(packet, engine.debug_mode);

                uint64 WorkCounter = platform_get_wall_clock();
                real32 WorkSecondsElapsed = (real32)platform_get_seconds_elapsed(LastCounter, WorkCounter);

                // TODO: Untested! buggy
                real32 SecondsElapsedForFrame = WorkSecondsElapsed;
                bool32 LockFramerate = true;
                if (LockFramerate) {
                    if (SecondsElapsedForFrame < engine.target_frame_time) {
                        uint64 SleepMS = (uint64)(1000.0f*(engine.target_frame_time - SecondsElapsedForFrame));
                        if (SleepMS > 0) {
                            platform_sleep(SleepMS);
                        }
    
                        while (SecondsElapsedForFrame < engine.target_frame_time) {
                            SecondsElapsedForFrame = (real32)platform_get_seconds_elapsed(LastCounter, platform_get_wall_clock());
                        }
                    } else {
                        // TODO: Missed Frame Rate!
                        //RH_ERROR("Frame rate missed!");
                    }
                }

                uint64 EndCounter = platform_get_wall_clock();
                engine.last_frame_time = (real32)platform_get_seconds_elapsed(LastCounter, EndCounter);
                real32 MSPerFrame = (real32)(1000.0f * engine.last_frame_time);
                //MSLastFrame.addSample(MSPerFrame);
                LastCounter = EndCounter;
    
                //Win32DisplayBufferToWindow(DeviceContext, Dimension.Width, Dimension.Height);
                renderer_present();
    
                FlipWallClock = platform_get_wall_clock();

                //uint64 EndCycleCount = __rdtsc();
                //uint64 CyclesElapsed = EndCycleCount - LastCycleCount;
                //LastCycleCount = EndCycleCount;
    
                real32 FPS = 1000.0f / MSPerFrame;
                //real32 MCPF = ((real32)CyclesElapsed / (1000.0f*1000.0f));
    
#if 0
                RH_TRACE("Frame: %.02f ms  %.02ffps\n", MSPerFrame, FPS);
#else
                platform_console_set_title("%s: %.02f ms, FPS: %.02ffps", config.application_name, MSPerFrame, FPS);
#endif

                input_update(engine.last_frame_time);
            }
        }

        engine.app->shutdown(engine.app);

        platform_free(memory);
    } else {
        RH_FATAL("Requested %u bytes of memory, but could not get it from the os!", config.requested_memory);
    }

    renderer_shutdown();
    platform_shutdown();

    // shutdown all systems
    input_shutdown();
    event_shutdown();
    ShutdownLogging();

    return true;
}