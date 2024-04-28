#include "Resource_Manager.h"

#include "Engine/Core/Logger.h"
#include "Engine/Core/String.h"
#include "Engine/Platform/Platform.h"

#include "Engine/Renderer/Renderer.h"

bool32 resource_load_shader_file(const char* resource_file_name,
                                 shader* shader_prog) {

    char full_path[256];
    uint64 len = platform_get_full_resource_path(full_path, 256, resource_file_name);

    renderer_api_type backend_type = renderer_backend_type();
    switch (backend_type) {
        case RENDERER_API_OPENGL:     full_path[len-4] = 'g'; break;
        case RENDERER_API_DIRECTX_12: full_path[len-4] = 'h'; break;
    }

    RH_TRACE("Full filename: [%s]", full_path);

    file_handle file = platform_read_entire_file(full_path);
    if (!file.num_bytes) {
        RH_ERROR("Failed to read resource file");
        return false;
    }

    memory_arena* arena = resource_get_arena();

    bool32 result = renderer_create_shader(shader_prog, file.data, file.num_bytes);
    platform_free_file_data(&file);

    return result;
}