#include "Renderer.h"

#include "Renderer_API.h"
#include "Engine/Core/Logger.h"
#include "Engine/Memory/Memory_Arena.h"
#include "Engine/Core/Asserts.h"
#include "Engine/Core/Event.h"
#include "Engine/Core/Input.h"

#include "Engine/Resources/Resource_Manager.h"

#include "Engine/Collision/Collision.h"

// include auto-generated shader source headers
#include "Engine/Renderer/ShaderSrc/ShaderSrc.h"

#include <stdarg.h>

#define SIMPLE_RENDER_PASS 1

struct renderer_state {
    uint32 render_width;
    uint32 render_height;

    render_texture_2D white_tex;
    render_texture_2D black_tex;
#if SIMPLE_RENDER_PASS
    // simple render pass
    //shader simple_shader;
    shader_simple simple_shader;
#else
    // deferred pbr render pass
    shader_PrePass pre_pass_shader;
    frame_buffer gbuffer;

    shader_Lighting lighting_shader;
    frame_buffer lbuffer;

    shader_Screen screen_shader;
#endif

    int32 current_shader_out;
    int32 gamma_correct;
    int32 tone_map;

    // debug wireframe
    shader wireframe_shader;
    render_geometry cube_geom;
    render_geometry axis_geom;
    render_geometry screen_quad;
};

global_variable renderer_api* backend;
global_variable renderer_state* render_state;

bool32 renderer_initialize(memory_arena* arena, const char* application_name, platform_state* plat_state) {
    if (!renderer_api_create(arena, RENDERER_API_OPENGL, plat_state, &backend)) {
        return false;
    }

    if (!backend->initialize(application_name, plat_state)) {
        RH_FATAL("Renderer API failed to initialize!");
        return false;
    }

    render_state = PushStruct(arena, renderer_state);
    render_state->render_height = 0;
    render_state->render_width = 0;

    render_state->current_shader_out = 1;
    render_state->gamma_correct = false;
    render_state->tone_map = false;

    return true;
}

bool32 renderer_create_pipeline() {
    Assert(render_state);

    // load default textures
    if (!resource_load_texture_file("Data/Images/white.png", &render_state->white_tex)) {
        RH_FATAL("Could not load default textures");
        return false;
    }
    if (!resource_load_texture_file("Data/Images/black.png", &render_state->black_tex)) {
        RH_FATAL("Could not load default textures");
        return false;
    }

#if SIMPLE_RENDER_PASS
    // setup simple shader
    shader* pSimple = (shader*)(&render_state->simple_shader);
    shader_simple& simple = render_state->simple_shader;
    if (!resource_load_shader_file("Data/Shaders/simple.glsl", pSimple)) {
        RH_FATAL("Could not setup the main shader");
        return false;
    }
    renderer_use_shader(pSimple);

    simple.InitShaderLocs();
    backend->upload_uniform_int(simple.u_texture, simple.u_texture.SamplerID);
#else
    // setup prepass shader
    shader* pPrepass = (shader*)(&render_state->pre_pass_shader);
    shader_PrePass& prepass = render_state->pre_pass_shader;
    if (!resource_load_shader_file("Data/Shaders/PrePass.glsl", pPrepass)) {
        RH_FATAL("Could not setup the pre-pass shader");
        return false;
    }
    renderer_use_shader(pPrepass);
    prepass.InitShaderLocs();
    backend->upload_uniform_int(prepass.u_AlbedoTexture,    prepass.u_AlbedoTexture.SamplerID);
    backend->upload_uniform_int(prepass.u_NormalTexture,    prepass.u_NormalTexture.SamplerID);
    backend->upload_uniform_int(prepass.u_MetalnessTexture, prepass.u_MetalnessTexture.SamplerID);
    backend->upload_uniform_int(prepass.u_RoughnessTexture, prepass.u_RoughnessTexture.SamplerID);
    backend->upload_uniform_int(prepass.u_AmbientTexture,   prepass.u_AmbientTexture.SamplerID);
    backend->upload_uniform_int(prepass.u_EmissiveTexture,  prepass.u_EmissiveTexture.SamplerID);
    // create g-buffer
    laml::Vec4 black;
    laml::Vec4 white(1.0f);
    laml::Vec4 _dist(100.0f, 100.0f, 100.0f, 1.0f);
    {
        frame_buffer_attachment attachments[] = {
            { 0, frame_buffer_texture_format::RGBA8,   black }, // Albedo
            { 0, frame_buffer_texture_format::RGBA16F, black }, // View-space normal
            { 0, frame_buffer_texture_format::RGBA8,   black }, // Ambient/Metallic/Roughness
            { 0, frame_buffer_texture_format::RGBA8,   black }, // Emissive
            { 0, frame_buffer_texture_format::RGBA32F, white }, // Depth-texture
            { 0, frame_buffer_texture_format::Depth,   black }, // Depth-buffer
        };
        render_state->gbuffer.width = render_state->render_width;
        render_state->gbuffer.height = render_state->render_height;
        renderer_create_framebuffer(&render_state->gbuffer, prepass.outputs.num_outputs+1, attachments);
    }

    // Lighting pass
    shader* pLighting = (shader*)(&render_state->lighting_shader);
    shader_Lighting& lighting = render_state->lighting_shader;
    if (!resource_load_shader_file("Data/Shaders/Lighting.glsl", pLighting)) {
        RH_FATAL("Could not setup the PBR lighting pass shader");
        return false;
    }
    renderer_use_shader(pLighting);
    lighting.InitShaderLocs();
    backend->upload_uniform_int(lighting.u_normal, lighting.u_normal.SamplerID);
    backend->upload_uniform_int(lighting.u_depth,  lighting.u_depth.SamplerID);
    backend->upload_uniform_int(lighting.u_amr,    lighting.u_amr.SamplerID);
    // create l-buffer
    {
        frame_buffer_attachment attachments[] = {
            { 0, frame_buffer_texture_format::RGBA8, black }, // Diffuse
            { 0, frame_buffer_texture_format::RGBA8, black }, // Specular
            { 0, frame_buffer_texture_format::Depth, black }, // depth/stencil
        };
        render_state->lbuffer.width = render_state->render_width;
        render_state->lbuffer.height = render_state->render_height;
        renderer_create_framebuffer(&render_state->lbuffer, lighting.outputs.num_outputs+1, attachments);
    }

    // Screen shader
    shader* pScreen = (shader*)(&render_state->screen_shader);
    shader_Screen& screen = render_state->screen_shader;
    if (!resource_load_shader_file("Data/Shaders/Screen.glsl", pScreen)) {
        RH_FATAL("Could not setup the screen shader");
        return false;
    }
    renderer_use_shader(pScreen);
    screen.InitShaderLocs();
    backend->upload_uniform_int(screen.u_albedo,   screen.u_albedo.SamplerID);
    backend->upload_uniform_int(screen.u_normal,   screen.u_normal.SamplerID);
    backend->upload_uniform_int(screen.u_amr,      screen.u_amr.SamplerID);
    backend->upload_uniform_int(screen.u_depth,    screen.u_depth.SamplerID);
    backend->upload_uniform_int(screen.u_diffuse,  screen.u_diffuse.SamplerID);
    backend->upload_uniform_int(screen.u_specular, screen.u_specular.SamplerID);
    backend->upload_uniform_int(screen.u_emissive, screen.u_emissive.SamplerID);
    backend->upload_uniform_int(screen.u_ssao,     screen.u_ssao.SamplerID);

    backend->upload_uniform_int(screen.r_outputSwitch, 0);
    backend->upload_uniform_float(screen.r_toneMap, render_state->tone_map ? 1.0f : 0.0f);
    backend->upload_uniform_float(screen.r_gammaCorrect, render_state->gamma_correct ? 1.0f : 0.0f);
#endif

    // setup wireframe shader
    if (!resource_load_shader_file("Data/Shaders/wireframe.glsl", &render_state->wireframe_shader)) {
        RH_FATAL("Could not setup the wireframe shader");
        return false;
    }
    renderer_use_shader(&render_state->wireframe_shader);

    // create cube mesh for debug purposes
    {
        real32 s = 1.0f;
        typedef laml::Vec3 col_grid_vert;
        col_grid_vert cube_verts[] = {
            { 0, 0, 0 }, // 0
            { s, 0, 0 }, // 1
            { s, s, 0 }, // 2
            { 0, s, 0 }, // 3

            { 0, 0, s }, // 4
            { s, 0, s }, // 5
            { s, s, s }, // 6
            { 0, s, s }, // 7
        };
        uint32 cube_inds[] = {
            // bottom face
            0, 1,
            1, 2,
            2, 3,
            3, 0,

            // top face
            4, 5,
            5, 6,
            6, 7,
            7, 4,

            //verticals
            0, 4,
            1, 5,
            2, 6,
            3, 7
        };
        const ShaderDataType cube_attrs[] = { ShaderDataType::Float3, ShaderDataType::None };
        backend->create_mesh(&render_state->cube_geom, 8, cube_verts, 36, cube_inds, cube_attrs);
    }

    {
        real32 s = 1.0f;
        real32 cube_verts[] = {
             -s, -s, 0, 0, 0, // 0
              s, -s, 0, 1, 0, // 1
              s,  s, 0, 1, 1, // 2
             -s,  s, 0, 0, 1  // 3
        };
        uint32 cube_inds[] = {
            // bottom tri
            0, 1, 2,
            0, 2, 3
        };
        const ShaderDataType quad_attrs[] = { ShaderDataType::Float3, ShaderDataType::Float2, ShaderDataType::None };
        backend->create_mesh(&render_state->screen_quad, 4, cube_verts, 6, cube_inds, quad_attrs);
    }

    resource_load_debug_mesh_into_geometry("Data/Models/debug/gizmo.stl", &render_state->axis_geom);

    return true;
}

void renderer_on_event(uint16 code, void* sender, void* listener, event_context context) {
    switch (code) {
        case EVENT_CODE_KEY_PRESSED:
            if (context.u16[0] == KEY_O) {
                if (render_state->current_shader_out >= 11) {
                    render_state->current_shader_out = 0;
                } else {
                    render_state->current_shader_out++;
                }
                const char* mode_strings[] = {
                    "Full Shading",
                    "Albedo",
                    "Normals",
                    "Ambient",
                    "Metalness",
                    "Roughness",
                    "AMR",
                    "Depth",
                    "Diffuse Lighting",
                    "Specular Highlights",
                    "Emissive",
                    "SSAO"
                };
                RH_INFO("Renderer output mode: %d - %s", render_state->current_shader_out, mode_strings[render_state->current_shader_out]);
            } else if (context.u16[0] == KEY_G) {
                render_state->gamma_correct = !render_state->gamma_correct;
                RH_INFO("Renderer gamma correction: %s", render_state->gamma_correct ? "Enabled" : "Disabled");
            } else if (context.u16[0] == KEY_T) {
                render_state->tone_map = !render_state->tone_map;
                RH_INFO("Renderer tone mapping: %s", render_state->tone_map ? "Enabled" : "Disabled");
            }
    }
}

void renderer_shutdown() {
    renderer_api_destroy(backend);
    backend = 0;
}

bool32 renderer_begin_Frame(real32 delta_time) {
    if (!backend->begin_frame(delta_time)) {
        return false;
    }

    backend->set_viewport(0, 0, render_state->render_width, render_state->render_height);
    backend->clear_viewport(0.8f, 0.1f, 0.8f, 0.1f);

    return true;
}
bool32 renderer_end_Frame(real32 delta_time) {
    bool32 result = backend->end_frame(delta_time);
    backend->frame_number++;
    return result;
}

bool32 renderer_draw_frame(render_packet* packet) {
    if (renderer_begin_Frame(packet->delta_time)) {
        // render all commands in the packet
        laml::Mat4 eye(1.0f);

        laml::Mat4 cam_transform;
        laml::transform::create_transform(cam_transform, packet->camera_orientation, packet->camera_pos);
        laml::transform::create_view_matrix_from_transform(packet->view_matrix, cam_transform);

#if SIMPLE_RENDER_PASS
        shader* pSimple = (shader*)(&render_state->simple_shader);
        renderer_use_shader(pSimple);

        shader_simple& simple = render_state->simple_shader;

        laml::Mat4 proj_view = laml::mul(packet->projection_matrix, packet->view_matrix);
        backend->upload_uniform_float4x4(simple.r_VP, proj_view._data);
        laml::Vec3 color(1.0f, 1.0f, 1.0f);
        backend->upload_uniform_float3(simple.u_color, color._data);

        // draw world axis
        //renderer_upload_uniform_float4x4(&render_state->simple_shader, "r_Transform", 
        //                                 eye._data);
        //renderer_draw_geometry(&render_state->axis_geom);

        for (uint32 cmd_index = 0; cmd_index < packet->num_commands; cmd_index++) {
            render_command& cmd = packet->commands[cmd_index];
            render_material& mat = cmd.material;

            backend->upload_uniform_float4x4(simple.r_Transform, cmd.model_matrix._data);

            backend->upload_uniform_float3(simple.u_color, mat.DiffuseFactor._data);
            if (mat.flag & 0x02) {
                backend->bind_texture(mat.DiffuseTexture.handle, 0);
            } else {
                backend->bind_texture(render_state->white_tex.handle, 0);
            }

            renderer_draw_geometry(&cmd.geom);
        }
#else
        // PREPASS STAGE
        shader* pPrepass = (shader*)(&render_state->pre_pass_shader);
        shader_PrePass& prepass = render_state->pre_pass_shader;
        renderer_use_shader(pPrepass);

        backend->upload_uniform_float4x4(prepass.r_View, packet->view_matrix._data);
        backend->upload_uniform_float4x4(prepass.r_Projection, packet->projection_matrix._data);
        laml::Vec3 color(1.0f, 0.0f, 1.0f);
        backend->upload_uniform_float(prepass.r_gammaCorrect, 1.0f);

        backend->use_framebuffer(&render_state->gbuffer);
        backend->clear_viewport(0, 0, 0, 0);
        backend->clear_framebuffer_attachment(&render_state->gbuffer.attachments[prepass.outputs.out_Depth], 1, 1, 1, 1);

        for (uint32 cmd_index = 0; cmd_index < packet->num_commands; cmd_index++) {
            render_material* mat = &packet->commands[cmd_index].material;

            backend->upload_uniform_float3(prepass.u_AlbedoColor, mat->DiffuseFactor._data);
            backend->upload_uniform_float(prepass.u_Metalness, mat->MetallicFactor);
            backend->upload_uniform_float(prepass.u_Roughness, mat->RoughnessFactor);
            backend->upload_uniform_float(prepass.u_TextureScale, 1.0f);

            backend->upload_uniform_float(prepass.r_AlbedoTexToggle,    (mat->flag & 0x02) ? 1.0f : 0.0f);
            backend->upload_uniform_float(prepass.r_NormalTexToggle,    (mat->flag & 0x04) ? 1.0f : 0.0f);
            backend->upload_uniform_float(prepass.r_MetalnessTexToggle, (mat->flag & 0x08) ? 1.0f : 0.0f);
            backend->upload_uniform_float(prepass.r_RoughnessTexToggle, (mat->flag & 0x08) ? 1.0f : 0.0f);
            backend->upload_uniform_float(prepass.r_AmbientTexToggle,   (mat->flag & 0x08) ? 1.0f : 0.0f);
            backend->upload_uniform_float(prepass.r_EmissiveTexToggle,  (mat->flag & 0x10) ? 1.0f : 0.0f);

            backend->bind_texture(mat->DiffuseTexture.handle,  prepass.u_AlbedoTexture.SamplerID);
            backend->bind_texture(mat->NormalTexture.handle,   prepass.u_NormalTexture.SamplerID);
            backend->bind_texture(mat->AMRTexture.handle,      prepass.u_AmbientTexture.SamplerID);
            backend->bind_texture(mat->AMRTexture.handle,      prepass.u_MetalnessTexture.SamplerID);
            backend->bind_texture(mat->AMRTexture.handle,      prepass.u_RoughnessTexture.SamplerID);
            backend->bind_texture(mat->EmissiveTexture.handle, prepass.u_EmissiveTexture.SamplerID);

            backend->upload_uniform_float4x4(prepass.r_Transform, 
                                             packet->commands[cmd_index].model_matrix._data);
            renderer_draw_geometry(&packet->commands[cmd_index].geom);
        }

        // LIGHTING STAGE
        shader* pLighting = (shader*)(&render_state->lighting_shader);
        shader_Lighting& lighting = render_state->lighting_shader;
        backend->use_framebuffer(&render_state->lbuffer);
        backend->use_shader(pLighting);

        // TODO: let the scene define these (and more lights)
        float sun_dir[] = {0.0f, -0.7071f, 0.7071f};
        float sun_color[] = {1.0f, 0.0f, 1.0f};
        float sun_strength = 1.0f;

        backend->upload_uniform_float4x4(lighting.r_Projection, packet->projection_matrix._data);
        backend->upload_uniform_float3(lighting.r_sun.Direction, sun_dir);
        backend->upload_uniform_float3(lighting.r_sun.Color, sun_color);
        backend->upload_uniform_float(lighting.r_sun.Strength, sun_strength);

        backend->bind_texture(render_state->gbuffer.attachments[prepass.outputs.out_Normal].handle, lighting.u_normal.SamplerID);
        backend->bind_texture(render_state->gbuffer.attachments[prepass.outputs.out_Depth].handle,  lighting.u_depth.SamplerID);
        backend->bind_texture(render_state->gbuffer.attachments[prepass.outputs.out_AMR].handle,    lighting.u_amr.SamplerID);

        backend->disable_depth_test();
        backend->clear_viewport(0.0f, 0.0f, 0.0f, 0.0f);
        backend->draw_geometry(&render_state->screen_quad);

        // SCREEN STAGE
        shader* pScreen = (shader*)(&render_state->screen_shader);
        shader_Screen& screen = render_state->screen_shader;
        backend->use_framebuffer(0);
        backend->use_shader(pScreen);

        backend->upload_uniform_int(screen.r_outputSwitch, render_state->current_shader_out);
        backend->upload_uniform_float(screen.r_toneMap, render_state->tone_map ? 1.0f : 0.0f);
        backend->upload_uniform_float(screen.r_gammaCorrect, render_state->gamma_correct ? 1.0f : 0.0f);

        backend->bind_texture(render_state->gbuffer.attachments[prepass.outputs.out_Albedo].handle,    screen.u_albedo.SamplerID);
        backend->bind_texture(render_state->gbuffer.attachments[prepass.outputs.out_Normal].handle,    screen.u_normal.SamplerID);
        backend->bind_texture(render_state->gbuffer.attachments[prepass.outputs.out_AMR].handle,       screen.u_amr.SamplerID);
        backend->bind_texture(render_state->gbuffer.attachments[prepass.outputs.out_Depth].handle,     screen.u_depth.SamplerID);
        backend->bind_texture(render_state->gbuffer.attachments[prepass.outputs.out_Emissive].handle,  screen.u_emissive.SamplerID);
        backend->bind_texture(render_state->lbuffer.attachments[lighting.outputs.out_Diffuse].handle,  screen.u_diffuse.SamplerID);
        backend->bind_texture(render_state->lbuffer.attachments[lighting.outputs.out_Specular].handle, screen.u_specular.SamplerID);
        backend->bind_texture(render_state->black_tex.handle,                                          screen.u_ssao.SamplerID);

        backend->disable_depth_test();
        backend->clear_viewport(0.0f, 0.0f, 0.0f, 0.0f);
        backend->draw_geometry(&render_state->screen_quad);
        backend->enable_depth_test();
        
#endif

#if 0
        if (packet->draw_colliders) {
            // Render debug wireframes
            renderer_begin_wireframe();
            renderer_use_shader(&render_state->wireframe_shader);

            //laml::Mat4 proj_view = laml::mul(packet->projection_matrix, packet->view_matrix);
            renderer_upload_uniform_float4x4(&render_state->wireframe_shader, "r_VP", proj_view._data);
            laml::Vec4 wire_color(.6f, 0.5f, 0.65f, 1.0f);
            renderer_upload_uniform_float4(&render_state->wireframe_shader, "u_color", wire_color._data);

            renderer_upload_uniform_float4x4(&render_state->wireframe_shader, "r_Transform",
                                             packet->collider_geom.model_matrix._data);
            renderer_draw_geometry(&packet->collider_geom.geom);
            //renderer_draw_geometry(&render_state->axis_geom);

            for (uint32 cmd_index = 0; cmd_index < packet->num_commands; cmd_index++) {
                renderer_upload_uniform_float4x4(&render_state->wireframe_shader, "r_Transform",
                                                 packet->commands[cmd_index].model_matrix._data);
                renderer_draw_geometry(&packet->commands[cmd_index].geom);
            
                laml::Vec4 point_color = { .8f, 0.4f, 0.25f, 1.0f };
                renderer_upload_uniform_float4(&render_state->wireframe_shader, "u_color", point_color._data);
                renderer_draw_geometry_points(&packet->commands[cmd_index].geom);
            }

            // draw sphere at contact point
            laml::Mat4 transform;
            laml::Quat rot(0.0f, 0.0f, 0.0f, 1.0f);
            laml::Vec3 scale(0.2f);
            laml::transform::create_transform(transform, rot, (packet->contact_point + laml::Vec3(-0.4f)), scale);
            renderer_upload_uniform_float4x4(&render_state->wireframe_shader, "r_Transform",
                                             transform._data);
            renderer_draw_geometry(&render_state->cube_geom);

            renderer_end_wireframe();

            #define DRAW_COLLISION_GRID 0
            #if DRAW_COLLISION_GRID
            if (packet->col_grid && packet->col_grid->num_filled_cells > 0) {
                laml::Mat4 cell_transform(1.0f);
                renderer_upload_uniform_float4x4(&render_state->wireframe_shader, "r_Transform",
                                                 cell_transform._data);

                wire_color = { .7f, 0.6f, 0.35f, 1.0f };
                renderer_upload_uniform_float4(&render_state->wireframe_shader, "u_color", wire_color._data);
                renderer_draw_geometry_lines(packet->col_grid->geom);
                //wire_color = { .8f, 0.4f, 0.25f, 1.0f };
                //renderer_upload_uniform_float4(&render_state->wireframe_shader, "u_color", wire_color._data);
                //renderer_draw_geometry_points(&packet->col_grid->geom);
            }
            #endif
            // Draw the highlighted cube/faces
            if (packet->col_grid && packet->sector.inside) {
                backend->set_highlight_mode(true);

                collision_grid * grid = packet->col_grid;

                if (packet->num_tris > 0) {
                    renderer_use_shader(&render_state->simple_shader);

                    color = laml::Vec4(0.5f, 0.6f, 0.4f, 1.0f);
                    renderer_upload_uniform_float4(&render_state->simple_shader, "u_color", color._data);

                    renderer_upload_uniform_float4x4(&render_state->simple_shader, "r_Transform",
                                                     packet->commands[0].model_matrix._data);

                    // draw possible triangles
                    for (uint32 n = 0; n < packet->num_tris; n++) {
                        uint32 tri_idx = packet->triangle_indices[n];

                        // TODO: dumb
                        // check that it isn't an intersecting oene
                        bool32 draw = true;
                        for (uint32 nn = 0; nn < packet->num_intersecting_tris; nn++) {
                            if (tri_idx == packet->intersecting_triangle_indices[nn]) {
                                draw = false;
                                break;
                            }
                        }

                        if (draw) {
                            uint32 start_idx = tri_idx * 3;
                            renderer_draw_geometry(&packet->commands[0].geom, start_idx, 3);
                        }
                    }

                    // draw intersecting triangles
                    color = laml::Vec4(0.7f, 0.2f, 0.5f, 1.0f);
                    renderer_upload_uniform_float4(&render_state->simple_shader, "u_color", color._data);
                    for (uint32 n = 0; n < packet->num_intersecting_tris; n++) {
                        uint32 tri_idx = packet->intersecting_triangle_indices[n];

                        uint32 start_idx = tri_idx * 3;
                        renderer_draw_geometry(&packet->commands[0].geom, start_idx, 3);
                    }
                }

                #define DRAW_SECTOR 1
                #if DRAW_SECTOR
                renderer_use_shader(&render_state->wireframe_shader);
                // render sector of cells
                for (int32 x = packet->sector.x_min; x <= packet->sector.x_max; x++) {
                    for (int32 y = packet->sector.y_min; y <= packet->sector.y_max; y++) {
                        for (int32 z = packet->sector.z_min; z <= packet->sector.z_max; z++) {
                            collision_grid_cell * cell = &packet->col_grid->cells[x][y][z];

                            laml::Vec3 cell_pos = collision_cell_to_world(packet->col_grid, x, y, z);
                            laml::Mat4 cell_transform;
                            laml::transform::create_transform_translate(cell_transform, cell_pos);
                            laml::Mat4 scale_m;
                            laml::transform::create_transform_scale(scale_m, laml::Vec3(grid->cell_size));
                            cell_transform = laml::mul(cell_transform, scale_m);
                            renderer_upload_uniform_float4x4(&render_state->wireframe_shader, "r_Transform",
                                                             cell_transform._data);
                            wire_color = { .35f, 0.6f, 0.7f, 0.5f };
                            renderer_upload_uniform_float4(&render_state->wireframe_shader, "u_color", wire_color._data);

                            backend->disable_depth_test();
                            renderer_draw_geometry_lines(&render_state->cube_geom);
                            backend->enable_depth_test();
                        }
                    }
                }
                #endif

                backend->set_highlight_mode(false);
            }
        }
        #endif

        bool32 result = renderer_end_Frame(packet->delta_time);

        if (!result) {
            RH_ERROR("end_frame() failed for some reason!");
            return false;
        }
    }

    return true;
}

void renderer_resized(uint32 width, uint32 height) {
    if (width != render_state->render_width || height != render_state->render_height) {
        // resize framebuffers
        render_state->render_width = width;
        render_state->render_height = height;
    }
}

bool32 renderer_begin_wireframe() {
    backend->set_draw_mode(render_draw_mode::Wireframe);
    return true;
}
bool32 renderer_end_wireframe() {
    backend->set_draw_mode(render_draw_mode::Normal);
    return true;
}





void renderer_create_texture(struct render_texture_2D* texture, const uint8* data) {
    backend->create_texture(texture, data);
}
void renderer_destroy_texture(struct render_texture_2D* texture) {
    backend->destroy_texture(texture);
}

void renderer_create_mesh(render_geometry* mesh, 
                          uint32 num_verts, const void* vertices,
                          uint32 num_inds, const uint32* indices,
                          const ShaderDataType* attributes) {
    backend->create_mesh(mesh, 
                         num_verts, vertices, 
                         num_inds, indices, 
                         attributes);
}
void renderer_destroy_mesh(render_geometry* mesh) {
    backend->destroy_mesh(mesh);
}

bool32 renderer_create_shader(shader* shader_prog, const uint8* shader_source, uint64 num_bytes) {
    return backend->create_shader(shader_prog, shader_source, num_bytes);
}
void renderer_destroy_shader(shader* shader_prog) {
    backend->destroy_shader(shader_prog);
}

bool32 renderer_create_framebuffer(frame_buffer* fbo, 
                                   int num_attachments, const frame_buffer_attachment* attachments) {
    return backend->create_framebuffer(fbo, num_attachments, attachments);
}
void renderer_destroy_framebuffer(frame_buffer* fbo) {
    backend->destroy_framebuffer(fbo);
}


void renderer_use_shader(shader* shader_prog) {
    backend->use_shader(shader_prog);
}
void renderer_draw_geometry(render_geometry* geom) {
    backend->draw_geometry(geom);
}
void renderer_draw_geometry(render_geometry* geom, uint32 start_idx, uint32 num_inds) {
    backend->draw_geometry(geom, start_idx, num_inds);
}
void renderer_draw_geometry(render_geometry * geom, render_material * mat) {
    backend->draw_geometry(geom, mat);
}
void renderer_draw_geometry_lines(render_geometry* geom) {
    backend->draw_geometry_lines(geom);
}
void renderer_draw_geometry_points(render_geometry* geom) {
    backend->draw_geometry_points(geom);
}