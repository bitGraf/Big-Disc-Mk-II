// Code generated by W:\rohin\Tools\shader_tool\bin\shader_tool.exe
#ifndef __SHADERS_GENERATED_H__
#define __SHADERS_GENERATED_H__

#include "Engine/Defines.h"

#include "Engine/Renderer/ShaderSrc/ShaderSrc.h"

struct shader_BRDF_Integrate {
    uint32 Handle;


    struct {
        uint32 num_outputs;
    } outputs;

    void InitShaderLocs();
};
struct shader_HDRI_to_cubemap {
    uint32 Handle;

    ShaderUniform_mat4 r_Projection;
    ShaderUniform_mat4 r_View;
    ShaderUniform_sampler2D u_hdri;

    struct {
        uint32 num_outputs;
        uint32 FragColor;
    } outputs;

    void InitShaderLocs();
};
struct shader_IBL_Prefilter {
    uint32 Handle;

    ShaderUniform_mat4 r_Projection;
    ShaderUniform_mat4 r_View;
    ShaderUniform_samplerCube u_env_cubemap;
    ShaderUniform_float r_roughness;

    struct {
        uint32 num_outputs;
        uint32 FragColor;
    } outputs;

    void InitShaderLocs();
};
struct shader_Irradiance_Convolve {
    uint32 Handle;

    ShaderUniform_mat4 r_Projection;
    ShaderUniform_mat4 r_View;
    ShaderUniform_samplerCube u_env_cubemap;

    struct {
        uint32 num_outputs;
        uint32 FragColor;
    } outputs;

    void InitShaderLocs();
};
struct shader_Lighting {
    uint32 Handle;

    ShaderUniform_mat4 r_Projection;
    ShaderUniform_Light r_pointLights[32];
    ShaderUniform_Light r_spotLights[32];
    ShaderUniform_Light r_sun;
    ShaderUniform_mat4 r_View;
    ShaderUniform_sampler2D u_albedo;
    ShaderUniform_sampler2D u_normal;
    ShaderUniform_sampler2D u_depth;
    ShaderUniform_sampler2D u_amr;
    ShaderUniform_samplerCube u_irradiance;
    ShaderUniform_samplerCube u_prefilter;
    ShaderUniform_sampler2D   u_brdf_LUT;

    struct {
        uint32 num_outputs;
        uint32 out_Diffuse;
        uint32 out_Specular;
    } outputs;

    void InitShaderLocs();
};
struct shader_Line {
    uint32 Handle;

    ShaderUniform_mat4 r_VP;
    ShaderUniform_mat4 r_Transform;
    ShaderUniform_vec3 r_CamPos;
    ShaderUniform_vec3 r_LineColor;
    ShaderUniform_float r_LineFadeStart;
    ShaderUniform_float r_LineFadeEnd;
    ShaderUniform_float r_LineFadeMaximum;
    ShaderUniform_float r_LineFadeMinimum;

    struct {
        uint32 num_outputs;
        uint32 FragColor;
    } outputs;

    void InitShaderLocs();
};
struct shader_Line2D {
    uint32 Handle;

    ShaderUniform_vec2 r_verts[2];
    ShaderUniform_mat4 r_orthoProjection;
    ShaderUniform_vec4 r_Color;

    struct {
        uint32 num_outputs;
    } outputs;

    void InitShaderLocs();
};
struct shader_Line3D {
    uint32 Handle;

    ShaderUniform_vec3 r_verts[2];
    ShaderUniform_mat4 r_View;
    ShaderUniform_mat4 r_Projection;
    ShaderUniform_vec4 r_Color;

    struct {
        uint32 num_outputs;
    } outputs;

    void InitShaderLocs();
};
struct shader_Mix {
    uint32 Handle;

    ShaderUniform_sampler2D r_tex1;
    ShaderUniform_sampler2D r_tex2;
    ShaderUniform_float r_mixValue;

    struct {
        uint32 num_outputs;
    } outputs;

    void InitShaderLocs();
};
struct shader_Normals {
    uint32 Handle;

    ShaderUniform_mat4 r_Transform;
    ShaderUniform_mat4 r_VP;

    struct {
        uint32 num_outputs;
        uint32 FragColor;
    } outputs;

    void InitShaderLocs();
};
struct shader_PBR_static {
    uint32 Handle;

    ShaderUniform_mat4 r_VP;
    ShaderUniform_mat4 r_Transform;
    ShaderUniform_Light r_pointLights[32];
    ShaderUniform_Light r_spotLights[32];
    ShaderUniform_Light r_sun;
    ShaderUniform_vec3 r_CamPos;
    ShaderUniform_sampler2D u_AlbedoTexture;
    ShaderUniform_sampler2D u_NormalTexture;
    ShaderUniform_sampler2D u_MetalnessTexture;
    ShaderUniform_sampler2D u_RoughnessTexture;
    ShaderUniform_vec3 u_AlbedoColor;
    ShaderUniform_float u_Metalness;
    ShaderUniform_float u_Roughness;
    ShaderUniform_float r_AlbedoTexToggle;
    ShaderUniform_float r_NormalTexToggle;
    ShaderUniform_float r_MetalnessTexToggle;
    ShaderUniform_float r_RoughnessTexToggle;

    struct {
        uint32 num_outputs;
        uint32 FragColor;
    } outputs;

    void InitShaderLocs();
};
struct shader_PrePass {
    uint32 Handle;

    ShaderUniform_mat4 r_Transform;
    ShaderUniform_mat4 r_View;
    ShaderUniform_mat4 r_Projection;
    ShaderUniform_sampler2D u_AlbedoTexture;
    ShaderUniform_sampler2D u_NormalTexture;
    ShaderUniform_sampler2D u_MetalnessTexture;
    ShaderUniform_sampler2D u_RoughnessTexture;
    ShaderUniform_sampler2D u_AmbientTexture;
    ShaderUniform_sampler2D u_EmissiveTexture;
    ShaderUniform_vec3 u_AlbedoColor;
    ShaderUniform_float u_Metalness;
    ShaderUniform_float u_Roughness;
    ShaderUniform_float u_TextureScale;
    ShaderUniform_float r_AlbedoTexToggle;
    ShaderUniform_float r_NormalTexToggle;
    ShaderUniform_float r_MetalnessTexToggle;
    ShaderUniform_float r_RoughnessTexToggle;
    ShaderUniform_float r_AmbientTexToggle;
    ShaderUniform_float r_EmissiveTexToggle;
    ShaderUniform_float r_gammaCorrect;

    struct {
        uint32 num_outputs;
        uint32 out_Albedo;
        uint32 out_Normal;
        uint32 out_AMR;
        uint32 out_Emissive;
        uint32 out_Depth;
    } outputs;

    void InitShaderLocs();
};
struct shader_PrePass_Anim {
    uint32 Handle;

    ShaderUniform_mat4 r_Bones[128];
    ShaderUniform_mat4 r_Transform;
    ShaderUniform_mat4 r_View;
    ShaderUniform_mat4 r_Projection;
    ShaderUniform_sampler2D u_AlbedoTexture;
    ShaderUniform_sampler2D u_NormalTexture;
    ShaderUniform_sampler2D u_MetalnessTexture;
    ShaderUniform_sampler2D u_RoughnessTexture;
    ShaderUniform_sampler2D u_AmbientTexture;
    ShaderUniform_sampler2D u_EmissiveTexture;
    ShaderUniform_vec3 u_AlbedoColor;
    ShaderUniform_float u_Metalness;
    ShaderUniform_float u_Roughness;
    ShaderUniform_float u_TextureScale;
    ShaderUniform_float r_AlbedoTexToggle;
    ShaderUniform_float r_NormalTexToggle;
    ShaderUniform_float r_MetalnessTexToggle;
    ShaderUniform_float r_RoughnessTexToggle;
    ShaderUniform_float r_AmbientTexToggle;
    ShaderUniform_float r_EmissiveTexToggle;
    ShaderUniform_float r_gammaCorrect;

    struct {
        uint32 num_outputs;
        uint32 out_Albedo;
        uint32 out_Normal;
        uint32 out_AMR;
        uint32 out_Depth;
        uint32 out_Emissive;
    } outputs;

    void InitShaderLocs();
};
struct shader_Screen {
    uint32 Handle;

    ShaderUniform_sampler2D u_albedo;
    ShaderUniform_sampler2D u_normal;
    ShaderUniform_sampler2D u_amr;
    ShaderUniform_sampler2D u_depth;
    ShaderUniform_sampler2D u_diffuse;
    ShaderUniform_sampler2D u_specular;
    ShaderUniform_sampler2D u_emissive;
    ShaderUniform_sampler2D u_ssao;
    ShaderUniform_int r_outputSwitch;
    ShaderUniform_float r_toneMap;
    ShaderUniform_float r_gammaCorrect;

    struct {
        uint32 num_outputs;
    } outputs;

    void InitShaderLocs();
};
struct shader_simple {
    uint32 Handle;

    ShaderUniform_mat4 r_VP;
    ShaderUniform_mat4 r_Transform;
    ShaderUniform_sampler2D u_texture;
    ShaderUniform_vec3 u_color;

    struct {
        uint32 num_outputs;
        uint32 FragColor;
    } outputs;

    void InitShaderLocs();
};
struct shader_Simple_static {
    uint32 Handle;

    ShaderUniform_mat4 r_VP;
    ShaderUniform_mat4 r_Transform;
    ShaderUniform_sampler2D u_texture;
    ShaderUniform_vec4 u_color;

    struct {
        uint32 num_outputs;
        uint32 FragColor;
    } outputs;

    void InitShaderLocs();
};
struct shader_simple_triplanar {
    uint32 Handle;

    ShaderUniform_mat4 r_VP;
    ShaderUniform_mat4 r_Transform;
    ShaderUniform_sampler2D u_texture;
    ShaderUniform_vec4 u_color;

    struct {
        uint32 num_outputs;
        uint32 FragColor;
    } outputs;

    void InitShaderLocs();
};
struct shader_Skybox {
    uint32 Handle;

    ShaderUniform_mat4 r_inverseVP;
    ShaderUniform_samplerCube r_skybox;

    struct {
        uint32 num_outputs;
    } outputs;

    void InitShaderLocs();
};
struct shader_Sobel {
    uint32 Handle;

    ShaderUniform_sampler2D r_texture;

    struct {
        uint32 num_outputs;
    } outputs;

    void InitShaderLocs();
};
struct shader_Sprite {
    uint32 Handle;

    ShaderUniform_vec4 r_transform;
    ShaderUniform_vec4 r_transformUV;
    ShaderUniform_mat4 r_orthoProjection;
    ShaderUniform_sampler2D r_spriteTex;
    ShaderUniform_vec3 r_textColor;

    struct {
        uint32 num_outputs;
    } outputs;

    void InitShaderLocs();
};
struct shader_SSAO {
    uint32 Handle;

    ShaderUniform_sampler2D u_amr;

    struct {
        uint32 num_outputs;
        uint32 out_SSAO;
    } outputs;

    void InitShaderLocs();
};
struct shader_wireframe {
    uint32 Handle;

    ShaderUniform_mat4 r_VP;
    ShaderUniform_mat4 r_Transform;
    ShaderUniform_sampler2D u_texture;
    ShaderUniform_vec4 u_color;

    struct {
        uint32 num_outputs;
        uint32 FragColor;
    } outputs;

    void InitShaderLocs();
};
#endif // __SHADERS_GENERATED_H__
// End of codegen
