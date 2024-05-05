#include "Logger.h"
#include "Asserts.h"
#include "Engine/Platform/Platform.h"

#include "Engine/Core/String.h"

#include <stdio.h>
#include <stdarg.h>

global_variable log_level max_log_level = log_level::LOG_LEVEL_TRACE;

bool32 InitLogging(bool32 create_console, log_level max_level) {
    max_log_level = max_level;
    platform_init_logging(create_console);
    return true;
}

void ShutdownLogging() {
}

/*
 *  LOG_LEVEL_FATAL = 0,
 *  LOG_LEVEL_ERROR = 1,
 *  LOG_LEVEL_WARN  = 2,
 *  LOG_LEVEL_INFO  = 3,
 *  LOG_LEVEL_DEBUG = 4,
 *  LOG_LEVEL_TRACE = 5
 */
log_level log_level_from_string(char* log_level_str) {
    if (string_compare(log_level_str, "FATAL") == 0) {
        return log_level::LOG_LEVEL_FATAL;
    }
    if (string_compare(log_level_str, "ERROR") == 0) {
        return log_level::LOG_LEVEL_ERROR;
    }
    if (string_compare(log_level_str, "WARN") == 0) {
        return log_level::LOG_LEVEL_WARN;
    }
    if (string_compare(log_level_str, "INFO") == 0) {
        return log_level::LOG_LEVEL_INFO;
    }
    if (string_compare(log_level_str, "DEBUG") == 0) {
        return log_level::LOG_LEVEL_DEBUG;
    }
    if (string_compare(log_level_str, "TRACE") == 0) {
        return log_level::LOG_LEVEL_TRACE;
    }

    return log_level::LOG_LEVEL_INFO;
}

void LogOutput(log_level Level, const char* Message, ...) {
    const char* LevelStings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};
    if (Level <= max_log_level) {
        bool32 IsError = Level < LOG_LEVEL_WARN;

        char MsgBuffer[4096] = { 0 };

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