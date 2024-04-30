#pragma once

#include "Engine/Renderer/Render_Types.h"

struct debug_geometry {
    struct debug_vertex {
        laml::Vec3 position;
        laml::Vec3 normal;
    };

    uint32 num_verts;
    uint32 num_inds;
    debug_vertex* vertices;
    uint32* indices;
};

enum class mesh_file_result : int32 {
    error = -1,
    is_static,
    is_skinned
};

struct resource_texture_2D {
    render_texture_2D texture;

    uint16 width;
    uint16 height;
    Texture_Format format;
};
struct resource_texture_3D {
    render_texture_3D texture;

    uint16 width;
    uint16 height;
    uint16 depth;
    Texture_Format format;
};
struct resource_texture_cube {
    render_texture_cube texture;

    uint16 width;
    uint16 height;
    Texture_Format format;
};

struct resource_material {
    laml::Vec3 DiffuseFactor;
    real32 NormalScale;
    real32 AmbientStrength;
    real32 MetallicFactor;
    real32 RoughnessFactor;
    laml::Vec3 EmissiveFactor;
    uint32 flag;

    resource_texture_2D DiffuseTexture;
    resource_texture_2D NormalTexture;
    resource_texture_2D AMRTexture;
    resource_texture_2D EmissiveTexture;
};

struct resource_static_mesh {
    uint32 num_primitives;
    render_geometry *primitives;
    resource_material *materials;

    //laml::Mat4 transform; // unused! maybe as a default mesh offset?
};
struct resource_bone {
    uint32 bone_idx;
    int32  parent_idx;
    laml::Mat4 local_matrix;
    laml::Mat4 inv_model_matrix;

    real32 debug_length;
    char* debug_name;
};
struct resource_skeleton {
    uint32 num_bones;
    resource_bone* bones;
};
struct resource_skinned_mesh {
    uint32 num_primitives;
    render_geometry *primitives;
    resource_material *materials;
    resource_skeleton skeleton;

    //laml::Mat4 transform; // unused! maybe as a default mesh offset?
};

struct level_data {
    uint32 num_geometry;
    uint32 num_colliders;

    render_geometry *geometry;
    laml::Mat4 *geo_transforms;

    render_geometry *colliders;
    laml::Mat4 *collider_transforms;

    collision_grid grid;
};

struct bone_anim {
    laml::Vec3* translation;
    laml::Quat* rotation;
    laml::Vec3* scale;
};
struct resource_animation {
    char* name;

    uint16 num_bones;
    uint16 num_samples;
    real32 frame_rate;
    real32 length;

    bone_anim* bones;
};

struct resource_env_map {
    // original equirectangular texture
    resource_texture_2D src;

    // final output
    render_env_map map;

    // cubemap dims (all square, rgb32f)
    uint16 skybox_size;     // 1024
    uint16 irradiance_size; // 32
    uint16 prefilter_size;  // 128

};