#include "Resource_Manager.h"

#include "Engine/Core/Logger.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Core/String.h"
#include "Engine/Core/Asserts.h"
#include "Engine/Core/Timing.h"

#include "Engine/Memory/Memory_Arena.h"

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

bool32 resource_load_texture_file(const char* resource_file_name,
                                  resource_texture_2D* texture) {

    char full_path[256];
    platform_get_full_resource_path(full_path, 256, resource_file_name);

    RH_TRACE("Full filename: [%s]", full_path);

    file_handle file = platform_read_entire_file(full_path);
    if (!file.num_bytes) {
        RH_ERROR("Failed to read resource file");
        return false;
    }

    memory_arena* arena = resource_get_arena();

    stbi_set_flip_vertically_on_load(true);
    // TODO: This calls malloc under the hood!
    //       can be overwritten with #define STBI_MALLOC ...
    int x,y,n;
    unsigned char *bitmap = stbi_load_from_memory(file.data, (int)file.num_bytes, &x, &y, &n, 4);
    platform_free_file_data(&file);
    if (bitmap == NULL) {
        RH_ERROR("Failed to load image file!");
        return false;
    }

    texture->width = (uint16)x;
    texture->height = (uint16)y;
    texture->num_channels = (uint16)n;
    texture->is_hdr = 0;

    texture_creation_info_2D info;
    info.width = texture->width;
    info.height = texture->height;
    info.num_channels = texture->num_channels;
    renderer_create_texture(&texture->texture, info, bitmap, false);

    stbi_image_free(bitmap);

    return true;
}


bool32 resource_load_texture_file_hdr(const char* resource_file_name,
                                      resource_texture_2D* texture) {

    char full_path[256];
    platform_get_full_resource_path(full_path, 256, resource_file_name);

    RH_TRACE("Full filename: [%s]", full_path);

    file_handle file = platform_read_entire_file(full_path);
    if (!file.num_bytes) {
        RH_ERROR("Failed to read resource file");
        return false;
    }

    memory_arena* arena = resource_get_arena();

    stbi_set_flip_vertically_on_load(true);
    // TODO: This calls malloc under the hood!
    //       can be overwritten with #define STBI_MALLOC ...
    int x,y,n;
    real32 *data = stbi_loadf_from_memory(file.data, (int)file.num_bytes, &x, &y, &n, 0);
    platform_free_file_data(&file);
    if (data == NULL) {
        RH_ERROR("Failed to load image file!");
        return false;
    }

    texture->width = (uint16)x;
    texture->height = (uint16)y;
    texture->num_channels = (uint16)n;
    texture->is_hdr = 1;

    texture_creation_info_2D info;
    info.width = texture->width;
    info.height = texture->height;
    info.num_channels = texture->num_channels;
    renderer_create_texture(&texture->texture, info, data, true);

    stbi_image_free(data);

    return true;
}

bool32 resource_load_texture_debug_cube_map(resource_texture_cube* texture) {
    char* cube_sides[] = {
        "Data/textures/pos_x.png",
        "Data/textures/neg_x.png",
        "Data/textures/pos_y.png",
        "Data/textures/neg_y.png",
        "Data/textures/pos_z.png",
        "Data/textures/neg_z.png",
    };

    char full_path[256];
    memory_arena* arena = resource_get_arena();

    stbi_set_flip_vertically_on_load(true);
    int x[6], y[6], n[6];
    uint8*** bitmaps;
    bitmaps = PushArray(arena, uint8**, 6);
    for (uint32 face = 0; face < 6; face++) {
        bitmaps[face] = PushArray(arena, uint8*, 1);

        platform_get_full_resource_path(full_path, 256, cube_sides[face]);

        RH_TRACE("Full filename: [%s]", full_path);

        file_handle file = platform_read_entire_file(full_path);
        if (!file.num_bytes) {
            RH_ERROR("Failed to read resource file");
            return false;
        }

        // TODO: This calls malloc under the hood!
        //       can be overwritten with #define STBI_MALLOC ...
        bitmaps[face][0] = stbi_load_from_memory(file.data, (int)file.num_bytes, &x[face], &y[face], &n[face], 0);
        platform_free_file_data(&file);
        if (bitmaps[face][0] == NULL) {
            RH_ERROR("Failed to load image file!");
            return false;
        }
    }

    texture->width = (uint16)x[0];
    texture->height = (uint16)y[0];
    texture->num_channels = (uint16)n[0];
    texture->is_hdr = 0;

    texture_creation_info_cube info;
    info.width = texture->width;
    info.height = texture->height;
    info.num_channels = texture->num_channels;
    renderer_create_texture_cube(&texture->texture, info, (const void***)(bitmaps), false);

    for (uint32 face = 0; face < 6; face++) {
        stbi_image_free(bitmaps[face][0]);
    }

    return true;
}

bool32 resource_write_env_map_metadata(const char* path, resource_env_map* env_map);
bool32 resource_load_env_map(const char* resource_file_name,
                             resource_env_map* env_map) {
    const bool32 save_bake_to_disk = false; // Disable! this is slow at the moment and doesn't speed of loading lol

    char full_path[256];
    platform_get_full_resource_path(full_path, 256, resource_file_name);

    RH_TRACE("Full filename: [%s]", full_path);

    memory_arena* arena = resource_get_arena();

    // first check if pre-computed data exists (doesn't for now...)
    char baked_path[256];
    string_build(baked_path, 256, "%s_bake", full_path);

    file_info finfo;
    if (platform_get_file_attributes(baked_path, &finfo)) {
        time_point start_load = start_timer();

        // pre-baked data exists! load meta-data file then load cubemaps into memory
        RH_ERROR("pre-baked files not implemented yet!");

        // TODO: read metadata from the bake file.
        //       for now, just hard-code values
        uint32 num_mips = 6;

        resource_texture_cube tmp;

        // skybox
        string_build(baked_path, 256, "%s_skybox", full_path);
        resource_load_texture_cube_map_hdr_with_mips(baked_path, &tmp, 1);
        env_map->map.skybox = tmp.texture;
        env_map->skybox_size = tmp.width;

        string_build(baked_path, 256, "%s_irradiance", full_path);
        resource_load_texture_cube_map_hdr_with_mips(baked_path, &tmp, 1);
        env_map->map.irradiance  = tmp.texture;
        env_map->irradiance_size = tmp.width;

        // This needs to load mip-maps
        string_build(baked_path, 256, "%s_prefilter", full_path);
        resource_load_texture_cube_map_hdr_with_mips(baked_path, &tmp, 6);
        env_map->map.prefilter  = tmp.texture;
        env_map->prefilter_size = tmp.width;

        // print time it took to load.
        RH_INFO("Took %.3f ms to load baked IBL", measure_elapsed_time(start_load)*1000.0f);
        return false;

    } else {
        // not yet baked, need to generate the environment map data
        time_point start_bake = start_timer();

        file_handle file = platform_read_entire_file(full_path);
        if (!file.num_bytes) {
            RH_ERROR("Failed to read resource file");
            return false;
        }

        stbi_set_flip_vertically_on_load(true);
        // TODO: This calls malloc under the hood!
        //       can be overwritten with #define STBI_MALLOC ...
        int x,y,n;
        real32 *data = stbi_loadf_from_memory(file.data, (int)file.num_bytes, &x, &y, &n, 0);
        platform_free_file_data(&file);
        if (data == NULL) {
            RH_ERROR("Failed to load image file!");
            return false;
        }

        env_map->src.width = (uint16)x;
        env_map->src.height = (uint16)y;
        env_map->src.num_channels = (uint16)n;
        env_map->src.is_hdr = 1;

        env_map->skybox_size     = 1024;
        env_map->irradiance_size = 32;
        env_map->prefilter_size  = 128;

        renderer_precompute_env_map_from_equirectangular(env_map, data);

        RH_INFO("Took %.3f ms to bake IBL", measure_elapsed_time(start_bake)*1000.0f);

        stbi_image_free(data);

        //
        // write to file
        //
        if (save_bake_to_disk) {
            resource_write_env_map_metadata(baked_path, env_map);

            resource_texture_cube tmp;

            string_build(baked_path, 256, "%s_skybox", full_path);
            tmp.texture = env_map->map.skybox;
            tmp.width = env_map->skybox_size;
            tmp.height = env_map->skybox_size;
            tmp.num_channels = 3; // TODO: this might not be the case
            resource_write_cubemap_file(baked_path, tmp, true, false);

            string_build(baked_path, 256, "%s_irradiance", full_path);
            tmp.texture = env_map->map.irradiance;
            tmp.width = env_map->irradiance_size;
            tmp.height = env_map->irradiance_size;
            tmp.num_channels = 3; // TODO: this might not be the case
            resource_write_cubemap_file(baked_path, tmp, true, false);

            string_build(baked_path, 256, "%s_prefilter", full_path);
            tmp.texture = env_map->map.prefilter;
            tmp.width = env_map->prefilter_size;
            tmp.height = env_map->prefilter_size;
            tmp.num_channels = 3; // TODO: this might not be the case
            resource_write_cubemap_file(baked_path, tmp, true, true);

            RH_INFO("Took %.3f ms to bake IBL and save to file", measure_elapsed_time(start_bake)*1000.0f);
        }
    }

    return true;
}


bool32 resource_write_texture_file(const char* resource_file_name,
                                   resource_texture_2D texture,
                                   bool32 is_hdr) {
    memory_arena* arena = resource_get_arena();

    uint64 type_size = is_hdr ? sizeof(real32) : sizeof(uint8);
    uint64 num = texture.width * texture.height * texture.num_channels;
    void* data = PushSize_(arena, num * type_size);
    renderer_get_texture_data(texture.texture, data, texture.num_channels, texture.is_hdr, 0);

    stbi_flip_vertically_on_write(true);

    int res;
    if (is_hdr) {
        res = stbi_write_hdr(resource_file_name, texture.width, texture.height, texture.num_channels, (float*)data);
    } else {
        res = stbi_write_bmp(resource_file_name, texture.width, texture.height, texture.num_channels, data);
    }
    
    return true;
}

bool32 resource_write_cubemap_file(const char* resource_base_file_name,
                                   resource_texture_cube texture,
                                   bool32 is_hdr,
                                   bool32 write_mips) {

    memory_arena* arena = resource_get_arena();

    char face_filename[256];

    uint64 type_size = is_hdr ? sizeof(real32) : sizeof(uint8);
    const char* ext  = is_hdr ? ".hdr" : ".bmp";
    uint64 num = texture.width * texture.height * texture.num_channels;
    void* data = PushSize_(arena, num * type_size);

    stbi_flip_vertically_on_write(true);

    if (write_mips) {
        uint32 num_mips = 6;
        for (uint32 face = 0; face < 6; face++) {
            for (uint32 mip = 0; mip < num_mips; mip++) {
                string_build(face_filename, 256, "%s_%u_mip%u%s", resource_base_file_name, face, mip, ext);
                renderer_get_cubemap_data(texture.texture, data, texture.num_channels, is_hdr, face, mip);

                uint32 mip_width  = (uint32)((real32)(texture.width)  * std::pow(0.5, mip));
                uint32 mip_height = (uint32)((real32)(texture.height) * std::pow(0.5, mip));

                int res;
                if (is_hdr) {
                    res = stbi_write_hdr(face_filename, mip_width, mip_height, texture.num_channels, (float*)data);
                } else {
                    res = stbi_write_bmp(face_filename, mip_width, mip_height, texture.num_channels, data);
                }
            }
        }
    } else {
        for (uint32 face = 0; face < 6; face++) {
            string_build(face_filename, 256, "%s_%u%s", resource_base_file_name, face, ext);
            renderer_get_cubemap_data(texture.texture, data, texture.num_channels, is_hdr, face, 0);

            int res;
            if (is_hdr) {
                res = stbi_write_hdr(face_filename, texture.width, texture.height, texture.num_channels, (float*)data);
            } else {
                res = stbi_write_bmp(face_filename, texture.width, texture.height, texture.num_channels, data);
            }
        }
    }
    
    return true;
}

bool32 resource_load_texture_cube_map_with_mips(const char* resource_file_base_name,
                                                resource_texture_cube* texture,
                                                uint32 mip_levels) {
    memory_arena* arena = resource_get_arena();

    char face_name[256];

    stbi_set_flip_vertically_on_load(true);

    const char* ext  = ".bmp";

    if (mip_levels < 1) mip_levels = 1;
    bool32 has_mips = mip_levels > 1;

    int cube_width = -1, cube_height = -1, num_channels = -1;
    uint8*** bitmaps;
    bitmaps = PushArray(arena, uint8**, 6);
    for (uint32 face = 0; face < 6; face++) {
        bitmaps[face] = PushArray(arena, uint8*, mip_levels);

        for (uint32 mip = 0; mip < mip_levels; mip++) {
            if (has_mips)
                string_build(face_name, 256, "%s_%u_mip%u%s", resource_file_base_name, face, mip, ext);
            else
                string_build(face_name, 256, "%s_%u%s", resource_file_base_name, face, ext);
            RH_DEBUG("Full filename: [%s]", face_name);

            file_handle file = platform_read_entire_file(face_name);
            if (!file.num_bytes) {
                RH_ERROR("Failed to read resource file");
                return false;
            }

            // TODO: This calls malloc under the hood!
            //       can be overwritten with #define STBI_MALLOC ...
            int face_width, face_height, face_num_channels;
            bitmaps[face][mip] = stbi_load_from_memory(file.data, (int)file.num_bytes, &face_width, &face_height, &face_num_channels, 0);
            if (face == 0 && mip == 0) {
                cube_width = face_width;
                cube_height = face_height;
                num_channels = face_num_channels;
            } else {
                int32 mip_width  = (int32)((real32)(cube_width)  * std::pow(0.5, mip));
                int32 mip_height = (int32)((real32)(cube_height) * std::pow(0.5, mip));

                AssertMsg(face_width == mip_width, "Cubemap Face has different width than the rest");
                AssertMsg(face_height == mip_height, "Cubemap Face has different height than the rest");
                AssertMsg(face_num_channels == num_channels, "Cubemap Face has different num_channels than the rest");
            }
            platform_free_file_data(&file);

            if (bitmaps[face][mip] == NULL) {
                RH_ERROR("Failed to load image file!");
                return false;
            }
        }
    }

    texture->width        = (uint16)cube_width;
    texture->height       = (uint16)cube_height;
    texture->num_channels = (uint16)num_channels;
    texture->is_hdr       = 0;

    texture_creation_info_cube info;
    info.width        = texture->width;
    info.height       = texture->height;
    info.num_channels = texture->num_channels;
    renderer_create_texture_cube(&texture->texture, info, (const void***)(bitmaps), false, mip_levels);

    for (uint32 face = 0; face < 6; face++) {
        for (uint32 mip = 0; mip < mip_levels; mip++) {
            stbi_image_free(bitmaps[face][mip]);
        }
    }

    return true;
}
bool32 resource_load_texture_cube_map_hdr_with_mips(const char* resource_file_base_name,
                                                    resource_texture_cube* texture,
                                                    uint32 mip_levels) {
    memory_arena* arena = resource_get_arena();

    char face_name[256];

    stbi_set_flip_vertically_on_load(true);

    const char* ext  = ".hdr";

    if (mip_levels < 1) mip_levels = 1;
    bool32 has_mips = mip_levels > 1;

    int cube_width = -1, cube_height = -1, num_channels = -1;
    real32*** bitmaps;
    bitmaps = PushArray(arena, real32**, 6);
    for (uint32 face = 0; face < 6; face++) {
        bitmaps[face] = PushArray(arena, real32*, mip_levels);

        for (uint32 mip = 0; mip < mip_levels; mip++) {
            if (has_mips)
                string_build(face_name, 256, "%s_%u_mip%u%s", resource_file_base_name, face, mip, ext);
            else
                string_build(face_name, 256, "%s_%u%s", resource_file_base_name, face, ext);
            RH_DEBUG("Full filename: [%s]", face_name);

            file_handle file = platform_read_entire_file(face_name);
            if (!file.num_bytes) {
                RH_ERROR("Failed to read resource file");
                return false;
            }

            // TODO: This calls malloc under the hood!
            //       can be overwritten with #define STBI_MALLOC ...
            int face_width, face_height, face_num_channels;
            bitmaps[face][mip] = stbi_loadf_from_memory(file.data, (int)file.num_bytes, &face_width, &face_height, &face_num_channels, 0);
            if (face == 0 && mip == 0) {
                cube_width = face_width;
                cube_height = face_height;
                num_channels = face_num_channels;
            } else {
                int32 mip_width  = (int32)((real32)(cube_width)  * std::pow(0.5, mip));
                int32 mip_height = (int32)((real32)(cube_height) * std::pow(0.5, mip));

                AssertMsg(face_width == mip_width,           "Cubemap Face has different width than the rest, or incorrect mip-map size");
                AssertMsg(face_height == mip_height,         "Cubemap Face has different height than the rest, or incorrect mip-map size");
                AssertMsg(face_num_channels == num_channels, "Cubemap Face has different num_channels than the rest");
            }
            platform_free_file_data(&file);

            if ( bitmaps[face][mip] == NULL) {
                RH_ERROR("Failed to load image file!");
                return false;
            }
        }
    }

    texture->width        = (uint16)cube_width;
    texture->height       = (uint16)cube_height;
    texture->num_channels = (uint16)num_channels;
    texture->is_hdr       = 1;

    texture_creation_info_cube info;
    info.width        = texture->width;
    info.height       = texture->height;
    info.num_channels = texture->num_channels;
    renderer_create_texture_cube(&texture->texture, info, (const void***)(bitmaps), true, mip_levels);

    for (uint32 face = 0; face < 6; face++) {
        for (uint32 mip = 0; mip < mip_levels; mip++) {
            stbi_image_free(bitmaps[face][mip]);
        }
    }

    return true;
}

// TODO: remove STD lib
#include <stdio.h>
bool32 resource_write_env_map_metadata(const char* path, resource_env_map* env_map) {
    FILE* fid = fopen(path, "w");
    if (fid) {
        fprintf(fid, "bleh\r\n");



        fclose(fid);
    }

    return true;
}