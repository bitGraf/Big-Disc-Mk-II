#include "Logger.h"
#include "Asserts.h"
#include "Engine/Platform/Platform.h"

#include <stdio.h>
#include <stdarg.h>

global_variable log_level max_log_level;

bool32 InitLogging(bool32 create_console, log_level max_level) {
    max_log_level = max_level;
    platform_init_logging(create_console);
    return true;
}

void ShutdownLogging() {
}

void LogOutput(log_level Level, const char* Message, ...) {
    const char* LevelStings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};
    if (Level <= max_log_level) {
        bool32 IsError = Level < LOG_LEVEL_WARN;

        char MsgBuffer[1024] = { 0 };

        int offset = snprintf(MsgBuffer, sizeof(MsgBuffer), "%s", LevelStings[Level]);

        va_list args;
        va_start(args, Message);
        offset += vsnprintf(MsgBuffer + offset, sizeof(MsgBuffer)-offset, Message, args);
        va_end(args);
        MsgBuffer[offset] = '\n';
        MsgBuffer[offset + 1] = '\0';

        if (IsError) {
            platform_console_write_error(MsgBuffer, (uint8)Level);
        } else {
            platform_console_write(MsgBuffer, (uint8)Level);
        }
    }
}

bool32 ReportAssertionFailure(const char* expression, const char* message, const char* file, int32 line) {
    LogOutput(LOG_LEVEL_FATAL, "Assertion Failure: %s, message: '%s', in file: %s, line: %d\n", expression, message, file, line);

    const char prefix[] = "D:\\Desktop\\Gamedev\\Rohin\\";
    uint64 offset = sizeof(prefix)-1;

    return platform_assert_message("Expression:     %s\n"
                                   "Message:        '%s'\n\n"
                                   "File:   %s\n"
                                   "Line:   %d", expression, message, file + offset, line);
}