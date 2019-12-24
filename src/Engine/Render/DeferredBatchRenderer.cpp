#include "DeferredBatchRenderer.hpp"
#include "Scene\Scene.hpp"

DeferredBatchRenderer::DeferredBatchRenderer() :
    shaderOutput(0),
    soStr("Albedo")
{}

DeferredBatchRenderer::~DeferredBatchRenderer() {}

void DeferredBatchRenderer::update(double dt) {
}

void DeferredBatchRenderer::handleMessage(Message msg) {
    if (msg.isType("InputKey")) {
        // int button, int action, int mods
        s32 key = msg.data[0];
        s32 scancode = msg.data[1];
        s32 action = msg.data[2];
        s32 mods = msg.data[3];

        if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS) {
            Console::logMessage("Reloading Shaders");

            m_geometryPassShader.create("pipeline/geometryPass.vert", "pipeline/geometryPass.frag", "geometryPassShader");
            m_screenShader.create("pipeline/screen.vert", "pipeline/screen.frag", "screenShader");

            m_debugMeshShader.create("debugMesh.vert", "debugMesh.frag", "debugMeshShader");
        }

        else if (key == GLFW_KEY_V && action == GLFW_PRESS) {
            shaderOutput++;
            if (shaderOutput > 3)
                shaderOutput = 0;

            switch (shaderOutput) {
            case 0:
                soStr = "Albedo";
                break;
            case 1:
                soStr = "Normal";
                break;
            case 2:
                soStr = "Emission";
                break;
            case 3:
                soStr = "Ambient Occlusion";
                break;
            case 4:
                soStr = "Metallic";
                break;
            case 5:
                soStr = "Roughness";
                break;
            case 6:
                soStr = "Albedo + Ambient Occlusion";
                break;
            case 7:
                soStr = "Albedo + Ambient Occlusion + Emission";
                break;
            }

            Console::logMessage("Toggling shader output: " + std::to_string(shaderOutput) 
                + ": " + soStr);
        }
    }

    if (msg.isType("NewWindowSize")) {
        s32 newX = msg.data[0];
        s32 newY = msg.data[1];

        m_gBuffer.resize(newX, newY);

        debugFont.resize(newX, newY);

        scr_width = newX;
        scr_height = newY;
    }
}

void DeferredBatchRenderer::destroy() {}
CoreSystem* DeferredBatchRenderer::create() {
    m_geometryPassShader.create("pipeline/geometryPass.vert", "pipeline/geometryPass.frag", "geometryPassShader");
    m_screenShader.create("pipeline/screen.vert", "pipeline/screen.frag", "screenShader");

    m_debugMeshShader.create("debugMesh.vert", "debugMesh.frag", "debugMeshShader");

    blackTex.loadImage("black.png");
    whiteTex.loadImage("white.png");
    normalTex.loadImage("normal.png");
    greenTex.loadImage("green.png");

    scr_width = DEFAULT_SCREEN_WIDTH;
    scr_height = DEFAULT_SCREEN_HEIGHT;

    m_gBuffer.create(scr_width, scr_height);

    debugFont.InitTextRendering();
    debugFont.create("UbuntuMono-Regular.ttf", 16, scr_width, scr_height);

    const float fullscreenVerts[] = {
        -1, -1,
        -1,  1,
        1, -1,

        1, -1,
        -1,  1,
        1,  1
    };

    GLuint fullscreenVBO;
    glGenVertexArrays(1, &fullscreenVAO);
    glGenBuffers(1, &fullscreenVBO);
    glBindVertexArray(fullscreenVAO);
    glBindBuffer(GL_ARRAY_BUFFER, fullscreenVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fullscreenVerts),
        &fullscreenVerts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(f32), (void*)0);
    glBindVertexArray(0);

    return this;
}

void DeferredBatchRenderer::renderBatch(RenderBatch* batch) {
    /* Use list of draw calls to render the scene in multiple passes
    1. Build geometry buffer
    */

    beginProfile();

    if (GetScene()) {

        geometryPass(batch);
        dur_geometryPass = profileRenderPass();

        screenPass(batch);
        dur_screenPass = profileRenderPass();
    }

    endProfile();
}

void DeferredBatchRenderer::geometryPass(RenderBatch* batch) {
    // bind geometry buffer
    m_gBuffer.bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_BLEND);
    m_geometryPassShader.use();
    m_geometryPassShader.setMat4("projectionMatrix", 
        batch->cameraProjection);

    m_geometryPassShader.setInt("material.baseColorTexture", 0);
    m_geometryPassShader.setInt("material.normalTexture", 1);
    m_geometryPassShader.setInt("material.metallicRoughnessTexture", 2);
    m_geometryPassShader.setInt("material.occlusionTexture", 3);
    m_geometryPassShader.setInt("material.emissiveTexture", 4);

    for (int n = 0; n < batch->numCalls; n++) {
        DrawCall* call = &batch->calls[n];

        // Render entity
        glBindVertexArray(call->VAO);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, call->mat->baseColorTexture.glTexID == 0 ? whiteTex.glTextureID : call->mat->baseColorTexture.glTexID);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, call->mat->normalTexture.glTexID == 0 ? normalTex.glTextureID : call->mat->normalTexture.glTexID);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, call->mat->metallicRoughnessTexture.glTexID == 0 ? whiteTex.glTextureID : call->mat->metallicRoughnessTexture.glTexID);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, call->mat->occlusionTexture.glTexID == 0 ? whiteTex.glTextureID : call->mat->occlusionTexture.glTexID);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, call->mat->emissiveTexture.glTexID == 0 ? whiteTex.glTextureID : call->mat->emissiveTexture.glTexID);

        m_geometryPassShader.setVec3("material.emissiveFactor", call->mat->emissiveFactor);
        m_geometryPassShader.setVec4("material.baseColorFactor", call->mat->baseColorFactor);
        m_geometryPassShader.setFloat("material.metallicFactor", call->mat->metallicFactor);
        m_geometryPassShader.setFloat("material.roughnessFactor", call->mat->roughnessFactor);

        m_geometryPassShader.setMat4("modelViewMatrix", batch->cameraView * call->modelMatrix);

        glDrawElements(GL_TRIANGLES, call->numVerts, GL_UNSIGNED_SHORT, 0);
    }

    // unbind gBuffer
    m_gBuffer.unbind();
    glEnable(GL_BLEND);
}

void DeferredBatchRenderer::screenPass(RenderBatch* batch) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_screenShader.use();

    m_screenShader.setInt("shaderOutput", shaderOutput);

    m_screenShader.setInt("Target0", 0);
    m_screenShader.setInt("Target1", 1);
    m_screenShader.setInt("Target2", 2);
    m_screenShader.setInt("TargetDepth", 3);

    glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D, m_gBuffer.rt0);
    glActiveTexture(GL_TEXTURE1);glBindTexture(GL_TEXTURE_2D, m_gBuffer.rt1);
    glActiveTexture(GL_TEXTURE2);glBindTexture(GL_TEXTURE_2D, m_gBuffer.rt2);
    glActiveTexture(GL_TEXTURE3);glBindTexture(GL_TEXTURE_2D, m_gBuffer.rtDepth);

    glBindVertexArray(fullscreenVAO);

    glDisable(GL_CULL_FACE);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEnable(GL_CULL_FACE);

    glBindVertexArray(0);
}



void DeferredBatchRenderer::renderDebug(
    RenderBatch* batch,
    double frameCount,
    long long lastFrame,
    bool debugMode) {

    profileStart = _clock::now();

    vec4 white(1, 1, 1, 1);
    char text[128];
    long long scale = 1LL;
    int y = -13;

    sprintf(text, "FPS: %-2.1lf [%lld us]", 1000000.0 / static_cast<double>(frameCount), lastFrame);
    debugFont.drawText(scr_width - 5, 5, white, text, ALIGN_TOP_RIGHT);
    sprintf(text, "FPS Lock: %s",
        g_options.limitFramerate ? (g_options.highFramerate ? "250" : "50") : "NONE");
    debugFont.drawText(scr_width - 5, 23, white, text, ALIGN_TOP_RIGHT);

    sprintf(text, "Render......%-3lld us (%-3lld us avg.)",
        dur_fullRenderPass / scale,
        static_cast<long long>(avgRenderPass.getCurrentAverage()) / scale);
    debugFont.drawText(5, y += 18, white, text, ALIGN_TOP_LEFT);

    if (GetScene() && (debugMode)) {
        sprintf(text, " Geometry Pass.......%-3lld us", dur_geometryPass / scale);
        debugFont.drawText(5, y += 18, white, text, ALIGN_TOP_LEFT);

        sprintf(text, " Screen Pass.........%-3lld us", dur_screenPass / scale);
        debugFont.drawText(5, y += 18, white, text, ALIGN_TOP_LEFT);

        sprintf(text, "Draw Calls: %-3d", batch->numCalls);
        debugFont.drawText(5, scr_height - 5, white, text, ALIGN_BOT_LEFT);

        sprintf(text, "Shader Output: %-3d [%s]", shaderOutput, soStr.c_str());
        debugFont.drawText(5, scr_height - 25, white, text, ALIGN_BOT_LEFT);

        glClear(GL_DEPTH_BUFFER_BIT);
        m_debugMeshShader.use();
        m_debugMeshShader.setMat4("projectionViewMatrix", batch->cameraProjection * batch->cameraView);
        m_debugMeshShader.setMat4("modelMatrix", batch->cameraModelMatrix);
        m_debugMeshShader.setVec3("sun.direction", batch->sun->Direction);
        m_debugMeshShader.setVec3("sun.color", batch->sun->Color);
        m_debugMeshShader.setFloat("sun.strength", batch->sun->Strength);
        m_debugMeshShader.setVec3("objColor", vec3(1, 1, 1));
        m_debugMeshShader.setVec3("camPos", batch->camPos); // use viewPos so camera model has correct specular

        glBindVertexArray(cameraMesh->VAO);
        glDrawElements(GL_TRIANGLES, cameraMesh->numFaces * 3, GL_UNSIGNED_SHORT, 0);
        glBindVertexArray(0);
    }
    sprintf(text, "Debug.......%-3lld us", dur_debug / scale);
    debugFont.drawText(5, y += 18, white, text, ALIGN_TOP_LEFT);

    // Print gamepad state
    if (Input::gamepadPresent) {
        int numAxes;
        const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &numAxes);

        for (int jj = 0; jj < numAxes; jj++) {
            sprintf(text, "Gamepad axis %d: %.3f", jj, axes[jj]);
            debugFont.drawText(5, 100 + jj * 20, white, text);
        }

        int numButtons;
        const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &numButtons);

        for (int jj = 0; jj < numButtons; jj++) {
            sprintf(text, "Gamepad button %d: %s", jj, buttons[jj] == GLFW_PRESS ? "Pressed" : "");
            debugFont.drawText(5, 100 + numAxes * 20 + jj * 20, white, text);
        }
    }

    dur_debug = profileRenderPass();
}






/* Render Profiling */
void DeferredBatchRenderer::beginProfile() {
    dur_fullRenderPass = 0;
    profileStart = _clock::now();
}
DeferredBatchRenderer::_dur DeferredBatchRenderer::profileRenderPass() {
    _dur duration =
        std::chrono::duration_cast<std::chrono::microseconds>(
            _clock::now() - profileStart).count();

    dur_fullRenderPass += duration;

    profileStart = _clock::now();
    return duration;
}
void DeferredBatchRenderer::endProfile() {
    avgRenderPass.addSample(dur_fullRenderPass);
}

void DeferredBatchRenderer::loadResources(ResourceManager* resource) {
    resource->loadModelFromFile("Data/Models/camera.glb", true);

    cameraMesh = resource->getMesh("Camera");
}