#pragma once

#include "../Renderer_API.h"

struct OpenGL_api final : public renderer_api {
    bool32 initialize(const char* application_name, struct platform_state* plat_state) override final;
    void shutdown() override final;

    void resized(uint16 width, uint16 height) override final;

    bool32 begin_frame(real32 delta_time) override final;
    bool32 end_frame(real32 delta_time) override final;

    bool32 set_draw_mode(render_draw_mode mode) override final;
    bool32 set_highlight_mode(bool32 enabled) override final;
    bool32 disable_depth_test() override final;
    bool32 enable_depth_test() override final;

    void create_texture(struct render_texture_2D* texture, const uint8* data) override final;
    void destroy_texture(struct render_texture_2D* texture) override final;
    void create_mesh(render_geometry* mesh, 
                     uint32 num_verts, const void* vertices,
                     uint32 num_inds, const uint32* indices,
                     const ShaderDataType* attributes) override final;
    void destroy_mesh(render_geometry* mesh) override final;
    bool32 create_shader(shader* shader_prog, const uint8* shader_source, uint64 num_bytes) override final;
    void destroy_shader(shader* shader_prog) override final;
    bool32 create_framebuffer(frame_buffer* fbo, 
                              int num_attachments, const frame_buffer_attachment* attachments) override final;
    void destroy_framebuffer(frame_buffer* fbo) override final;

    void use_shader(shader* shader_prog) override final;
    void use_framebuffer(frame_buffer *fbuffer) override final;

    void draw_geometry(render_geometry* geom) override final;
    void draw_geometry(render_geometry* geom, uint32 start_idx, uint32 num_inds) override final;
    void draw_geometry(render_geometry* geom, render_material* mat) override final;
    void draw_geometry_lines(render_geometry* geom) override final;
    void draw_geometry_points(render_geometry* geom) override final;

    void bind_texture(uint32 tex_handle, uint32 slot) override final;

    void set_viewport(uint32 x, uint32 y, uint32 width, uint32 height) override final;
    void clear_viewport(real32 r, real32 g, real32 b, real32 a) override final;
    void clear_framebuffer_attachment(frame_buffer_attachment* attach, real32 r, real32 b, real32 g, real32 a) override final;

    // uniforms
    //void upload_uniform_float(   shader* shader_prog, const char* uniform_name, float  value) override final;
    //void upload_uniform_float2(  shader* shader_prog, const char* uniform_name, float* values) override final;
    //void upload_uniform_float3(  shader* shader_prog, const char* uniform_name, float* values) override final;
    //void upload_uniform_float4(  shader* shader_prog, const char* uniform_name, float* values) override final;
    //void upload_uniform_float4x4(shader* shader_prog, const char* uniform_name, float* values) override final;
    //void upload_uniform_int(     shader* shader_prog, const char* uniform_name, int  value) override final;
    //void upload_uniform_int2(    shader* shader_prog, const char* uniform_name, int* values) override final;
    //void upload_uniform_int3(    shader* shader_prog, const char* uniform_name, int* values) override final;
    //void upload_uniform_int4(    shader* shader_prog, const char* uniform_name, int* values) override final;

    void upload_uniform_float(   ShaderUniform uniform, float  value) override final;
    void upload_uniform_float2(  ShaderUniform uniform, float* values) override final;
    void upload_uniform_float3(  ShaderUniform uniform, float* values) override final;
    void upload_uniform_float4(  ShaderUniform uniform, float* values) override final;
    void upload_uniform_float4x4(ShaderUniform uniform, float* values) override final;
    void upload_uniform_int(     ShaderUniform uniform, int  value) override final;
    void upload_uniform_int2(    ShaderUniform uniform, int* values) override final;
    void upload_uniform_int3(    ShaderUniform uniform, int* values) override final;
    void upload_uniform_int4(    ShaderUniform uniform, int* values) override final;
};