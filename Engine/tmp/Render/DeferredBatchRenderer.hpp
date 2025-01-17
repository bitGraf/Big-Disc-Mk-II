#ifndef DEFERRED_RENDER_MANAGER_H
#define DEFERRED_RENDER_MANAGER_H

#include "GLFW/glfw3.h"
#include "Shader.hpp"
#include "Shadowmap.hpp"
#include "Engine/Resource/Texture.hpp"
#include "RenderBatch.hpp"
#include "DynamicFont.hpp"
#include "SpriteRenderer.hpp"
#include "Engine/Core/Utils.hpp"
#include "Engine/Resource/ResourceCatalog.hpp"

#include "Engine/Window/GBuffer.hpp"
#include "Engine/Window/Framebuffer.hpp"

#include <random>

class DeferredBatchRenderer {
public:
    DeferredBatchRenderer();
    ~DeferredBatchRenderer();

    void update(double dt);
    void destroy();

    /* System Unique functions */
    void renderBatch(RenderBatch* batch);
    void renderDebug(RenderBatch* batch,
        double frameCount, long long lastFrame,
        bool debugMode);

    void loadResources();

private:
    // Profiling
    using _clock = std::chrono::system_clock;
    using _time = std::chrono::system_clock::time_point;
    using _dur = long long;

    _time profileStart;
    _dur dur_fullRenderPass;
    MovingAverage<_dur, 100> avgRenderPass;

    void beginProfile();
    _dur profileRenderPass();
    void endProfile();

    // Geometry Pass
    void geometryPass(RenderBatch* batch);
    Shader m_geometryPassShader;
    _dur dur_geometryPass;
    //GBuffer m_gBuffer;
    Framebuffer_new m_gBuffer;

    // Screen Pass
    void screenPass(RenderBatch* batch);
    Shader m_screenShader;
    _dur dur_screenPass;

    std::string soStr;
    u8 shaderOutput;

    // SSAO Pass
    void ssaoPass(RenderBatch* batch);
    Shader m_ssaoPassShader;
    _dur dur_ssaoPass;
    Framebuffer_new ssaoFBO;
    const unsigned int Kernel_Size = 16;
    std::vector<vec3> ssaoKernel;
    GLuint noiseTexture;

    Shader m_ssaoBlurShader;
    Framebuffer_new ssaoBlurFBO;

	// Sprite Pass
	Shader m_spriteShader;
	_dur dur_spritePass;
	SpriteRenderer spriteRender;

    // Debug Pass
    Shader m_debugMeshShader;
    _dur dur_debug;
    DynamicFont debugFont;
    TriangleMesh* cameraMesh;

    // Common vars
    Texture blackTex, whiteTex, normalTex, greenTex;
    GLuint fullscreenVAO;

    u16 scr_width, scr_height;
};

#endif
