#pragma once

#include "../Renderer_API.h"

struct DirectX12_api final : public renderer_api {
    bool32 initialize(const char* application_name, 
                      struct platform_state* plat_state,
                      memory_arena* backend_storage) override final;
    void shutdown() override final;

    void resized(uint16 width, uint16 height) override final;

    bool32 begin_frame(real32 delta_time) override final;
    bool32 end_frame(real32 delta_time) override final;
    bool32 present(uint32 sync_interval) override final;

    uint32 get_batch_size() override final;
    void set_batch_index(uint32 index) override final;

    void push_debug_group(const char* label) override final;
    void pop_debug_group() override final;
    
    bool32 ImGui_Init() override final;
    bool32 ImGui_begin_frame() override final;
    bool32 ImGui_end_frame() override final;
    bool32 ImGui_Shutdown() override final;

    void set_draw_mode(render_draw_mode mode) override final;
    void set_highlight_mode(bool32 enabled) override final;
    void disable_depth_test() override final;
    void enable_depth_test() override final;
    void disable_depth_mask() override final;
    void enable_depth_mask() override final;

    void disable_stencil_test() override final;
    void enable_stencil_test() override final;
    void set_stencil_mask(uint32 mask) override final;
    void set_stencil_func(render_stencil_func func, uint32 ref, uint32 mask) override final;
    void set_stencil_op(render_stencil_op sfail, render_stencil_op dpfail, render_stencil_op dppass) override final;

    void create_texture_2D(struct render_texture_2D*     texture, texture_creation_info_2D   create_info, const void* data, bool32 is_hdr) override final;
    void create_texture_3D(struct render_texture_3D*     texture, texture_creation_info_3D   create_info, const void* data, bool32 is_hdr) override final;
    void create_texture_cube(struct render_texture_cube* texture, texture_creation_info_cube create_info, 
                             const void*** data, bool32 is_hdr, uint32 mip_levels) override final;

    void destroy_texture_2D(struct render_texture_2D* texture) override final;
    void destroy_texture_3D(struct render_texture_3D* texture) override final;
    void destroy_texture_cube(struct render_texture_cube* texture) override final;

    void create_mesh(render_geometry* mesh, 
                     uint32 num_verts, const void* vertices,
                     uint32 num_inds, const uint32* indices,
                     const ShaderDataType* attributes) override final;
    void destroy_mesh(render_geometry* mesh) override final;
    bool32 create_shader(shader* shader_prog, const uint8* shader_source, uint64 num_bytes) override final;
    void destroy_shader(shader* shader_prog) override final;
    bool32 create_framebuffer(frame_buffer* fbo, 
                              int num_attachments, 
                              const frame_buffer_attachment* attachments,
                              frame_buffer_create_info info) override final;
    bool32 recreate_framebuffer(frame_buffer* fbo) override final;
    bool32 create_framebuffer_cube(frame_buffer* fbo, 
                                   int num_attachments, 
                                   const frame_buffer_attachment* attachments,
                                   bool32 generate_mipmaps) override final;
    void destroy_framebuffer(frame_buffer* fbo) override final;
    void set_framebuffer_cube_face(frame_buffer* fbuffer, uint32 attach_idx, uint32 slot, uint32 mip_level) override final;
    void resize_framebuffer_renderbuffer(frame_buffer* fbuffer, uint32 new_width, uint32 new_height) override final;
    void copy_framebuffer_depthbuffer(frame_buffer * src, frame_buffer * dst) override final;
    void copy_framebuffer_stencilbuffer(frame_buffer * src, frame_buffer * dst) override final;

    void create_render_pass(render_pass* pass, shader* pass_shader,
                            uint64 sz_per_pass, uint64 sz_per_obj) override final;
    void begin_render_pass(render_pass* pass) override final;
    void end_render_pass(render_pass* pass) override final;

    void use_shader(shader* shader_prog) override final;
    void use_framebuffer(frame_buffer *fbuffer) override final;

    void bind_geometry(render_geometry* geom) override final;

    void draw(uint32 verts_per_instance, uint32 first_vertex) override final;
    void draw_instanced(uint32 verts_per_instance, uint32 instance_count, uint32 first_vertex, uint32 first_instance) override final;

    void draw_indexed(uint32 inds_per_instance, uint32 first_index, uint32 first_vertex) override final;
    void draw_indexed_instanced(uint32 inds_per_instance, uint32 instance_count, uint32 first_index, uint32 first_vertex, uint32 first_instance) override final;

    void bind_texture_2D(render_texture_2D texture, uint32 slot) override final;
    void bind_texture_3D(render_texture_3D texture, uint32 slot) override final;
    void bind_texture_cube(render_texture_cube texture, uint32 slot) override final;

    void bind_texture_2D(frame_buffer_attachment   attachment, uint32 slot) override final;
    void bind_texture_3D(frame_buffer_attachment   attachment, uint32 slot) override final;
    void bind_texture_cube(frame_buffer_attachment attachment, uint32 slot) override final;

    void set_viewport(uint32 x, uint32 y, uint32 width, uint32 height) override final;
    void clear_viewport(real32 r, real32 g, real32 b, real32 a) override final;
    void clear_viewport_only_color(real32 r, real32 g, real32 b, real32 a) override final;
    void clear_framebuffer_attachment(frame_buffer_attachment* attach, real32 r, real32 b, real32 g, real32 a) override final;

    void get_texture_data(render_texture_2D texture, void* data, int num_channels, bool is_hdr, uint32 mip) override final;
    void get_cubemap_data(render_texture_cube texture, void* data, int num_channels, bool is_hdr, uint32 face, uint32 mip) override final;

    void upload_uniform_float(   ShaderUniform_float uniform, real32  value) override final;
    void upload_uniform_float2(  ShaderUniform_vec2  uniform, const laml::Vec2& values) override final;
    void upload_uniform_float3(  ShaderUniform_vec3  uniform, const laml::Vec3& values) override final;
    void upload_uniform_float4(  ShaderUniform_vec4  uniform, const laml::Vec4& values) override final;
    void upload_uniform_float4x4(ShaderUniform_mat4  uniform, const laml::Mat4& values) override final;
    void upload_uniform_int(     ShaderUniform_int   uniform, int32  value) override final;
    void upload_uniform_int2(    ShaderUniform_ivec2 uniform, const laml::Vector<int32,2>& values) override final;
    void upload_uniform_int3(    ShaderUniform_ivec3 uniform, const laml::Vector<int32,3>& values) override final;
    void upload_uniform_int4(    ShaderUniform_ivec4 uniform, const laml::Vector<int32,4>& values) override final;
};