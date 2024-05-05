#pragma once

#include "Engine/Core/Logger.h"
#include "Engine/Application.h"
#include "Engine/Core/Engine.h"

// todo: remove this?
#include "Engine/Platform/Platform.h"

extern bool32 create_application(RohinApp* app);

int main(int argc, char** argv) {
    RohinApp app;
    app.app_config.args.argc = argc;
    app.app_config.args.argv = (const char**)argv;
    memory_zero(app.app_config.application_name, sizeof(app.app_config.application_name));

    if (!create_application(&app)) {
        RH_FATAL("Could not create application!");
		platform_sleep(2500);
        return -1;
    }

    if (!app.update_and_render || !app.initialize || !app.on_resize) {
        RH_FATAL("Application function pointers not setup!");
		platform_sleep(2500);
        return -2;
    }

    // begin the game-loop
    if (!start_rohin_engine(&app)) {
        RH_FATAL("Something went wrong! shutting down...");
        platform_shutdown();
        platform_sleep(2500);
        return -3;
    }

    return 0;
}

#if RH_PLATFORM_WINDOWS
#include <cstdlib>
int WinMain(void* instance, void* prev_instance, char* cmd_line, int show_cmd) {
    //int argc = __argc;
    //char** argv = __argv;
    return main(__argc, __argv);
}
#endif