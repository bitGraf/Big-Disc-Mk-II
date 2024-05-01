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
    uint64 path_len = platform_get_full_resource_path(full_path, 256, resource_file_name);

    RH_TRACE("Full filename: [%s]", full_path);

    file_handle file = platform_read_entire_file(full_path);
    if (!file.num_bytes) {
        RH_ERROR("Failed to read resource file");
        return false;
    }

    memory_arena* arena = resource_get_arena();

    // check if a .dds file type. If so, use the directx loading function
    char* ext;
    for (ext = &full_path[path_len - 1]; (*ext != '.') && (ext != full_path); ext--) {}

    Texture_Format format = TEXTURE_FORMAT_UNKNOWN;
    int width,height;
    void* data = nullptr;
    bool stbi = false;

    if (string_compare(ext, ".dds") == 0) {
        width = 0;
        height = 0;
        data = 0;
    } else {
        stbi = true; // to cleanup

        int num_channels = 0;
        stbi_set_flip_vertically_on_load(true);
        // TODO: This calls malloc under the hood!
        //       can be overwritten with #define STBI_MALLOC ...

        if (string_compare(ext, ".hdr") == 0) {
            float* f32_data = stbi_loadf_from_memory(file.data, (int)file.num_bytes, &width, &height, &num_channels, 0);

            switch (num_channels) {
                case 1: format = TEXTURE_FORMAT_R_FLOAT32;    break;
                case 2: format = TEXTURE_FORMAT_RG_FLOAT32;   break;
                case 3: format = TEXTURE_FORMAT_RGB_FLOAT32;  break;
                case 4: format = TEXTURE_FORMAT_RGBA_FLOAT32; break;
            }
            data = f32_data;
        } else {
            // NOTE: 3-channel (24-bpp) formats are not easily supported by the GPU, so if the 
            //       file has 3 channels, then we request a 4th channel.
            uint8* bitmap = stbi_load_from_memory(file.data, (int)file.num_bytes, &width, &height, &num_channels, 0);

            switch (num_channels) {
                case 1: format = TEXTURE_FORMAT_R_U8_NORM;    break;
                case 2: format = TEXTURE_FORMAT_RG_U8_NORM;   break;
                case 4: format = TEXTURE_FORMAT_RGBA_U8_NORM; break;
            }

            // if 3 channels in file, reload but request 4 channels instead.
            if (num_channels == 3) {
                stbi_image_free(bitmap); // free old data, since we alloc some new data
                bitmap = stbi_load_from_memory(file.data, (int)file.num_bytes, &width, &height, NULL, 4);
                format = TEXTURE_FORMAT_RGBA_U8_NORM;
            }

            data = bitmap;
        }

        if (data == NULL) {
            RH_ERROR("Failed to load image file!");
            return false;
        }
    }

    AssertMsg(format != TEXTURE_FORMAT_UNKNOWN, "Could not determine the format!");

    texture_creation_info_2D info;
    info.width  = static_cast<uint16>(width);
    info.height = static_cast<uint16>(height);
    info.format = format;
    info.data = data;
    info.mip_levels = 1;
    renderer_create_texture(&texture->texture, info);

    platform_free_file_data(&file);
    if (stbi) {
        stbi_image_free(data);
    }

    return true;
}

bool32 resource_load_texture_cube_map_from_files(const char* face_file_names[6],
                                                       resource_texture_cube* texture) {
    
    stbi_set_flip_vertically_on_load(true);
    int cube_width = -1, cube_height = -1; 
    Texture_Format cube_format = TEXTURE_FORMAT_UNKNOWN;
    void* cube_data[6] = {};
    bool stbi = false;

    for (uint32 n = 0; n < 6; n++) {
        char full_path[256];
        uint64 path_len = platform_get_full_resource_path(full_path, 256, face_file_names[n]);

        RH_TRACE("Full filename: [%s]", full_path);

        file_handle file = platform_read_entire_file(full_path);
        if (!file.num_bytes) {
            RH_ERROR("Failed to read resource file");
            return false;
        }

        // check if a .dds file type. If so, use the directx loading function
        char* ext;
        for (ext = &full_path[path_len - 1]; (*ext != '.') && (ext != full_path); ext--) {}

        if (string_compare(ext, ".dds") == 0) {
            return false;
        }

        Texture_Format face_format = TEXTURE_FORMAT_UNKNOWN;
        int face_width,face_height;
        stbi = true;

        int face_num_channels = 0;

        // TODO: These call malloc under the hood!
        //       can be overwritten with #define STBI_MALLOC ...
        if (string_compare(ext, ".hdr") == 0) {
            float* f32_data = stbi_loadf_from_memory(file.data, (int)file.num_bytes, &face_width, &face_height, &face_num_channels, 0);

            switch (face_num_channels) {
                case 1: face_format = TEXTURE_FORMAT_R_FLOAT32; break;
                case 2: face_format = TEXTURE_FORMAT_RG_FLOAT32; break;
                case 3: face_format = TEXTURE_FORMAT_RGB_FLOAT32; break;
                case 4: face_format = TEXTURE_FORMAT_RGBA_FLOAT32; break;
            }
            cube_data[n] = f32_data;
        } else {
            // NOTE: 3-channel formats are not easily supported by the GPU, so for now we request
            //       stbi to return a 4-channel texture always. 
            //       A better way might be to load normally, then if its a 3-channel call
            //       stbi_load again and request a 4-channel in that case, so that we can
            //       still support 1- and 2-channel textures. Not sure if I want to do this, since I 
            //       would prefer to swap everything to .dds anyway...
            uint8* bitmap = stbi_load_from_memory(file.data, (int)file.num_bytes, &face_width, &face_height, &face_num_channels, 4);

            switch (face_num_channels) {
                case 1: face_format = TEXTURE_FORMAT_R_U8_NORM; break;
                case 2: face_format = TEXTURE_FORMAT_RG_U8_NORM; break;
                case 4: face_format = TEXTURE_FORMAT_RGBA_U8_NORM; break;
            }
            cube_data[n] = bitmap;
        }

        if (cube_width == -1 && cube_height == -1 && cube_format == TEXTURE_FORMAT_UNKNOWN) {
            cube_width = face_width;
            cube_height = face_height;
            cube_format = face_format;
        } else if ((cube_width != face_width) || (cube_height != face_height) || (cube_format != face_format)) {
            RH_ERROR("Cube face mismatch!. Cube is [%ux%u:%u], but face %u is [%ux%u:%u].\n",
                     cube_width, cube_height, (uint32)cube_format,
                     n,
                     face_width, face_height, (uint32)face_format);
            return false;
        }

        if (cube_data == NULL) {
            RH_ERROR("Failed to load image file!");
            return false;
        }
    }

    texture_creation_info_cube info;
    info.width  = (uint16)cube_width;
    info.height = (uint16)cube_height;
    info.format = cube_format;
    info.mip_levels = 1;
    for (uint32 face = 0; face < 6; face++) {
        info.data[face] = cube_data[face];
    }
    renderer_create_texture_cube(&texture->texture, info);

    if (stbi) {
        for (uint32 face = 0; face < 6; face++) {
            stbi_image_free(info.data[face]);
        }
    }

    return true;
}

bool32 resource_load_texture_debug_cube_map(resource_texture_cube* texture) {
    const char* cube_sides[] = {
        "Data/textures/pos_x.png",
        "Data/textures/neg_x.png",
        "Data/textures/pos_y.png",
        "Data/textures/neg_y.png",
        "Data/textures/pos_z.png",
        "Data/textures/neg_z.png",
    };

    return resource_load_texture_cube_map_from_files(cube_sides, texture);
}

#if 0
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
    //info.num_channels = texture->num_channels;
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
    //info.num_channels = texture->num_channels;
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
#endif