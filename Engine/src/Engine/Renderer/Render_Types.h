#pragma once

#include "Engine/Defines.h"
#include <laml/laml.hpp>

struct memory_arena;
struct triangle_geometry;
struct shader;

enum renderer_api_type {
    RENDERER_API_OPENGL,
};

enum class ShaderDataType : uint8 {
    None = 0, 
    Float, Float2, Float3, Float4,
    Mat3, Mat4, 
    Int, Int2, Int3, Int4,
    Bool
};

struct renderer_api {
    //struct platform_state* plat_state; // platform-specific state
    uint64 frame_number;

    virtual bool32 initialize(const char* application_name, struct platform_state* plat_state) = 0;
    virtual void shutdown() = 0;
    virtual void resized(uint16 width, uint16 height) = 0;

    virtual bool32 begin_frame(real32 delta_time) = 0;
    virtual bool32 end_frame(real32 delta_time) = 0;

    virtual void create_texture(struct texture_2D* texture, const uint8* data) = 0;
    virtual void destroy_texture(struct texture_2D* texture) = 0;
    virtual void create_mesh(triangle_geometry* mesh, 
                             uint32 num_verts, const void* vertices,
                             uint32 num_inds, const uint32* indices,
                             const ShaderDataType* attributes) = 0;
    virtual void destroy_mesh(triangle_geometry* mesh) = 0;
    virtual bool32 create_shader(shader* shader_prog, const uint8* shader_source, uint64 num_bytes) = 0;
    virtual void destroy_shader(shader* shader_prog) = 0;

    virtual void use_shader(shader* shader_prog) = 0;
    virtual void draw_geometry(triangle_geometry* geom) = 0;

    // uniforms
    virtual void upload_uniform_float( shader* shader_prog, const char* uniform_name, float  value) = 0;
    virtual void upload_uniform_float2(shader* shader_prog, const char* uniform_name, float* values) = 0;
    virtual void upload_uniform_float3(shader* shader_prog, const char* uniform_name, float* values) = 0;
    virtual void upload_uniform_float4(shader* shader_prog, const char* uniform_name, float* values) = 0;
    virtual void upload_uniform_float4x4(shader* shader_prog, const char* uniform_name, float* values) = 0;
    virtual void upload_uniform_int( shader* shader_prog, const char* uniform_name, int  value) = 0;
    virtual void upload_uniform_int2(shader* shader_prog, const char* uniform_name, int* values) = 0;
    virtual void upload_uniform_int3(shader* shader_prog, const char* uniform_name, int* values) = 0;
    virtual void upload_uniform_int4(shader* shader_prog, const char* uniform_name, int* values) = 0;
};

// the actual geometry that the gpu holds onto
// separately, a 'resource' will exist to 
// represent an actual mesh.
struct triangle_geometry {
    uint32 handle; // handle to the gpu version of this data

    uint32 num_verts;
    uint32 num_inds;
    uint32 flag;
};

struct texture_2D {
    uint32 handle; // handle to the gpu version of this data

    uint16 width;
    uint16 height;
    uint16 num_channels;
    uint16 flag;
};

struct shader {
    uint32 handle; // handle to the gpu version of this data
};

struct render_command {
    laml::Mat4 model_matrix;
    triangle_geometry geom;
    uint32 material_handle;
};

struct render_packet {
    memory_arena* arena;

    real32 delta_time;

    laml::Mat4 projection_matrix;
    laml::Mat4 view_matrix;
    laml::Vec3 camera_pos;

    uint32 num_commands;
    render_command* commands;
};