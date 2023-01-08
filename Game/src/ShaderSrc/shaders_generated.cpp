// Code generated by W:\Rohin\unity_build\shader_tool.exe on Sunday, January 8, 2023
struct shader_Lighting {
    uint32 Handle;

    ShaderUniform_mat4 r_Projection;
    ShaderUniform_Light r_pointLights[32];
    ShaderUniform_Light r_spotLights[32];
    ShaderUniform_Light r_sun;
    ShaderUniform_mat4 r_View;
    ShaderUniform_sampler2D u_normal;
    ShaderUniform_sampler2D u_distance;
    ShaderUniform_sampler2D u_amr;

    void InitShaderLocs() {
        r_Projection.Location = 1;
        for (int n = 0; n < 32; n++) {
            r_pointLights[n].Position.Location = 2+(n*6);
            r_pointLights[n].Direction.Location = 3+(n*6);
            r_pointLights[n].Color.Location = 4+(n*6);
            r_pointLights[n].Strength.Location = 5+(n*6);
            r_pointLights[n].Inner.Location = 6+(n*6);
            r_pointLights[n].Outer.Location = 7+(n*6);
        }
        for (int n = 0; n < 32; n++) {
            r_spotLights[n].Position.Location = 194+(n*6);
            r_spotLights[n].Direction.Location = 195+(n*6);
            r_spotLights[n].Color.Location = 196+(n*6);
            r_spotLights[n].Strength.Location = 197+(n*6);
            r_spotLights[n].Inner.Location = 198+(n*6);
            r_spotLights[n].Outer.Location = 199+(n*6);
        }
        r_sun.Position.Location = 386;
        r_sun.Direction.Location = 387;
        r_sun.Color.Location = 388;
        r_sun.Strength.Location = 389;
        r_sun.Inner.Location = 390;
        r_sun.Outer.Location = 391;
        r_View.Location = 392;
        u_normal.Location = 393;
        u_distance.Location = 394;
        u_amr.Location = 395;
    }
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

    void InitShaderLocs() {
        r_VP.Location = 1;
        r_Transform.Location = 2;
        r_CamPos.Location = 3;
        r_LineColor.Location = 4;
        r_LineFadeStart.Location = 5;
        r_LineFadeEnd.Location = 6;
        r_LineFadeMaximum.Location = 7;
        r_LineFadeMinimum.Location = 8;
    }
};
struct shader_Line2D {
    uint32 Handle;

    ShaderUniform_vec2 r_verts[2];
    ShaderUniform_mat4 r_orthoProjection;
    ShaderUniform_vec4 r_Color;

    void InitShaderLocs() {
        for (int n = 0; n < 2; n++) {
            r_verts[n].Location = 1+n;
        }
        r_orthoProjection.Location = 3;
        r_Color.Location = 4;
    }
};
struct shader_Line3D {
    uint32 Handle;

    ShaderUniform_vec3 r_verts[2];
    ShaderUniform_mat4 r_View;
    ShaderUniform_mat4 r_Projection;
    ShaderUniform_vec4 r_Color;

    void InitShaderLocs() {
        for (int n = 0; n < 2; n++) {
            r_verts[n].Location = 1+n;
        }
        r_View.Location = 3;
        r_Projection.Location = 4;
        r_Color.Location = 5;
    }
};
struct shader_Mix {
    uint32 Handle;

    ShaderUniform_sampler2D r_tex1;
    ShaderUniform_sampler2D r_tex2;
    ShaderUniform_float r_mixValue;

    void InitShaderLocs() {
        r_tex1.Location = 1;
        r_tex2.Location = 2;
        r_mixValue.Location = 3;
    }
};
struct shader_Normals {
    uint32 Handle;

    ShaderUniform_mat4 r_Transform;
    ShaderUniform_mat4 r_VP;

    void InitShaderLocs() {
        r_Transform.Location = 1;
        r_VP.Location = 2;
    }
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

    void InitShaderLocs() {
        r_VP.Location = 1;
        r_Transform.Location = 2;
        for (int n = 0; n < 32; n++) {
            r_pointLights[n].Position.Location = 3+(n*6);
            r_pointLights[n].Direction.Location = 4+(n*6);
            r_pointLights[n].Color.Location = 5+(n*6);
            r_pointLights[n].Strength.Location = 6+(n*6);
            r_pointLights[n].Inner.Location = 7+(n*6);
            r_pointLights[n].Outer.Location = 8+(n*6);
        }
        for (int n = 0; n < 32; n++) {
            r_spotLights[n].Position.Location = 195+(n*6);
            r_spotLights[n].Direction.Location = 196+(n*6);
            r_spotLights[n].Color.Location = 197+(n*6);
            r_spotLights[n].Strength.Location = 198+(n*6);
            r_spotLights[n].Inner.Location = 199+(n*6);
            r_spotLights[n].Outer.Location = 200+(n*6);
        }
        r_sun.Position.Location = 387;
        r_sun.Direction.Location = 388;
        r_sun.Color.Location = 389;
        r_sun.Strength.Location = 390;
        r_sun.Inner.Location = 391;
        r_sun.Outer.Location = 392;
        r_CamPos.Location = 393;
        u_AlbedoTexture.Location = 394;
        u_NormalTexture.Location = 395;
        u_MetalnessTexture.Location = 396;
        u_RoughnessTexture.Location = 397;
        u_AlbedoColor.Location = 398;
        u_Metalness.Location = 399;
        u_Roughness.Location = 400;
        r_AlbedoTexToggle.Location = 401;
        r_NormalTexToggle.Location = 402;
        r_MetalnessTexToggle.Location = 403;
        r_RoughnessTexToggle.Location = 404;
    }
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

    void InitShaderLocs() {
        r_Transform.Location = 1;
        r_View.Location = 2;
        r_Projection.Location = 3;
        u_AlbedoTexture.Location = 4;
        u_NormalTexture.Location = 5;
        u_MetalnessTexture.Location = 6;
        u_RoughnessTexture.Location = 7;
        u_AmbientTexture.Location = 8;
        u_EmissiveTexture.Location = 9;
        u_AlbedoColor.Location = 10;
        u_Metalness.Location = 11;
        u_Roughness.Location = 12;
        u_TextureScale.Location = 13;
        r_AlbedoTexToggle.Location = 14;
        r_NormalTexToggle.Location = 15;
        r_MetalnessTexToggle.Location = 16;
        r_RoughnessTexToggle.Location = 17;
        r_AmbientTexToggle.Location = 18;
        r_EmissiveTexToggle.Location = 19;
        r_gammaCorrect.Location = 20;
    }
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

    void InitShaderLocs() {
        for (int n = 0; n < 128; n++) {
            r_Bones[n].Location = 1+n;
        }
        r_Transform.Location = 129;
        r_View.Location = 130;
        r_Projection.Location = 131;
        u_AlbedoTexture.Location = 132;
        u_NormalTexture.Location = 133;
        u_MetalnessTexture.Location = 134;
        u_RoughnessTexture.Location = 135;
        u_AmbientTexture.Location = 136;
        u_EmissiveTexture.Location = 137;
        u_AlbedoColor.Location = 138;
        u_Metalness.Location = 139;
        u_Roughness.Location = 140;
        u_TextureScale.Location = 141;
        r_AlbedoTexToggle.Location = 142;
        r_NormalTexToggle.Location = 143;
        r_MetalnessTexToggle.Location = 144;
        r_RoughnessTexToggle.Location = 145;
        r_AmbientTexToggle.Location = 146;
        r_EmissiveTexToggle.Location = 147;
        r_gammaCorrect.Location = 148;
    }
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

    void InitShaderLocs() {
        u_albedo.Location = 1;
        u_normal.Location = 2;
        u_amr.Location = 3;
        u_depth.Location = 4;
        u_diffuse.Location = 5;
        u_specular.Location = 6;
        u_emissive.Location = 7;
        u_ssao.Location = 8;
        r_outputSwitch.Location = 9;
        r_toneMap.Location = 10;
        r_gammaCorrect.Location = 11;
    }
};
struct shader_simple {
    uint32 Handle;

    ShaderUniform_mat4 r_VP;
    ShaderUniform_mat4 r_Transform;
    ShaderUniform_sampler2D u_texture;
    ShaderUniform_vec4 u_color;

    void InitShaderLocs() {
        r_VP.Location = 1;
        r_Transform.Location = 2;
        u_texture.Location = 3;
        u_color.Location = 4;
    }
};
struct shader_Simple_static {
    uint32 Handle;

    ShaderUniform_mat4 r_VP;
    ShaderUniform_mat4 r_Transform;
    ShaderUniform_sampler2D u_texture;
    ShaderUniform_vec4 u_color;

    void InitShaderLocs() {
        r_VP.Location = 1;
        r_Transform.Location = 2;
        u_texture.Location = 3;
        u_color.Location = 4;
    }
};
struct shader_Skybox {
    uint32 Handle;

    ShaderUniform_mat4 r_inverseVP;
    ShaderUniform_samplerCube r_skybox;

    void InitShaderLocs() {
        r_inverseVP.Location = 1;
        r_skybox.Location = 2;
    }
};
struct shader_Sobel {
    uint32 Handle;

    ShaderUniform_sampler2D r_texture;

    void InitShaderLocs() {
        r_texture.Location = 1;
    }
};
struct shader_Sprite {
    uint32 Handle;

    ShaderUniform_vec4 r_transform;
    ShaderUniform_vec4 r_transformUV;
    ShaderUniform_mat4 r_orthoProjection;
    ShaderUniform_sampler2D r_spriteTex;
    ShaderUniform_vec3 r_textColor;

    void InitShaderLocs() {
        r_transform.Location = 1;
        r_transformUV.Location = 2;
        r_orthoProjection.Location = 3;
        r_spriteTex.Location = 4;
        r_textColor.Location = 5;
    }
};
struct shader_SSAO {
    uint32 Handle;

    ShaderUniform_sampler2D u_amr;

    void InitShaderLocs() {
        u_amr.Location = 1;
    }
};
