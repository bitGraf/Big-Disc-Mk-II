#pragma once

#include "Engine/Defines.h"
#include <laml/laml.hpp>

#include "Engine/Collision/Collision_Types.h"

struct memory_arena;
struct render_geometry;
struct render_material;
struct shader;
struct ShaderUniform;
struct frame_buffer;
struct frame_buffer_attachment;
struct collision_grid;

enum renderer_api_type {
    RENDERER_API_OPENGL,
    RENDERER_API_DIRECTX_12,
};

enum class ShaderDataType : uint8 {
    None = 0, 
    Float, Float2, Float3, Float4,
    Mat3, Mat4, 
    Int, Int2, Int3, Int4,
    Bool
};

enum class render_draw_mode : uint8 {
    Normal = 0,
    Wireframe,
    Points
};

enum class render_stencil_func : uint8 {
    Never = 0,
    Less,
    LessOrEqual,
    Greater,
    GreaterOrEqual,
    Equal,
    NotEqual,
    Always
};

enum class render_stencil_op : uint8 {
    Keep = 0,
    Zero,
    Replace,
    Increment,
    Increment_wrap,
    Decrement,
    Decrement_wrap,
    Invert
};

enum class Index_Buffer_Type : uint32 {
    UInt16 = 0,
    UInt32
};

// the actual geometry that the gpu holds onto
// separately, a 'resource' will exist to 
// represent an actual mesh.
struct render_geometry {
    // DirectX12 binds vertex buffers and index buffers directly no problem
    // OpenGL needs a Vertex Array Object (VAO). We simply store the VAO handle
    // In the Vertex Buffer handle, and let it handle that
    struct _Vertex_Buffer {
        uint64 handle;          // handle to the gpu version of this data
        uint32 buffer_size;     // total buffer size, in bytes
        uint32 buffer_stride;   // stide of buffer element, in bytes
    } vertex_buffer;

    struct _Index_Buffer {
        uint64 handle;                // handle to the gpu version of this data
        uint32 buffer_size;           // total buffer size, in bytes
        Index_Buffer_Type index_type;   // index buffer type (16/32 uint)
    } index_buffer;

    uint32 num_verts;
    uint32 num_inds;
};

struct render_texture_2D {
    uint32 handle; // handle to the gpu version of this data
};
struct texture_creation_info_2D {
    uint16 width;
    uint16 height;
    uint16 num_channels;

    // filtering flags
};

struct render_texture_3D {
    uint32 handle; // handle to the gpu version of this data
};
struct texture_creation_info_3D {
    uint16 width;
    uint16 height;
    uint16 depth;
    uint16 num_channels;

    // filtering flags
};

struct render_texture_cube {
    uint32 handle; // handle to the gpu version of this data
};
struct texture_creation_info_cube {
    uint16 width;
    uint16 height;
    uint16 num_channels;

    // filtering flags
};

struct render_material {
    laml::Vec3 DiffuseFactor;
    real32 NormalScale;
    real32 AmbientStrength;
    real32 MetallicFactor;
    real32 RoughnessFactor;
    laml::Vec3 EmissiveFactor;
    uint32 flag;

    render_texture_2D DiffuseTexture;
    render_texture_2D NormalTexture;
    render_texture_2D AMRTexture;
    render_texture_2D EmissiveTexture;
};

struct shader {
    uint32 handle; // handle to the gpu version of this data
};

struct ShaderUniform {
    uint32 Location;
    uint32 SamplerID; // only for samplers
};

typedef ShaderUniform ShaderUniform_int;
typedef ShaderUniform ShaderUniform_ivec2;
typedef ShaderUniform ShaderUniform_ivec3;
typedef ShaderUniform ShaderUniform_ivec4;

typedef ShaderUniform ShaderUniform_float;
typedef ShaderUniform ShaderUniform_vec2;
typedef ShaderUniform ShaderUniform_vec3;
typedef ShaderUniform ShaderUniform_vec4;

typedef ShaderUniform ShaderUniform_mat3;
typedef ShaderUniform ShaderUniform_mat4;
typedef ShaderUniform ShaderUniform_sampler2D;
typedef ShaderUniform ShaderUniform_samplerCube;

struct ShaderUniform_Light {
    uint32 Location;

    ShaderUniform_vec3 Position;
    ShaderUniform_vec3 Direction;
    ShaderUniform_vec3 Color;
    ShaderUniform_float Strength;
    ShaderUniform_float Inner;
    ShaderUniform_float Outer;
};

enum class frame_buffer_texture_format {
    None = 0,

    // Color
    RED8,
    //RGB8,
    RGBA8,
    RGBA16F,
    RGB16F,
    RG16F,
    RGBA32F,
    R32F,
    //RGB32F,
    //RG32F,

    // Depth/stencil
    //DEPTH32F,
    DEPTH24STENCIL8,
    //DepthStencil=DEPTH24STENCIL8,

    // Defaults
    Depth = DEPTH24STENCIL8
};
/*
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
*/
enum class texture_wrap_type {
    clamp_to_edge,
    clamp_to_border,
    mirrored_repeat,
    repeat,
};
enum class texture_filter_type {
    // only options for mag_filter
    linear,
    nearest,

    // works for both min and mag
    nearest_mipmap_nearest,
    linear_mipmap_nearest,
    nearest_mipmap_linear,
    linear_mipmap_linear
};
struct frame_buffer_create_info {
    texture_filter_type min_filter;
    texture_filter_type mag_filter;

    texture_wrap_type wrap_s;
    texture_wrap_type wrap_t;

    laml::Vec4 border;
};

struct frame_buffer_attachment {
    uint32 handle;

    frame_buffer_texture_format texture_format;
    laml::Vec4 clear_color;
};

struct frame_buffer {
    uint32 handle;

    uint32 width;
    uint32 height;

    uint32 num_attachments;
    frame_buffer_attachment attachments[6];
};

struct render_pass {
    uint16 handle;

    uint8* per_frame;
    uint8* per_object;
};

struct render_command {
    laml::Mat4 model_matrix;
    render_geometry geom;
    render_material material;
    uint32 skeleton_idx; // idx of 0 is NO skeleton! 1+ are valid skeletons
};

struct render_skeleton {
    uint32 num_bones;
    laml::Mat4* bones;
};

struct render_light {
    laml::Vec3 position;
    laml::Vec3 direction;
    laml::Vec3 color;
    real32 strength;
    real32 inner, outer; // cos(cone_angle)! must be precalculated

    bool32 cast_shadow;
    laml::Mat4 light_space;
};

struct render_env_map {
    render_texture_cube skybox;
    render_texture_cube prefilter;
    render_texture_cube irradiance;
    real32 strength; // contribution to diffuse and specular lighting
};

struct render_packet {
    memory_arena* arena;

    real32 delta_time;

    laml::Mat4 projection_matrix;
    laml::Mat4 view_matrix;

    laml::Vec3 camera_pos;
    laml::Quat camera_orientation;

    uint32 num_commands;
    render_command* commands;
    uint32 num_skeletons;
    render_skeleton* skeletons;

    // lighting
    render_light sun;
    uint32 num_point_lights;
    render_light* point_lights;
    uint32 num_spot_lights;
    render_light* spot_lights;
    render_env_map env_map;

    bool32 draw_skybox;

    // internal
    uint32 _num_static, _num_skinned;
    render_command* _static_cmds;
    render_command* _skinned_cmds;
};




struct renderer_api {
    //struct platform_state* plat_state; // platform-specific state
    uint64 frame_number;

    virtual bool32 initialize(const char* application_name, 
                              struct platform_state* plat_state,
                              memory_arena* backend_storage) = 0;
    virtual void shutdown() = 0;
    virtual void resized(uint16 width, uint16 height) = 0;

    virtual bool32 begin_frame(real32 delta_time) = 0;
    virtual bool32 end_frame(real32 delta_time) = 0;
    virtual bool32 present(uint32 sync_interval) = 0;

    virtual uint32 get_batch_size() = 0;
    virtual void set_batch_index(render_pass* pass, uint32 index) = 0;

    virtual bool32 ImGui_Init() = 0;
    virtual bool32 ImGui_begin_frame() = 0;
    virtual bool32 ImGui_end_frame() = 0;
    virtual bool32 ImGui_Shutdown() = 0;

    virtual void set_draw_mode(render_draw_mode mode) = 0;
    virtual void set_highlight_mode(bool32 enabled) = 0;
    virtual void enable_depth_test() = 0;
    virtual void disable_depth_test() = 0;
    virtual void enable_depth_mask() = 0;
    virtual void disable_depth_mask() = 0;

    virtual void enable_stencil_test() = 0;
    virtual void disable_stencil_test() = 0;
    virtual void set_stencil_mask(uint32 mask) = 0;
    virtual void set_stencil_func(render_stencil_func func, uint32 ref, uint32 mask) = 0;
    virtual void set_stencil_op(render_stencil_op sfail, render_stencil_op dpfail, render_stencil_op dppass) = 0;

    virtual void push_debug_group(const char* label) = 0;
    virtual void pop_debug_group() = 0;

    virtual void create_texture_2D(struct   render_texture_2D*   texture, texture_creation_info_2D   create_info, const void*  data, bool32 is_hdr) = 0;
    virtual void create_texture_3D(struct   render_texture_3D*   texture, texture_creation_info_3D   create_info, const void*  data, bool32 is_hdr) = 0;
    virtual void create_texture_cube(struct render_texture_cube* texture, texture_creation_info_cube create_info, 
                                     const void*** data, bool32 is_hdr, uint32 mip_levels) = 0;

    virtual void destroy_texture_2D(struct   render_texture_2D*   texture) = 0;
    virtual void destroy_texture_3D(struct   render_texture_3D*   texture) = 0;
    virtual void destroy_texture_cube(struct render_texture_cube* texture) = 0;

    virtual void create_mesh(render_geometry* mesh, 
                             uint32 num_verts, const void* vertices,
                             uint32 num_inds, const uint32* indices,
                             const ShaderDataType* attributes) = 0;
    virtual void destroy_mesh(render_geometry* mesh) = 0;

    virtual void create_render_pass(render_pass* pass, shader* pass_shader,
                                    uint64 sz_per_pass, uint64 sz_per_obj) = 0;
    virtual void begin_render_pass(render_pass* pass) = 0;
    virtual void end_render_pass(render_pass* pass) = 0;

    virtual bool32 create_shader(shader* shader_prog, const uint8* shader_source, uint64 num_bytes) = 0;
    virtual void destroy_shader(shader* shader_prog) = 0;

    virtual bool32 create_framebuffer(frame_buffer* fbo, 
                                      int num_attachments, 
                                      const frame_buffer_attachment* attachments,
                                      frame_buffer_create_info info) = 0;
    virtual bool32 recreate_framebuffer(frame_buffer* fbo) = 0;
    virtual bool32 create_framebuffer_cube(frame_buffer* fbo, 
                                           int num_attachments, 
                                           const frame_buffer_attachment* attachments,
                                           bool32 generate_mipmaps) = 0;
    virtual void copy_framebuffer_depthbuffer(frame_buffer * src, frame_buffer * dst) = 0;
    virtual void copy_framebuffer_stencilbuffer(frame_buffer * src, frame_buffer * dst) = 0;
    virtual void destroy_framebuffer(frame_buffer* fbo) = 0;

    virtual void use_shader(shader* shader_prog) = 0;
    virtual void use_framebuffer(frame_buffer *fbuffer) = 0;
    virtual void set_framebuffer_cube_face(frame_buffer* fbuffer, uint32 attach_idx, uint32 slot, uint32 mip_level) = 0;
    virtual void resize_framebuffer_renderbuffer(frame_buffer* fbuffer, uint32 new_width, uint32 new_height) = 0;

    virtual void bind_geometry(render_geometry* geom) = 0;

    virtual void draw(uint32 verts_per_instance, uint32 first_vertex) = 0;
    virtual void draw_instanced(uint32 verts_per_instance, uint32 instance_count, uint32 first_vertex, uint32 first_instance) = 0;

    virtual void draw_indexed(uint32 inds_per_instance, uint32 first_index, uint32 first_vertex) = 0;
    virtual void draw_indexed_instanced(uint32 inds_per_instance, uint32 instance_count, uint32 first_index, uint32 first_vertex, uint32 first_instance) = 0;

    virtual void bind_texture_2D(  render_texture_2D texture, uint32 slot) = 0;
    virtual void bind_texture_3D(  render_texture_3D texture, uint32 slot) = 0;
    virtual void bind_texture_cube(render_texture_cube texture, uint32 slot) = 0;

    virtual void bind_texture_2D(frame_buffer_attachment attachment, uint32 slot) = 0;
    virtual void bind_texture_3D(frame_buffer_attachment attachment, uint32 slot) = 0;
    virtual void bind_texture_cube(frame_buffer_attachment attachment, uint32 slot) = 0;

    virtual void set_viewport(uint32 x, uint32 y, uint32 width, uint32 height) = 0;
    virtual void clear_viewport(real32 r, real32 g, real32 b, real32 a) = 0;
    virtual void clear_viewport_only_color(real32 r, real32 g, real32 b, real32 a) = 0;
    virtual void clear_framebuffer_attachment(frame_buffer_attachment* attach, real32 r, real32 g, real32 b, real32 a) = 0;

    // read data from gpu to cpu
    virtual void get_texture_data(render_texture_2D   texture, void* data, int num_channels, bool is_hdr, uint32 mip) = 0;
    virtual void get_cubemap_data(render_texture_cube texture, void* data, int num_channels, bool is_hdr, uint32 face, uint32 mip) = 0;

    // uniforms
    virtual void upload_uniform_float(   ShaderUniform_float uniform, real32  value) = 0;
    virtual void upload_uniform_float2(  ShaderUniform_vec2  uniform, const laml::Vec2& values) = 0;
    virtual void upload_uniform_float3(  ShaderUniform_vec3  uniform, const laml::Vec3& values) = 0;
    virtual void upload_uniform_float4(  ShaderUniform_vec4  uniform, const laml::Vec4& values) = 0;
    virtual void upload_uniform_float4x4(ShaderUniform_mat4  uniform, const laml::Mat4& values) = 0;
    virtual void upload_uniform_int(     ShaderUniform_int   uniform, int32  value) = 0;
    virtual void upload_uniform_int2(    ShaderUniform_ivec2 uniform, const laml::Vector<int32,2>& values) = 0;
    virtual void upload_uniform_int3(    ShaderUniform_ivec3 uniform, const laml::Vector<int32,3>& values) = 0;
    virtual void upload_uniform_int4(    ShaderUniform_ivec4 uniform, const laml::Vector<int32, 4>& values) = 0;
};