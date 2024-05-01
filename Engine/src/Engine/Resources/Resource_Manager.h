#pragma once

#include "Engine/Resources/Resource_Types.h"

struct collision_grid;
struct mesh_file;

/* Resource_Manager.cpp */
bool32 resource_init(memory_arena* arena);
void resource_shutdown();
RHAPI memory_arena* resource_get_arena(); // remove api for this
RHAPI bool32 resource_load_debug_mesh_into_geometry(const char* resource_file_name, render_geometry* geom);
RHAPI bool32 resource_load_debug_mesh_data(const char* resource_file_name, debug_geometry* geom);

/* mesh_loader.cpp */
RHAPI bool32 resource_load_static_mesh( const char* resource_file_name, resource_static_mesh*  mesh);
RHAPI bool32 resource_load_skinned_mesh(const char* resource_file_name, resource_skinned_mesh* mesh);

/* anim_loader.cpp */
RHAPI bool32 resource_load_animation(const char* resource_file_name, resource_animation* mesh);

/* Level_Loader.cpp */
// RHAPI bool32 resource_load_level_file(const char* resource_file_name, level_data* data);

/* Shader_Loader.cpp */
bool32 resource_load_shader_file(const char* resource_file_name,
                                 shader* shader_prog);

/* Image_Loader.cpp */
RHAPI bool32 resource_load_texture_file(const char* resource_file_name,
                                        resource_texture_2D* texture);

RHAPI bool32 resource_load_texture_cube_map_from_files(const char* face_file_names[6],
                                                       resource_texture_cube* texture);
RHAPI bool32 resource_load_texture_debug_cube_map(resource_texture_cube* texture);

//RHAPI bool32 resource_load_texture_cube_map_with_mips(const char* resource_file_base_name,
//                                            resource_texture_cube* texture,
//                                            uint32 mip_levels);
//RHAPI bool32 resource_load_texture_cube_map_hdr_with_mips(const char* resource_file_base_name,
//                                            resource_texture_cube* texture,
//                                            uint32 mip_levels);

//RHAPI bool32 resource_load_env_map(const char* resource_file_name,
//                                   resource_env_map* env_map);

//RHAPI bool32 resource_write_texture_file(const char* resource_file_name,
//                                         resource_texture_2D texture,
//                                         bool32 is_hdr);
//RHAPI bool32 resource_write_cubemap_file(const char* resource_file_name,
//                                         resource_texture_cube texture,
//                                         bool32 is_hdr,
//                                         bool32 write_mips);
