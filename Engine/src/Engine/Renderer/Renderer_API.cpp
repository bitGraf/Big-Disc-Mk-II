#include "Renderer_API.h"

#include "Engine/Core/Logger.h"
#include "Engine/Memory/Memory_Arena.h"

// Render apis
#include "Engine/Renderer/OpenGL/OpenGL_API.h"
#include "Engine/Renderer/DirectX12/DirectX12_API.h"

// Need this for palcement new?
#include <new>

bool32 renderer_api_create(memory_arena* arena, renderer_api_type type, platform_state* plat_state, renderer_api** out_api) {
    switch(type) {
        case RENDERER_API_OPENGL: {
            OpenGL_api* api = PushStruct(arena, OpenGL_api);
            RH_INFO("Creating OpenGL Renderer");
            
            api = new(api) OpenGL_api;
            //out_api->plat_state = plat_state;
            api->frame_number = 0;

            *out_api = api;

            return true;
        }

        case RENDERER_API_DIRECTX_12: {
            DirectX12_api* api = PushStruct(arena, DirectX12_api);
            RH_INFO("Creating Directx12 Renderer");

            api = new(api) DirectX12_api;
            //out_api->plat_state = plat_state;
            api->frame_number = 0;

            *out_api = api;

            return true;
        }

        default:
            RH_FATAL("Unsupported renderer backend!");
    }

    RH_FATAL("Failed to create renderer backend!");
    return false;
}

void renderer_api_destroy(renderer_api* api) {
}