#include <enpch.hpp>
#include "Renderer.hpp"

#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/Framebuffer.hpp"
#include "Engine/Renderer/Buffer.hpp"
#include "Engine/Renderer/TextRenderer.hpp"

#include "Engine/Sound/SoundEngine.hpp"
#include "Engine/Core/Input.hpp"

#include "Engine/Resources/MaterialCatalog.hpp"

namespace rh {

    struct Lightingdata { // holds view-space lighting data
        u32 NumPointLights;
        u32 NumSpotLights;
        Light pointLights[32];
        Light spotLights[32];
        Light sun;

        laml::Mat4 projection;
        laml::Mat4 view;
    };

    struct RendererData {
        std::unique_ptr<rh::ShaderLibrary> ShaderLibrary;
        TextureCube* Skybox;

        Ref<VertexArray> FullscreenQuad;
        Ref<VertexArray> debug_coordinate_axis; //lineRender
        Ref<VertexArray> Line;

        // Render buffers
        Ref<Framebuffer> gBuffer;
        Ref<Framebuffer> DiffuseSpecularLighting;
        Ref<Framebuffer> SSAO;
        Ref<Framebuffer> screenBuffer;
        Ref<Framebuffer> sobelBuffer;
        Ref<Framebuffer> mixBuffer1, mixBuffer2;

        Lightingdata Lights;

        u32 OutputMode;
        bool ToneMap;
        bool Gamma;
        bool soundDebug;
        bool controllerDebug;
    };

    RendererData s_Data;

    void InitLights(const Ref<Shader> shader) {
        // uploads lights to shader in view-space

        // set directional light
        shader->SetVec3("r_sun.Direction", laml::Vec3(0, 0, 0));
        shader->SetVec3("r_sun.Color", laml::Vec3(0, 0, 0));
        shader->SetFloat("r_sun.Strength", 0);

        // set point lights
        for (int n = 0; n < 32; n++) {
            shader->SetVec3("r_pointLights[" + std::to_string(n) + "].Position", laml::Vec3(0, 0, 0));
            shader->SetVec3("r_pointLights[" + std::to_string(n) + "].Color", laml::Vec3(0, 0, 0));
            shader->SetFloat("r_pointLights[" + std::to_string(n) + "].Strength", 0);
        }

        // set spot lights
        for (int n = 0; n < 32; n++) {
            shader->SetVec3("r_spotLights[" + std::to_string(n) + "].Position", laml::Vec3(0,0,0));
            shader->SetVec3("r_spotLights[" + std::to_string(n) + "].Direction", laml::Vec3(0, 0, 0));
            shader->SetVec3("r_spotLights[" + std::to_string(n) + "].Color", laml::Vec3(0, 0, 0));
            shader->SetFloat("r_spotLights[" + std::to_string(n) + "].Strength", 0);
            shader->SetFloat("r_spotLights[" + std::to_string(n) + "].Inner", 0);
            shader->SetFloat("r_spotLights[" + std::to_string(n) + "].Outer", 0);
        }
    }

    void Renderer::Init() {
        BENCHMARK_FUNCTION();
        RenderCommand::Init();

        s_Data.ShaderLibrary = std::make_unique<ShaderLibrary>();
        //Renderer::GetShaderLibrary()->Load("Data/Shaders/PBR_static.glsl");
        //Renderer::GetShaderLibrary()->Load("Data/Shaders/Skybox.glsl");
        Renderer::GetShaderLibrary()->Load("Data/Shaders/Line.glsl");
        Renderer::GetShaderLibrary()->Load("Data/Shaders/Line3D.glsl");
        //Renderer::GetShaderLibrary()->Load("Data/Shaders/Normals.glsl");

        Renderer::GetShaderLibrary()->Load("Data/Shaders/PrePass.glsl");
        Renderer::GetShaderLibrary()->Load("Data/Shaders/PrePass_Anim.glsl");

        Renderer::GetShaderLibrary()->Load("Data/Shaders/Lighting.glsl");
        Renderer::GetShaderLibrary()->Load("Data/Shaders/SSAO.glsl");
        Renderer::GetShaderLibrary()->Load("Data/Shaders/Screen.glsl");
        Renderer::GetShaderLibrary()->Load("Data/Shaders/Sobel.glsl");
        Renderer::GetShaderLibrary()->Load("Data/Shaders/Mix.glsl");

        //s_Data.Skybox = TextureCube::Create("Data/Images/DebugCubeMap.tga");
        s_Data.Skybox = MaterialCatalog::GetTextureCube("Data/Images/snowbox.png");

        // Create fullscreen quad
        {
            float x = -1;
            float y = -1;
            float width = 2, height = 2;
            struct _vertex
            {
                laml::Vec3 Position;
                laml::Vec2 TexCoord;
            };

            _vertex* data = new _vertex[4];

            data[0].Position = laml::Vec3(x, y, 0.1f);
            data[0].TexCoord = laml::Vec2(0.0f, 0.0f);

            data[1].Position = laml::Vec3(x + width, y, 0.1f);
            data[1].TexCoord = laml::Vec2(1.0f, 0.0f);

            data[2].Position = laml::Vec3(x + width, y + height, 0.1f);
            data[2].TexCoord = laml::Vec2(1.0f, 1.0f);

            data[3].Position = laml::Vec3(x, y + height, 0.1f);
            data[3].TexCoord = laml::Vec2(0.0f, 1.0f);

            u32 indices[6] = { 0, 1, 2, 2, 3, 0 };


            auto vbo = VertexBuffer::Create(data, 4 * sizeof(_vertex));
            vbo->SetLayout({
                {ShaderDataType::Float3, "a_Position"} ,
                {ShaderDataType::Float2, "a_TexCoord"}
                });
            auto ebo = IndexBuffer::Create(indices, 6);

            s_Data.FullscreenQuad = VertexArray::Create();
            s_Data.FullscreenQuad->Bind();
            s_Data.FullscreenQuad->AddVertexBuffer(vbo);
            s_Data.FullscreenQuad->SetIndexBuffer(ebo);
            s_Data.FullscreenQuad->Unbind();
        }

        // Create debug meshes
        {
            struct _vertex
            {
                laml::Vec3 Position;
            };

            _vertex* data = new _vertex[6];
            float s = 0.5f;
            data[0].Position = laml::Vec3(-s, 0, 0);
            data[1].Position = laml::Vec3( s, 0, 0);
            data[2].Position = laml::Vec3(0, -s, 0);
            data[3].Position = laml::Vec3(0,  s, 0);
            data[4].Position = laml::Vec3(0, 0, -s);
            data[5].Position = laml::Vec3(0, 0,  s);

            u32 indices[6] = { 0, 1, 2, 3, 4, 5 };


            auto vbo = VertexBuffer::Create(data, 6 * sizeof(_vertex));
            vbo->SetLayout({
                { ShaderDataType::Float3, "a_Position" }
                });
            auto ebo = IndexBuffer::Create(indices, 6);

            s_Data.debug_coordinate_axis = VertexArray::Create();
            s_Data.debug_coordinate_axis->Bind();
            s_Data.debug_coordinate_axis->AddVertexBuffer(vbo);
            s_Data.debug_coordinate_axis->SetIndexBuffer(ebo);
            s_Data.debug_coordinate_axis->Unbind();
        }
        // create Line
        {
            struct _vertex
            {
                float Position; // can we remove this entirely?
            };

            _vertex* data = new _vertex[2];

            data[0].Position = 0;
            data[1].Position = 1;

            u32 indices[2] = { 0, 1 };

            auto vbo = VertexBuffer::Create(data, 2 * sizeof(_vertex));
            vbo->SetLayout({
                { ShaderDataType::Float, "a_Position" }
                });
            auto ebo = IndexBuffer::Create(indices, 2);

            s_Data.Line = VertexArray::Create();
            s_Data.Line->Bind();
            s_Data.Line->AddVertexBuffer(vbo);
            s_Data.Line->SetIndexBuffer(ebo);
            s_Data.Line->Unbind();
        }

        // create the pipeline buffers
        FramebufferSpecification gBufferSpec;
        gBufferSpec.Attachments = {
            FramebufferTextureFormat::RGBA8, // Albedo
            FramebufferTextureFormat::RGBA16F, // View-space normal
            FramebufferTextureFormat::RGBA8, // Ambient/Metallic/Roughness
            {FramebufferTextureFormat::RGBA16F, 100, 100, 100, 1},  // Distance
            FramebufferTextureFormat::RGBA8,  // Emissive
            FramebufferTextureFormat::Depth  // Depth-buffer
        };
        s_Data.gBuffer = Framebuffer::Create(gBufferSpec);

        FramebufferSpecification LightingBufferSpec;
        LightingBufferSpec.Attachments = {
            FramebufferTextureFormat::RGBA16F, // Diffuse
            FramebufferTextureFormat::RGBA16F, // Specular
        };
        s_Data.DiffuseSpecularLighting = Framebuffer::Create(LightingBufferSpec);

        FramebufferSpecification SSAOBufferSpec;
        SSAOBufferSpec.Attachments = {
            FramebufferTextureFormat::RGBA8, // SSAO output
        };
        s_Data.SSAO = Framebuffer::Create(SSAOBufferSpec);

        FramebufferSpecification ScreenBufferSpec;
        ScreenBufferSpec.SwapChainTarget = true;
        s_Data.screenBuffer = Framebuffer::Create(ScreenBufferSpec);

        FramebufferSpecification SobelBufferSpec;
        SobelBufferSpec.Attachments = {
            {FramebufferTextureFormat::RGBA8, 0, 0, 0, 0},
            FramebufferTextureFormat::Depth
        };
        s_Data.sobelBuffer = Framebuffer::Create(SobelBufferSpec);

        FramebufferSpecification MixBufferSpec;
        MixBufferSpec.Attachments = {
            { FramebufferTextureFormat::RGBA8, 0, 0, 0, 0 },
        };
        s_Data.mixBuffer1 = Framebuffer::Create(MixBufferSpec);
        s_Data.mixBuffer2 = Framebuffer::Create(MixBufferSpec);

        s_Data.OutputMode = 0;
        s_Data.ToneMap = true;
        s_Data.Gamma = true;
        s_Data.soundDebug = false;
        s_Data.controllerDebug = false;

        Precompute();
    }

    void Renderer::Precompute() {
        // do startup computations (calc IBL and image probes)
        // TODO: the scene probably runs this code instead of the renderer...
    }

    void Renderer::NextOutputMode() {
        s_Data.OutputMode++;
        std::vector<std::string> outputModes = {
            "Combined Output",
            "Albedo",
            "View-Space Normals",
            "Baked Ambient Occlusion",
            "Metalness",
            "Roughness",
            "Ambient/Metallic/Roughness",
            "Depth",
            "Diffuse Lighting",
            "Specular Lighting",
            "Emission",
            "SSAO + Baked ao"
        };
        if (s_Data.OutputMode == outputModes.size())
            s_Data.OutputMode = 0;

        ENGINE_LOG_INFO("Output mode {1}:{0}", outputModes[s_Data.OutputMode], s_Data.OutputMode);
    }

    void Renderer::Shutdown() {
        RenderCommand::Shutdown();
    }


    void DrawSkybox() {
        /*
        // TODO: make this work with a deferred renderer :(
        auto skyboxShader = s_Data.ShaderLibrary->Get("Skybox");
        skyboxShader->Bind();
        skyboxShader->SetMat4("r_inverseVP", viewProj);
        auto samplers = skyboxShader->GetSamplers();
        for (auto& s : samplers) {
        if (s->GetName().compare("r_skybox") == 0)
        s_Data.Skybox->Bind(s->GetID());
        }
        SubmitFullscreenQuad(); // draw skybox
        */
    }

    void Renderer::UpdateLighting(const laml::Mat4& ViewMatrix,
        u32 numPointLights, const Light pointLights[32],
        u32 numSpotLights, const Light spotLights[32],
        const Light& sun, const laml::Mat4& projection) {
        BENCHMARK_FUNCTION();

        s_Data.Lights.NumPointLights = numPointLights;
        s_Data.Lights.NumSpotLights = numSpotLights;
        memcpy(&s_Data.Lights.sun, &(sun), sizeof(Light));
        memcpy(s_Data.Lights.pointLights, pointLights, sizeof(Light)*numPointLights);
        memcpy(s_Data.Lights.spotLights, spotLights, sizeof(Light)*numSpotLights);

        laml::Mat4 normalMatrix = laml::Mat4(ViewMatrix); //TODO: check the math to see if this is needed

        // recaulculate position and direction in view-space
        for (int n = 0; n < numPointLights; n++) {
            s_Data.Lights.pointLights[n].position = laml::transform::transform_point(ViewMatrix, s_Data.Lights.pointLights[n].position, 1.0f);
        }
        for (int n = 0; n < numSpotLights; n++) {
            s_Data.Lights.spotLights[n].position = laml::transform::transform_point(ViewMatrix, s_Data.Lights.spotLights[n].position, 1.0f);
            s_Data.Lights.spotLights[n].direction = laml::normalize(laml::transform::transform_point(normalMatrix, s_Data.Lights.spotLights[n].direction, 0.0f));
        }
        s_Data.Lights.sun.direction = laml::normalize(laml::transform::transform_point(normalMatrix, s_Data.Lights.sun.direction, 0.0f));

        // for debugging
        s_Data.Lights.projection = projection;
        s_Data.Lights.view = ViewMatrix;
    }

    void Renderer::PrintState() {
        ENGINE_LOG_TRACE("View-space pl position: <{0},{1},{2}>", 
            s_Data.Lights.pointLights[1].position.x, 
            s_Data.Lights.pointLights[1].position.y,
            s_Data.Lights.pointLights[1].position.z);
    }

    void Renderer::ToggleToneMapping() {
        s_Data.ToneMap = !s_Data.ToneMap;
        ENGINE_LOG_INFO("Tone Mapping: {0}", s_Data.ToneMap);
    }

    void Renderer::ToggleGammaCorrection() {
        s_Data.Gamma = !s_Data.Gamma;
        ENGINE_LOG_INFO("Gamma Correction: {0}", s_Data.Gamma);
    }

    void Renderer::ToggleDebugSoundOutput() {
        s_Data.soundDebug = !s_Data.soundDebug;
        ENGINE_LOG_INFO("Showing Sound Debug: {0}", s_Data.soundDebug);
    }

    void Renderer::ToggleDebugControllerOutput() {
        s_Data.controllerDebug = !s_Data.controllerDebug;
        ENGINE_LOG_INFO("Showing Controller Debug: {0}", s_Data.controllerDebug);
    }

    void Renderer::UploadLights(const Ref<Shader> shader) {
        // uploads lights to shader in view-space

        // set directional light
        shader->SetVec3("r_sun.Direction", s_Data.Lights.sun.direction);
        shader->SetVec3("r_sun.Color", s_Data.Lights.sun.color);
        shader->SetFloat("r_sun.Strength", s_Data.Lights.sun.strength);

        // set point lights
        for (int n = 0; n < s_Data.Lights.NumPointLights; n++) {
            shader->SetVec3("r_pointLights[" + std::to_string(n) + "].Position", s_Data.Lights.pointLights[n].position);
            shader->SetVec3("r_pointLights[" + std::to_string(n) + "].Color", s_Data.Lights.pointLights[n].color);
            shader->SetFloat("r_pointLights[" + std::to_string(n) + "].Strength", s_Data.Lights.pointLights[n].strength);
        }

        // set spot lights
        for (int n = 0; n < s_Data.Lights.NumSpotLights; n++) {
            shader->SetVec3("r_spotLights[" + std::to_string(n) + "].Position", s_Data.Lights.spotLights[n].position);
            shader->SetVec3("r_spotLights[" + std::to_string(n) + "].Direction", s_Data.Lights.spotLights[n].direction);
            shader->SetVec3("r_spotLights[" + std::to_string(n) + "].Color", s_Data.Lights.spotLights[n].color);
            shader->SetFloat("r_spotLights[" + std::to_string(n) + "].Strength", s_Data.Lights.spotLights[n].strength);
            shader->SetFloat("r_spotLights[" + std::to_string(n) + "].Inner", s_Data.Lights.spotLights[n].inner);
            shader->SetFloat("r_spotLights[" + std::to_string(n) + "].Outer", s_Data.Lights.spotLights[n].outer);
        }
    }

    void Renderer::Begin3DScene(const Camera& camera, const laml::Mat4& transform, 
        u32 numPointLights, const Light pointLights[32],
        u32 numSpotLights, const Light spotLights[32],
        const Light& sun) {
        BENCHMARK_FUNCTION();

        laml::Mat4 ViewMatrix;
        laml::transform::create_view_matrix_from_transform(ViewMatrix, transform);
        laml::Mat4 ProjectionMatrix = camera.GetProjection();
        laml::Vec3 camPos(transform.c_14, transform.c_24, transform.c_34);
        UpdateLighting(ViewMatrix, numPointLights, pointLights, numSpotLights, spotLights, sun, ProjectionMatrix);

        // PrePass Shader
        auto prePassShader = s_Data.ShaderLibrary->Get("PrePass");
        prePassShader->Bind();
        prePassShader->SetMat4("r_Projection", ProjectionMatrix);
        prePassShader->SetMat4("r_View", ViewMatrix);
        prePassShader->SetFloat("r_AlbedoTexToggle",    1.0f);
        prePassShader->SetFloat("r_NormalTexToggle",    0.0f);
        prePassShader->SetFloat("r_MetalnessTexToggle", 1.0f);
        prePassShader->SetFloat("r_RoughnessTexToggle", 1.0f);
        prePassShader->SetFloat("r_AmbientTexToggle",   1.0f);
        prePassShader->SetFloat("r_EmissiveTexToggle",  1.0f);
        prePassShader->SetFloat("r_gammaCorrect", s_Data.Gamma ? 1.0 : 0.0);

        // PrePass_Anim Shader
        auto prePassAnimShader = s_Data.ShaderLibrary->Get("PrePass_Anim");
        prePassAnimShader->Bind();
        prePassAnimShader->SetMat4("r_Projection", ProjectionMatrix);
        prePassAnimShader->SetMat4("r_View", ViewMatrix);
        prePassAnimShader->SetFloat("r_AlbedoTexToggle", 1.0f);
        prePassAnimShader->SetFloat("r_NormalTexToggle", 0.0f);
        prePassAnimShader->SetFloat("r_MetalnessTexToggle", 1.0f);
        prePassAnimShader->SetFloat("r_RoughnessTexToggle", 1.0f);
        prePassAnimShader->SetFloat("r_AmbientTexToggle", 1.0f);
        prePassAnimShader->SetFloat("r_EmissiveTexToggle", 1.0f);
        prePassAnimShader->SetFloat("r_gammaCorrect", s_Data.Gamma ? 1.0 : 0.0);

        // Lighting Pass
        auto lightingShader = s_Data.ShaderLibrary->Get("Lighting");
        lightingShader->Bind();
        lightingShader->SetInt("u_normal", 0);
        lightingShader->SetInt("u_distance", 1);
        lightingShader->SetInt("u_amr", 2);
        UploadLights(lightingShader);
        lightingShader->SetMat4("r_Projection", s_Data.Lights.projection);
        lightingShader->SetMat4("r_View", s_Data.Lights.view);

        // SSAO Pass
        auto ssaoShader = s_Data.ShaderLibrary->Get("SSAO");
        ssaoShader->Bind();
        ssaoShader->SetInt("u_amr", 0);

        // Screen Output Pass
        auto screenShader = s_Data.ShaderLibrary->Get("Screen");
        screenShader->Bind();
        screenShader->SetInt("u_albedo", 0);
        screenShader->SetInt("u_normal", 1);
        screenShader->SetInt("u_amr", 2);
        screenShader->SetInt("u_depth", 3);
        screenShader->SetInt("u_diffuse", 4);
        screenShader->SetInt("u_specular", 5);
        screenShader->SetInt("u_emissive", 6);
        screenShader->SetInt("u_ssao", 7);
        screenShader->SetInt("r_outputSwitch", s_Data.OutputMode);
        screenShader->SetFloat("r_toneMap", s_Data.ToneMap ? 1.0 : 0.0);
        screenShader->SetFloat("r_gammaCorrect", s_Data.Gamma ? 1.0 : 0.0);

        // Line/Simple mesh Pass
        auto lineShader = s_Data.ShaderLibrary->Get("Line");
        lineShader->Bind();
        lineShader->SetMat4("r_VP", laml::mul(ProjectionMatrix, ViewMatrix));
        lineShader->SetVec3("r_CamPos", camPos);
        lineShader->SetFloat("r_LineFadeStart", 5);
        lineShader->SetFloat("r_LineFadeEnd", 20);
        lineShader->SetFloat("r_LineFadeMaximum", 0.5f);
        lineShader->SetFloat("r_LineFadeMinimum", 0.25f);

        // 3D Line shader
        auto line3DShader = s_Data.ShaderLibrary->Get("Line3D");
        line3DShader->Bind();
        line3DShader->SetMat4("r_Projection", ProjectionMatrix);
        line3DShader->SetMat4("r_View", ViewMatrix);

        // Sobel pass
        auto sobelShader = s_Data.ShaderLibrary->Get("Sobel");
        sobelShader->SetInt("r_texture", 0);
        auto mixShader = s_Data.ShaderLibrary->Get("Mix");
        mixShader->SetInt("r_tex1", 0);
        mixShader->SetInt("r_tex2", 1);
    }

    void Renderer::BeginDeferredPrepass() {
        BENCHMARK_FUNCTION();
        // Clear G-Buffer
        s_Data.gBuffer->Bind();
        s_Data.gBuffer->ClearBuffers();
        auto prePassShader = s_Data.ShaderLibrary->Get("PrePass");
        prePassShader->Bind();
    }

    void Renderer::EndDeferredPrepass() {
        BENCHMARK_FUNCTION();
        s_Data.gBuffer->Unbind();

        // Lighting Pass
        s_Data.DiffuseSpecularLighting->Bind();
        s_Data.DiffuseSpecularLighting->ClearBuffers();
        auto lightingShader = s_Data.ShaderLibrary->Get("Lighting");
        lightingShader->Bind();
        s_Data.gBuffer->BindTexture(1, 0);
        s_Data.gBuffer->BindTexture(3, 1);
        s_Data.gBuffer->BindTexture(2, 2);
        SubmitFullscreenQuad();
        s_Data.DiffuseSpecularLighting->Unbind();

        // SSAO pass
        s_Data.SSAO->Bind();
        s_Data.SSAO->ClearBuffers();
        auto ssaoShader = s_Data.ShaderLibrary->Get("SSAO");
        ssaoShader->Bind();
        s_Data.gBuffer->BindTexture(2, 0);
        SubmitFullscreenQuad();
        s_Data.SSAO->Unbind();

        // Output to mixBuffer1
        s_Data.mixBuffer1->Bind();
        s_Data.mixBuffer1->ClearBuffers();
        auto screenShader = s_Data.ShaderLibrary->Get("Screen");
        screenShader->Bind();
        s_Data.gBuffer->BindTexture(0, 0);
        s_Data.gBuffer->BindTexture(1, 1);
        s_Data.gBuffer->BindTexture(2, 2);
        s_Data.gBuffer->BindTexture(3, 3);
        s_Data.DiffuseSpecularLighting->BindTexture(0, 4);
        s_Data.DiffuseSpecularLighting->BindTexture(1, 5);
        s_Data.gBuffer->BindTexture(4, 6);
        s_Data.SSAO->BindTexture(0, 7);
        SubmitFullscreenQuad();
    }

    void Renderer::RenderDebugUI() {
        BENCHMARK_FUNCTION();

        s_Data.screenBuffer->Bind();

        // Render text
        std::vector<std::string> outputModes = {
            "Combined Output",
            "Albedo",
            "View-Space Normals",
            "Baked Ambient Occlusion",
            "Metalness",
            "Roughness",
            "Ambient/Metallic/Roughness",
            "Depth",
            "Diffuse Lighting",
            "Specular Lighting",
            "Emission",
            "SSAO + Baked ao"
        };

        TextRenderer::SubmitText(outputModes[s_Data.OutputMode], 10, 10, laml::Vec3(.1f, .9f, .75f));

        // Render sound debug
        if (s_Data.soundDebug) {
            auto status = SoundEngine::GetStatus();
            float startx = 10, starty = 150;
            if (NumSoundChannels > 16) starty = 30;
            float fontSize = 20;
            char text[64];
            TextRenderer::SubmitText("Sound Engine Status:", startx, starty, laml::Vec3(.6f, .8f, .75f));
            for (int n = 0; n < NumSoundChannels; n++) {
                if (status.channels[n].active) {
                    sprintf_s(text, 64, "Channel %2d: %0.2f/%0.2f %2d:[%s]", n, 
                        status.channels[n].current, 
                        status.channels[n].length,
                        status.channels[n].soundID,
                        status.channels[n].cue.c_str());
                    TextRenderer::SubmitText(text, startx + 15, starty += fontSize, laml::Vec3(.6f, .8f, .75f));
                } else {
                    sprintf_s(text, 64, "Channel %2d: inactive", n);
                    TextRenderer::SubmitText(text, startx+15, starty+= fontSize, laml::Vec3(.4f, .6f, .55f));
                }
            }
            sprintf_s(text, 64, "Sounds in queue: %d", status.queueSize);
            TextRenderer::SubmitText(text, startx, starty += fontSize, laml::Vec3(.6f, .8f, .75f));
        }

        // Render input debug
        if (s_Data.controllerDebug) {
            const auto& state = Input::GetState();
            float startx = 10, starty = 150;
            if (s_Data.soundDebug) startx = 250;
            float fontSize = 20;
            char text[64];
            if (!state.present || !state.valid) {
                TextRenderer::SubmitText("Gamepad Status: disconnected", startx, starty, laml::Vec3(.6f, .8f, .75f));
            } else {
                sprintf_s(text, 64, "Gamepad state: [%s]", state.name);
                TextRenderer::SubmitText(text, startx, starty, laml::Vec3(.6f, .8f, .75f));
                for (int n = 0; n < 6; n++) {
                    sprintf_s(text, 64, "Axis %d: %.2f", n, state.axes[n]);
                    TextRenderer::SubmitText(text, startx + 15, starty += fontSize, laml::Vec3(.6f, .8f, .75f));
                }
            }
        }

        s_Data.screenBuffer->Unbind();
    }

    void Renderer::BeginSobelPass() {
        BENCHMARK_FUNCTION();

        // prep the sobel buffer
        s_Data.sobelBuffer->Bind();
        s_Data.sobelBuffer->ClearBuffers();
    }

    void Renderer::EndSobelPass() {
        BENCHMARK_FUNCTION();

        // prep the screenbuffer, and urn the sobel shader
        s_Data.sobelBuffer->Unbind();

        s_Data.mixBuffer2->Bind();
        s_Data.mixBuffer2->ClearBuffers();
        auto sobelShader = s_Data.ShaderLibrary->Get("Sobel");
        sobelShader->Bind();
        s_Data.sobelBuffer->BindTexture(0, 0);
        SubmitFullscreenQuad();
    }
    
    void Renderer::End3DScene() {
        BENCHMARK_FUNCTION();

        s_Data.screenBuffer->Bind();
        s_Data.screenBuffer->ClearBuffers();
        auto mixShader = s_Data.ShaderLibrary->Get("Mix");
        mixShader->Bind();
        s_Data.mixBuffer1->BindTexture(0, 0);
        s_Data.mixBuffer2->BindTexture(0, 1);
        SubmitFullscreenQuad();

        Flush();
    }

    void Renderer::Flush() {
        // currently doing nothing ^_^
    }

    void Renderer::OnWindowResize(uint32_t width, uint32_t height)
    {
        RenderCommand::SetViewport(0, 0, width, height);
    }

    const std::unique_ptr<ShaderLibrary>& Renderer::GetShaderLibrary() {
        return s_Data.ShaderLibrary;
    }

    void Renderer::Submit(const laml::Mat4& transform) {
        auto shader = s_Data.ShaderLibrary->Get("simple");
        shader->Bind();
        shader->SetMat4("r_Transform", transform);

        //s_Data.VertexArray->Bind();
        //RenderCommand::DrawIndexed(s_Data.VertexArray);
    }

    void Renderer::Submit(const Ref<VertexArray>& vao, const laml::Mat4& transform,
        const laml::Vec3& color) {

        auto shader = s_Data.ShaderLibrary->Get("Line");
        shader->Bind();
        shader->SetMat4("r_Transform", transform);
        shader->SetVec3("r_LineColor", color);

        vao->Bind();
        //RenderCommand::SetWireframe(true);
        RenderCommand::DrawIndexed(vao, true);
        //RenderCommand::SetWireframe(false);
    }

    void Renderer::SubmitMesh(const Mesh* mesh, const laml::Mat4& transform) {
        BENCHMARK_FUNCTION();

        mesh->GetVertexArray()->Bind();
        auto shader = mesh->GetMeshShader();
        shader->Bind();

        auto& materials = mesh->GetMaterials();
        for (const Submesh& submesh : mesh->GetSubmeshes()) {
            auto material = materials[submesh.MaterialIndex];
            material->Bind();

            shader->SetMat4("r_Transform", laml::mul(transform, submesh.Transform));

            // set bone transforms
            const auto& skeleton = mesh->GetSkeleton();
            for (int n = 0; n < skeleton.bones.size(); n++) {
                const auto& bone = skeleton.bones[n];

                laml::Mat4 fix(1.0f);

                //shader->SetMat4("r_Bones[" + std::to_string(n) + "]", bone1.transform * bone0.invTransform);
                shader->SetMat4("r_Bones[" + std::to_string(n) + "]", laml::mul(bone.finalTransform, bone.inverse_model_matrix));
                //shader->SetMat4("r_Bones[" + std::to_string(n) + "]", laml::Mat4());
            }

            //RenderCommand::DrawIndexed(mesh->GetVertexArray());
            RenderCommand::DrawSubIndexed(submesh.BaseIndex, 0, submesh.IndexCount);
        }
    }

    // Animation variant
    // ANIM_HOOK
    //void Renderer::SubmitMesh(const Mesh* mesh, const laml::Mat4& transform, md5::Animation* anim) {
    //    BENCHMARK_FUNCTION();
    //
    //    mesh->GetVertexArray()->Bind();
    //    auto shader = mesh->GetMeshShader();
    //    shader->Bind();
    //
    //    // set bone transforms
    //    const auto& bindPose = mesh->GetBindPose();
    //    const auto& Pose = anim->AnimatedSkeleton.Joints;
    //    assert(bindPose.size() == Pose.size());
    //    for (int n = 0; n < bindPose.size(); n++) {
    //        const auto& bone0 = bindPose[n];
    //        const auto& bone1 = Pose[n];
    //
    //        //shader->SetMat4("r_Bones[" + std::to_string(n) + "]", bone1.transform * bone0.invTransform);
    //        shader->SetMat4("r_Bones[" + std::to_string(n) + "]", laml::Mat4());
    //    }
    //
    //    auto& materials = mesh->GetMaterials();
    //    for (const Submesh& submesh : mesh->GetSubmeshes()) {
    //        auto material = materials[submesh.MaterialIndex];
    //        material->Bind();
    //
    //        shader->SetMat4("r_Transform", transform * submesh.Transform);
    //
    //        //RenderCommand::DrawIndexed(mesh->GetVertexArray());
    //        RenderCommand::DrawSubIndexed(submesh.BaseIndex, 0, submesh.IndexCount);
    //    }
    //}

    void Renderer::SubmitMesh_drawNormals(const Ref<Mesh>& mesh, const laml::Mat4& transform) {
        mesh->GetVertexArray()->Bind();
        auto shader = s_Data.ShaderLibrary->Get("Normals");
        shader->Bind();

        for (Submesh& submesh : mesh->GetSubmeshes()) {
            shader->SetMat4("r_Transform", laml::mul(transform, submesh.Transform));

            RenderCommand::DrawSubIndexed_points(submesh.BaseIndex, 0, submesh.IndexCount);
        }
    }

    void Renderer::SubmitLine(const laml::Vec3& v0, const laml::Vec3& v1, const laml::Vec4& color) {
        auto shader = s_Data.ShaderLibrary->Get("Line3D");
        shader->Bind();

        shader->SetVec3("r_verts[0]", v0);
        shader->SetVec3("r_verts[1]", v1);
        shader->SetVec4("r_Color", color);

        s_Data.Line->Bind();
        RenderCommand::DrawLines(s_Data.Line, false);
    }

    void Renderer::SubmitFullscreenQuad() {
        s_Data.FullscreenQuad->Bind();
        RenderCommand::DrawIndexed(s_Data.FullscreenQuad, false);
    }

    void Renderer::RecompileShaders() {
        s_Data.ShaderLibrary->ReloadAll();
    }

    void Renderer::Draw3DText(const std::string& text, const laml::Vec3& pos, const laml::Vec3& color) {
        laml::Vec4 screenSpace = laml::transform::transform_point(laml::mul(s_Data.Lights.projection, s_Data.Lights.view), laml::Vec4(pos, 1.0f));

        screenSpace = screenSpace / screenSpace.w;
        screenSpace = screenSpace + laml::Vec4(1.0f, 1.0f, 1.0f, 1.0f);
        screenSpace = screenSpace * 0.5f;
        screenSpace.x *= 1280;
        screenSpace.y *= 720;
        TextRenderer::SubmitText(text, screenSpace.x, 720-screenSpace.y, color);
    }

    // Draw skeleton
    void Renderer::DrawSkeletonDebug(
        const TagComponent& tag,
        const TransformComponent& transform,
        const MeshRendererComponent& mesh,
        const laml::Vec3& text_color, bool bind_pose) {
    
        //laml::Mat3 BlenderCorrection(laml::Vec3(0, 0, 1), laml::Vec3(1, 0, 0), laml::Vec3(0, 1, 0));
        //laml::Mat4 T = laml::mul(transform.Transform, mesh.MeshPtr->GetSubmeshes()[0].Transform);
        //laml::Mat4 T(100.0f, 100.0f, 100.0f, 1.0f);
        laml::Mat4 T = transform.Transform;
        float s = 0.075f;
        float length = 0.55f;
        float length2 = 0.05f;
    
        laml::Vec3 localR(1.0f, 0.0f, 0.0f);
        laml::Vec3 localF(0.0f, 1.0f, 0.0f);
        laml::Vec3 localU(0.0f, 0.0f, 1.0f);

        laml::Vec4 bone_color = laml::Vec4(text_color.x, text_color.y, text_color.z, 1.0f) * 0.75f;
    
        //for (const auto& joint : anim.Anim->AnimatedSkeleton.Joints) {
        auto skele = mesh.MeshPtr->GetSkeleton();
        for (int j = 0; j < skele.num_bones; j++) {
            const auto& joint = skele.bones[j];

            laml::Vec3 translation, scale;
            laml::Mat3 rotation;
            if (bind_pose) {
                laml::transform::decompose(skele.bones[j].model_matrix, rotation, translation, scale);
            } else {
                laml::transform::decompose(skele.bones[j].finalTransform, rotation, translation, scale);
            }
            laml::Vec3 start = translation;
            //ENGINE_LOG_DEBUG("{0} Translation: {1}", j, translation);
        
            laml::Vec3 boneR = laml::transform::transform_point(rotation, localR);
            laml::Vec3 boneU = laml::transform::transform_point(rotation, localU);
            laml::Vec3 boneF = laml::transform::transform_point(rotation, localF);
        
            laml::Vec3 end = start + boneF * length;
            laml::Vec3 mid = start + boneF * length2;
        
            laml::Vec3 A = laml::transform::transform_point(T, mid + boneR * s, 1.0f);
            laml::Vec3 B = laml::transform::transform_point(T, mid + boneU * s, 1.0f);
            laml::Vec3 C = laml::transform::transform_point(T, mid - boneR * s, 1.0f);
            laml::Vec3 D = laml::transform::transform_point(T, mid - boneU * s, 1.0f);
        
            start = laml::transform::transform_point(T, start, 1.0f);
            end   = laml::transform::transform_point(T, end, 1.0f);
        
            Renderer::SubmitLine(start, A, bone_color);
            Renderer::SubmitLine(start, B, bone_color);
            Renderer::SubmitLine(start, C, bone_color);
            Renderer::SubmitLine(start, D, bone_color);
        
            Renderer::SubmitLine(A, end, bone_color);
            Renderer::SubmitLine(B, end, bone_color);
            Renderer::SubmitLine(C, end, bone_color);
            Renderer::SubmitLine(D, end, bone_color);
        
            Renderer::SubmitLine(A, B, bone_color);
            Renderer::SubmitLine(B, C, bone_color);
            Renderer::SubmitLine(C, D, bone_color);
            Renderer::SubmitLine(D, A, bone_color);
        
            // draw bone name
            laml::Vec4 screenSpace = laml::transform::transform_point(laml::mul(s_Data.Lights.projection, s_Data.Lights.view), laml::Vec4(end, 1));
            screenSpace = screenSpace / screenSpace.w;
            screenSpace = screenSpace + laml::Vec4(1, 1, 1, 1);
            screenSpace = screenSpace * 0.5f;
            screenSpace.x *= 1280;
            screenSpace.y *= 720;
            TextRenderer::SubmitText(joint.bone_name, (float)screenSpace.x, 720 - screenSpace.y, text_color);
        }
    }
}