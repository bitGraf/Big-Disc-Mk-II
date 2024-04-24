#include "OpenGL_Platform.h"

#include "Engine/Core/Logger.h"
#include "Engine/Core/Asserts.h"
#include "Engine/Debug_UI/Debug_UI.h"

#if RH_PLATFORM_WINDOWS

#if defined(RH_COMPILE_OPENGL_4_4)
    #define __DESIRED_GL_VERSION_MAJOR 4
    #define __DESIRED_GL_VERSION_MINOR 4
#elif defined(RH_COMPILE_OPENGL_4_3)
    #define __DESIRED_GL_VERSION_MAJOR 4
    #define __DESIRED_GL_VERSION_MINOR 3
#else
    #define __DESIRED_GL_VERSION_MAJOR 4
    #define __DESIRED_GL_VERSION_MINOR 0
#endif

#if defined(RH_IMGUI)
#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_opengl3.h"
#endif

/******************************************************************************
    * WGL_ARB_extensions_string 
    * https://registry.khronos.org/OpenGL/extensions/ARB/WGL_ARB_extensions_string.txt
    */

typedef const char *(__stdcall * _wglGetExtensionsStringARB_PROC)(HDC hdc);

/******************************************************************************
    * WGL_ARB_create_context 
    * https://registry.khronos.org/OpenGL/extensions/ARB/WGL_ARB_create_context.txt
    * */

typedef HGLRC (__stdcall * _wglCreateContextAttribsARB_PROC)(HDC hDC, HGLRC hShareContext, const int *attribList);

// Accepted as an attribute name in <*attribList>:
#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126

// Accepted as bits in the attribute value for WGL_CONTEXT_FLAGS in <*attribList>:
#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002

// Accepted as bits in the attribute value for WGL_CONTEXT_PROFILE_MASK_ARB in <*attribList>:
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

// New errors returned by GetLastError:
#define ERROR_INVALID_VERSION_ARB               0x2095
#define ERROR_INVALID_PROFILE_ARB               0x2096

/******************************************************************************
    * WGL_ARB_pixel_format
    * https://registry.khronos.org/OpenGL/extensions/ARB/WGL_ARB_pixel_format.txt
    * */

typedef BOOL (__stdcall * _wglGetPixelFormatAttribivARB_PROC)(HDC hdc,int iPixelFormat,int iLayerPlane,UINT nAttributes,const int *piAttributes,int *piValues);
typedef BOOL (__stdcall * _wglGetPixelFormatAttribfvARB_PROC)(HDC hdc,int iPixelFormat,int iLayerPlane,UINT nAttributes,const int *piAttributes,FLOAT *pfValues);
typedef BOOL (__stdcall * _wglChoosePixelFormatARB_PROC)(HDC hdc,const int *piAttribIList,const FLOAT *pfAttribFList,UINT nMaxFormats,int *piFormats,UINT *nNumFormats);

// Accepted in the <piAttributes> parameter array of 
// wglGetPixelFormatAttribivARB, and wglGetPixelFormatAttribfvARB, 
// and as a type in the <piAttribIList> and <pfAttribFList> 
// parameter arrays of wglChoosePixelFormatARB:
#define WGL_NUMBER_PIXEL_FORMATS_ARB            0x2000
#define WGL_DRAW_TO_WINDOW_ARB                  0x2001
#define WGL_DRAW_TO_BITMAP_ARB                  0x2002
#define WGL_ACCELERATION_ARB                    0x2003
#define WGL_NEED_PALETTE_ARB                    0x2004
#define WGL_NEED_SYSTEM_PALETTE_ARB             0x2005
#define WGL_SWAP_LAYER_BUFFERS_ARB              0x2006
#define WGL_SWAP_METHOD_ARB                     0x2007
#define WGL_NUMBER_OVERLAYS_ARB                 0x2008
#define WGL_NUMBER_UNDERLAYS_ARB                0x2009
#define WGL_TRANSPARENT_ARB                     0x200A
#define WGL_TRANSPARENT_RED_VALUE_ARB           0x2037
#define WGL_TRANSPARENT_GREEN_VALUE_ARB         0x2038
#define WGL_TRANSPARENT_BLUE_VALUE_ARB          0x2039
#define WGL_TRANSPARENT_ALPHA_VALUE_ARB         0x203A
#define WGL_TRANSPARENT_INDEX_VALUE_ARB         0x203B
#define WGL_SHARE_DEPTH_ARB                     0x200C
#define WGL_SHARE_STENCIL_ARB                   0x200D
#define WGL_SHARE_ACCUM_ARB                     0x200E
#define WGL_SUPPORT_GDI_ARB                     0x200F
#define WGL_SUPPORT_OPENGL_ARB                  0x2010
#define WGL_DOUBLE_BUFFER_ARB                   0x2011
#define WGL_STEREO_ARB                          0x2012
#define WGL_PIXEL_TYPE_ARB                      0x2013
#define WGL_COLOR_BITS_ARB                      0x2014
#define WGL_RED_BITS_ARB                        0x2015
#define WGL_RED_SHIFT_ARB                       0x2016
#define WGL_GREEN_BITS_ARB                      0x2017
#define WGL_GREEN_SHIFT_ARB                     0x2018
#define WGL_BLUE_BITS_ARB                       0x2019
#define WGL_BLUE_SHIFT_ARB                      0x201A
#define WGL_ALPHA_BITS_ARB                      0x201B
#define WGL_ALPHA_SHIFT_ARB                     0x201C
#define WGL_ACCUM_BITS_ARB                      0x201D
#define WGL_ACCUM_RED_BITS_ARB                  0x201E
#define WGL_ACCUM_GREEN_BITS_ARB                0x201F
#define WGL_ACCUM_BLUE_BITS_ARB                 0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB                0x2021
#define WGL_DEPTH_BITS_ARB                      0x2022
#define WGL_STENCIL_BITS_ARB                    0x2023
#define WGL_AUX_BUFFERS_ARB                     0x2024

// Accepted as a value in the <piAttribIList> and <pfAttribFList>
// parameter arrays of wglChoosePixelFormatARB, and returned in the
// <piValues> parameter array of wglGetPixelFormatAttribivARB, and the
// <pfValues> parameter array of wglGetPixelFormatAttribfvARB:
#define WGL_NO_ACCELERATION_ARB                 0x2025
#define WGL_GENERIC_ACCELERATION_ARB            0x2026
#define WGL_FULL_ACCELERATION_ARB               0x2027
#define WGL_SWAP_EXCHANGE_ARB                   0x2028
#define WGL_SWAP_COPY_ARB                       0x2029
#define WGL_SWAP_UNDEFINED_ARB                  0x202A
#define WGL_TYPE_RGBA_ARB                       0x202B
#define WGL_TYPE_COLORINDEX_ARB                 0x202C

//static void Win32InitOpenGL(HWND Window, int* MonitorRefreshHz, int* GameRefreshHz);
//static void Win32UpdateVSync(int* MonitorRefreshHz, int* GameRefreshHz);

/******************************************************************************
    * WGL_EXT_swap_control
    * https://registry.khronos.org/OpenGL/extensions/EXT/WGL_EXT_swap_control.txt
    * */
typedef BOOL (__stdcall * _wglSwapIntervalEXT_PROC)(int interval);
typedef int (__stdcall * _wglGetSwapIntervalEXT_PROC)(void);

/******************************************************************************
    * random funcs
    * */
#define GL_NUM_EXTENSIONS 0x821D
typedef const GLubyte * (__stdcall * _glGetStringi_PROC)(GLenum name,GLuint index);

// static function pointers
global_variable _wglSwapIntervalEXT_PROC wglSwapIntervalEXT;
global_variable _wglGetSwapIntervalEXT_PROC wglGetSwapIntervalEXT;

// debug message callback
void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id,
                                     GLenum severity, GLsizei length,
                                     const GLchar *msg, const void *data);

int win32_get_monitor_refresh_rate(HWND window);

bool32 OpenGL_create_context() {
    HWND window = GetActiveWindow();
    if (!window) {
        return false;
    }
    HDC device_context = GetDC(window);
    if (!device_context) {
        return false;
    }

    // first, get basic opengl context
    PIXELFORMATDESCRIPTOR DesiredPixelFormat = { sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
        PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
        32,                   // Colordepth of the framebuffer.
        0, 0, 0, 0, 0, 0,     // r/g/b bits/shift
        0,                    // alpha bits
        0,                    // alpha shift
        0,                    // accum bits
        0, 0, 0, 0,           // accum r/g/b/a bits
        24,                   // Number of bits for the depthbuffer
        8,                    // Number of bits for the stencilbuffer
        0,                    // Number of Aux buffers in the framebuffer.
        PFD_MAIN_PLANE,       // Layer type
        0,                    // bReserved
        0, 0, 0               // Masks
    };

    int SuggestedPixelFormatIndex = ChoosePixelFormat(device_context, &DesiredPixelFormat);
    PIXELFORMATDESCRIPTOR SuggestedPixelFormat;
    DescribePixelFormat(device_context, SuggestedPixelFormatIndex, sizeof(PIXELFORMATDESCRIPTOR), &SuggestedPixelFormat);
    SetPixelFormat(device_context, SuggestedPixelFormatIndex, &SuggestedPixelFormat);

    // create a dummy context to get access to wgl extensions... to create the real context
    HGLRC DummyOpenGLRC = wglCreateContext(device_context);
    if (!wglMakeCurrent(device_context, DummyOpenGLRC)) {
        return false;
    }

    // Get extension functions from wgl
    _wglCreateContextAttribsARB_PROC wglCreateContextAttribsARB = (_wglCreateContextAttribsARB_PROC)wglGetProcAddress("wglCreateContextAttribsARB");
    _wglGetPixelFormatAttribivARB_PROC wglGetPixelFormatAttribivARB = (_wglGetPixelFormatAttribivARB_PROC)wglGetProcAddress("wglGetPixelFormatAttribivARB");
    _wglGetPixelFormatAttribfvARB_PROC wglGetPixelFormatAttribfvARB = (_wglGetPixelFormatAttribfvARB_PROC)wglGetProcAddress("wglGetPixelFormatAttribfvARB");
    _wglChoosePixelFormatARB_PROC wglChoosePixelFormatARB = (_wglChoosePixelFormatARB_PROC)wglGetProcAddress("wglChoosePixelFormatARB");

    const int ARBPixelFormatAttribList[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, 32,
        WGL_DEPTH_BITS_ARB, 24,
        WGL_STENCIL_BITS_ARB, 8,
        0, // End
    };

    int ARBSuggestedPixelFormatIndex;
    UINT NumFormats;
        
    bool32 ARBResult = wglChoosePixelFormatARB(device_context, ARBPixelFormatAttribList, NULL, 1, &ARBSuggestedPixelFormatIndex, &NumFormats);
    //Assert(ARBResult && (ARBSuggestedPixelFormatIndex == SuggestedPixelFormatIndex));
    if (ARBResult && (ARBSuggestedPixelFormatIndex != SuggestedPixelFormatIndex)) {
        // We got a "better" pixel format from the wgl extension
        SetPixelFormat(device_context, ARBSuggestedPixelFormatIndex, NULL);
    }

    // now create a higher-version context
    const int ARBContexAttrivbs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, __DESIRED_GL_VERSION_MAJOR,
        WGL_CONTEXT_MINOR_VERSION_ARB, __DESIRED_GL_VERSION_MINOR,
        WGL_CONTEXT_FLAGS_ARB,         WGL_CONTEXT_DEBUG_BIT_ARB | WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0 // End
    };

    HGLRC ActualOpenGLRC = wglCreateContextAttribsARB(device_context, 0, ARBContexAttrivbs);
    if (ActualOpenGLRC) {
        // able to create an OpenGL 4.0 context!
        wglMakeCurrent(device_context, ActualOpenGLRC);

        // delete the dummy context too
        wglDeleteContext(DummyOpenGLRC);
    } else {
        // Stuck with an OpenGL 1.0 context ;~;
        ActualOpenGLRC = DummyOpenGLRC;
        RH_FATAL("Tried to get an OpenGL4.3 context, but could not!");
        return false;
    }

    RH_INFO("OpenGL 4.3 Context created!");
    gladLoadGL(); // Bind the rest of the OpenGL 4.0 functions

    // enable debug output
    //glEnable              ( GL_DEBUG_OUTPUT );
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback( GLDebugMessageCallback, NULL );

    const char* glVendorString     = (const char*) glGetString(GL_VENDOR);
    const char* glRendererString   = (const char*) glGetString(GL_RENDERER);
    const char* glVersionString    = (const char*) glGetString(GL_VERSION);

    GLint MaxUniformLocations;
    glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS,&MaxUniformLocations);

    RH_INFO(glVendorString);
    RH_INFO(glRendererString);
    RH_INFO(glVersionString);

#if 0
    _glGetStringi_PROC glGetStringi = (_glGetStringi_PROC)wglGetProcAddress("glGetStringi");
    GLint num_extensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
    Win32LogMessage("OpenGL Extensions:");
    for (GLint n = 0; n < num_extensions; n++) {
        const char* glExtensionsString = (const char*) glGetStringi(GL_EXTENSIONS, n);
        Win32LogMessage("  %s", glExtensionsString);
    }
#endif

    // Setup VSync
    wglSwapIntervalEXT = (_wglSwapIntervalEXT_PROC)wglGetProcAddress("wglSwapIntervalEXT");
    wglGetSwapIntervalEXT = (_wglGetSwapIntervalEXT_PROC)wglGetProcAddress("wglGetSwapIntervalEXT");

#if 0
    uint32 NewRefreshHz = win32_get_monitor_refresh_rate(window);
    if (NewRefreshHz) {
        *MonitorRefreshHz = NewRefreshHz;
        RH_INFO("Detected monitor refresh rate: %d Hz", *MonitorRefreshHz);
    }
        
    RH_INFO("Requested game refresh rate: %d Hz", *GameRefreshHz);
    win32_update_vsync(MonitorRefreshHz, GameRefreshHz);
#endif

    return true;
}

bool32 OpenGL_destroy_context() {
    // TODO: see if anything else needs to be destroyed here
    return true;
}

bool32 OpenGL_ImGui_Init() {
    HWND window = GetActiveWindow();
    if (!window) {
        return false;
    }

    #if defined(RH_IMGUI)
    // setup imgui
    ImGui_ImplWin32_Init(window);
    ImGui_ImplOpenGL3_Init();
    #endif

    return true;
}

bool32 OpenGL_ImGui_Shutdown() {
    #if defined(RH_IMGUI)
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    #endif

    return true;
}

bool32 OpenGL_ImGui_Begin_Frame() {
    #if defined(RH_IMGUI)
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    #endif

    return true;
}

bool32 OpenGL_ImGui_End_Frame() {
    #if defined(RH_IMGUI)
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    #endif
    
    return true;
}


//void OpenGL_swap_buffers() {
//    SwapBuffers(global_win32_state.device_context);
//}

void OpenGL_set_swap_interval(int32 interval) {
    AssertMsg(wglSwapIntervalEXT, "wglSwapIntervalEXT is NULL");
    
    wglSwapIntervalEXT(interval);
}
int32 OpenGL_get_swap_interval() {
    AssertMsg(wglGetSwapIntervalEXT, "wglGetSwapIntervalEXT is NULL");

    return wglGetSwapIntervalEXT();
}



void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id,
                                     GLenum severity, GLsizei length,
                                     const GLchar *msg, const void *data)
{
    char* _source;
    char* _type;
    char* _severity;

    switch (source) {
        case GL_DEBUG_SOURCE_API:
            _source = "API";
            break;

        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            _source = "WINDOW SYSTEM";
            break;

        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            _source = "SHADER COMPILER";
            break;

        case GL_DEBUG_SOURCE_THIRD_PARTY:
            _source = "THIRD PARTY";
            break;

        case GL_DEBUG_SOURCE_APPLICATION:
            _source = "APPLICATION";
            break;

        case GL_DEBUG_SOURCE_OTHER:
            _source = "UNKNOWN";
            break;

        default:
            _source = "UNKNOWN";
            break;
    }

    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            _type = "ERROR";
            break;

        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            _type = "DEPRECATED BEHAVIOR";
            break;

        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            _type = "UDEFINED BEHAVIOR";
            break;

        case GL_DEBUG_TYPE_PORTABILITY:
            _type = "PORTABILITY";
            break;

        case GL_DEBUG_TYPE_PERFORMANCE:
            _type = "PERFORMANCE";
            break;

        case GL_DEBUG_TYPE_OTHER:
            _type = "OTHER";
            break;

        case GL_DEBUG_TYPE_MARKER:
            _type = "MARKER";
            break;

        default:
            _type = "UNKNOWN";
            break;
    }

    log_level _level;

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            _severity = "HIGH";
            _level = LOG_LEVEL_FATAL;
            break;

        case GL_DEBUG_SEVERITY_MEDIUM:
            _severity = "MEDIUM";
            _level = LOG_LEVEL_ERROR;
            break;

        case GL_DEBUG_SEVERITY_LOW:
            _severity = "LOW";
            _level = LOG_LEVEL_WARN;
            break;

        case GL_DEBUG_SEVERITY_NOTIFICATION:
            _severity = "NOTIFICATION";
            _level = LOG_LEVEL_INFO;
            break;

        default:
            _severity = "UNKNOWN";
            _level = LOG_LEVEL_FATAL;
            break;
    }

    if (_level == LOG_LEVEL_INFO) return;

    LogOutput(_level, "%d: %s of %s severity, raised from %s: %s", id, _type, _severity, _source, msg);
}





int win32_get_monitor_refresh_rate(HWND window) {
    int MonitorRefreshHz = 0;
    HMONITOR CurrentMonitor = MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);
    MONITORINFOEXA MonitorInfo;
    MonitorInfo.cbSize = { sizeof(MONITORINFOEX) };
    if (GetMonitorInfoA(CurrentMonitor, &MonitorInfo)) {
        DEVMODEA MonitorDeviceMode = { sizeof(DEVMODEA) };
        if (EnumDisplaySettingsA(MonitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &MonitorDeviceMode)) {
            MonitorRefreshHz = MonitorDeviceMode.dmDisplayFrequency;
        }
    }
    return MonitorRefreshHz;
}

#if 0
static void win32_update_vsync(uint32* MonitorRefreshHz, uint32* GameRefreshHz) {
    if (*GameRefreshHz > *MonitorRefreshHz) {
        // game update rate is higher than the refresh rate, can't do vsync
        RH_WARN("Requesting %dHz, but the Monitor is only %dHz!\nRun at own risk...", *GameRefreshHz, *MonitorRefreshHz);
    } else {
        int SwapInterval = 0;
        if (((*MonitorRefreshHz) % (*GameRefreshHz)) == 0 ) {
            // monitor refresh rate is an even multiple of the game refresh rate
            SwapInterval = (*MonitorRefreshHz) / (*GameRefreshHz);
            wglSwapIntervalEXT(SwapInterval);
        } else {
            // not an even multiple -> change the game refresh rate to a lower multiple of the monitor rate
            SwapInterval = ((*MonitorRefreshHz) / (*GameRefreshHz)) + 1;
            *GameRefreshHz = (*MonitorRefreshHz) / SwapInterval;
            RH_INFO("Changing game update Hz to %d to be an even multiple of the Monitor update Hz [%d].", *GameRefreshHz, *MonitorRefreshHz);
            wglSwapIntervalEXT(SwapInterval);
        }

        RH_INFO("Swap interval: %d", SwapInterval);
    }
}
#endif

#endif