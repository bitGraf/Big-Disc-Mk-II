#include "RenderManager.hpp"

BatchRenderer::BatchRenderer() {

}

BatchRenderer::~BatchRenderer() {}

void BatchRenderer::update(double dt) {}
void BatchRenderer::handleMessage(Message msg) {
    if (msg.isType("InputKey")) {
        // int button, int action, int mods
        using dt = Message::Datatype;
        dt key = msg.data[0];
        dt scancode = msg.data[1];
        dt action = msg.data[2];
        dt mods = msg.data[3];

        if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS) {
            Console::logMessage("Reloading Shaders");

            m_shadowPass.create("shadow.vert", "shadow.frag", "shadowPassShader");
            m_staticPass.create("static_pbr_withShadow.vert", "static_pbr_withShadow.frag", "staticPassShader");
            m_skyboxPass.create("skybox.vert", "skybox.frag", "skyboxPassShader");
            m_lightVolumePass.create("lightVolume.vert", "lightVolume.frag", "lightVolumeShader");
            m_toneMap.create("toneMap.vert", "toneMap.frag", "toneMapShader");
            m_gammaCorrect.create("toneMap.vert", "gammaCorrect.frag", "gammaCorrectShader");
            m_debugLineShader.create("DebugLine.vert", "DebugLine.frag", "debugLineShader");
        }
    }
}
void BatchRenderer::destroy() {}
CoreSystem* BatchRenderer::create() {
    m_shadowPass.create("shadow.vert", "shadow.frag", "shadowPassShader");
    m_staticPass.create("static_pbr_withShadow.vert", "static_pbr_withShadow.frag", "staticPassShader");
    m_skyboxPass.create("skybox.vert", "skybox.frag", "skyboxPassShader");
    m_lightVolumePass.create("lightVolume.vert", "lightVolume.frag", "lightVolumeShader");
    m_toneMap.create("toneMap.vert", "toneMap.frag", "toneMapShader");
    m_gammaCorrect.create("toneMap.vert", "gammaCorrect.frag", "gammaCorrectShader");
    m_debugLineShader.create("DebugLine.vert", "DebugLine.frag", "debugLineShader");
    m_debugMeshShader.create("DebugMesh.vert", "DebugMesh.frag", "debugMeshShader");

    blackTex.loadImage("black.png");
    whiteTex.loadImage("white.png");
    normalTex.loadImage("normal.png");
    greenTex.loadImage("green.png");

    Shadowmap::initShadows();
    sm.create(2048, 2048); // resolution of shadow map

    fb.create(800, 600);
    fb_volume.create(800, 600);
    fb_toneMap.create(800, 600);

    debugFont.InitTextRendering();
    debugFont.create("UbuntuMono-Regular.ttf", 16, 800, 600);

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


    const f32 debugLineVerts[] = {
        0, 
        1
    };

    GLuint debugLineVBO;
    glGenVertexArrays(1, &debugLineVAO);
    glGenBuffers(1, &debugLineVBO);
    glBindVertexArray(debugLineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, debugLineVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(debugLineVerts),
        &debugLineVerts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(f32), (void*)0);

    glBindVertexArray(0);

    return this;
}

void BatchRenderer::renderBatch(BatchDrawCall* batch) {
    /* Use list of draw calls to render the scene in multiple passes
    1. Shadow Pass
    2. Static Entity Pass
    3. Dynamic Entity Pass
    4. Skybox Pass
    5. Volumetric Light Pass
    6. HDR Tone Map
    7. Gamma Correction
    */

    beginProfile();

    shadowPass(batch);
    dur_shadowPass = profileRenderPass();

    fb.bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //static const float black[] = { 0, 0, 0, 1 };
    //static const float red[] = { -100, 0, -100, 1 };
    //glClearBufferfv(GL_COLOR, 0, black);
    //glClearBufferfv(GL_COLOR, 1, red);

    staticPass(batch);
    dur_staticPass = profileRenderPass();

    dynamicPass(batch);
    dur_dynamicPass = profileRenderPass();

    skyboxPass(batch);
    dur_skyboxPass = profileRenderPass();

    fb.unbind();
    fb_volume.bind();
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    lightVolumePass(batch);
    dur_lightVolumePass = profileRenderPass();

    fb_volume.unbind();
    fb_toneMap.bind();
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    toneMap(batch);
    dur_toneMap = profileRenderPass();

    fb_toneMap.unbind();

    gammaCorrect(batch);
    dur_gammaCorrect = profileRenderPass();

    endProfile();
}

void BatchRenderer::shadowPass(BatchDrawCall* batch) {
    // First, render to depth map
    m_shadowPass.use();
    glViewport(0, 0, sm.width, sm.height);
    glBindFramebuffer(GL_FRAMEBUFFER, sm.depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    mat4 identity;
    m_shadowPass.setMat4("viewMatrix", batch->sunViewProjectionMatrix);
    m_shadowPass.setMat4("projectionMatrix", identity);
    for (int n = 0; n < batch->numCalls; n++) {
        DrawCall* draw = &batch->calls[n];

        glBindVertexArray(draw->VAO);
        m_shadowPass.setMat4("modelMatrix", draw->modelMatrix);
        glDrawElements(GL_TRIANGLES, draw->numVerts, GL_UNSIGNED_SHORT, 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, 800, 600);
}

void BatchRenderer::staticPass(BatchDrawCall* batch) {
    glEnable(GL_DEPTH_TEST);

    m_staticPass.use();
    m_staticPass.setLights(
        batch->sun,
        batch->pointLights,
        batch->spotLights
    );

    mat4 identity;
    m_staticPass.setMat4("viewMatrix", batch->cameraViewProjectionMatrix);
    m_staticPass.setMat4("projectionMatrix", identity);
    m_staticPass.setVec3("camPos", batch->camPos);

    m_staticPass.setInt("material.baseColorTexture", 0);
    m_staticPass.setInt("material.normalTexture", 1);
    m_staticPass.setInt("material.metallicRoughnessTexture", 2);
    m_staticPass.setInt("material.occlusionTexture", 3);
    m_staticPass.setInt("material.emissiveTexture", 4);
    m_staticPass.setInt("irradianceMap", 5);
    m_staticPass.setInt("prefilterMap", 6);
    m_staticPass.setInt("brdfLUT", 7);
    m_staticPass.setInt("shadowMap", 8);

    m_staticPass.setMat4("lightViewMatrix", batch->sunViewProjectionMatrix);
    m_staticPass.setMat4("lightProjectionMatrix", identity);

    batch->env->bindPBR(GL_TEXTURE5, GL_TEXTURE6, GL_TEXTURE7);
    sm.bind(GL_TEXTURE8);

    /* Render Entities */
    for (int n = 0; n < batch->numCalls; n++) {
        DrawCall* call = &batch->calls[n];
        // Render entity ent
        glBindVertexArray(call->VAO);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, call->mat->baseColorTexture.glTexID == 0 ? blackTex.glTextureID : call->mat->baseColorTexture.glTexID);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, call->mat->normalTexture.glTexID == 0 ? normalTex.glTextureID : call->mat->normalTexture.glTexID);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, call->mat->metallicRoughnessTexture.glTexID == 0 ? greenTex.glTextureID : call->mat->metallicRoughnessTexture.glTexID);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, call->mat->occlusionTexture.glTexID == 0 ? greenTex.glTextureID : call->mat->occlusionTexture.glTexID);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, call->mat->emissiveTexture.glTexID == 0 ? blackTex.glTextureID : call->mat->emissiveTexture.glTexID);

        m_staticPass.setVec3("material.emissiveFactor", call->mat->emissiveFactor);
        m_staticPass.setVec4("material.baseColorFactor", call->mat->baseColorFactor);
        m_staticPass.setFloat("material.metallicFactor", call->mat->metallicFactor);
        m_staticPass.setFloat("material.roughnessFactor", call->mat->roughnessFactor);

        m_staticPass.setMat4("modelMatrix", call->modelMatrix);
        
        glDrawElements(GL_TRIANGLES, call->numVerts, GL_UNSIGNED_SHORT, 0);
    }
}

void BatchRenderer::dynamicPass(BatchDrawCall* batch) {
}

void BatchRenderer::skyboxPass(BatchDrawCall* batch) {
    m_skyboxPass.use();
    mat4 identity;
    m_skyboxPass.setMat4("viewMatrix", mat4(mat3(batch->cameraView)));
    m_skyboxPass.setMat4("projectionMatrix", batch->cameraProjection);

    m_skyboxPass.setInt("skybox", 0);
    //skybox->bind(GL_TEXTURE0);
    batch->env->bindSkybox(GL_TEXTURE0);

    glDepthFunc(GL_LEQUAL);
    glBindVertexArray(batch->env->skyboxVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthFunc(GL_LESS);
}

void BatchRenderer::lightVolumePass(BatchDrawCall* batch) {
    m_lightVolumePass.use();
    m_lightVolumePass.setInt("tex", 0);
    m_lightVolumePass.setInt("positionMap", 1);
    m_lightVolumePass.setInt("shadowMap", 2);
    m_lightVolumePass.setVec3("camPos", batch->camPos);
    m_lightVolumePass.setVec3("sun.direction", batch->sun->direction);
    m_lightVolumePass.setVec4("sun.color", batch->sun->color);
    m_lightVolumePass.setFloat("sun.strength", batch->sun->strength);
    m_lightVolumePass.setMat4("lightViewMatrix", batch->sunViewProjectionMatrix);
    mat4 identity;
    m_lightVolumePass.setMat4("lightProjectionMatrix", identity);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fb.getTexture());
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, fb.getPositionTex());
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, sm.depthMap);

    //glBindTexture(GL_TEXTURE_2D, sm.depthMap);

    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(fullscreenVAO);
    glDisable(GL_CULL_FACE);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}

void BatchRenderer::toneMap(BatchDrawCall* batch) {
    // read from FBO, render to screen buffer
    // perform hdr correction
    m_toneMap.use();
    m_toneMap.setInt("tex", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fb_volume.getTexture());

    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(fullscreenVAO);
    glDisable(GL_CULL_FACE);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}

void BatchRenderer::gammaCorrect(BatchDrawCall* batch) {
    // read from FBO, render to screen buffer
    // perform hdr correction
    m_gammaCorrect.use();
    m_gammaCorrect.setInt("tex", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fb_toneMap.getTexture());

    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(fullscreenVAO);
    glDisable(GL_CULL_FACE);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}

void BatchRenderer::renderDebug(
    BatchDrawCall* batch, 
    double frameCount, 
    long long lastFrame,
    bool debugMode) {

    vec4 white(1, 1, 1, 1);
    char text[128];
    sprintf(text, "FPS: %-2.1lf [%lld us]", 1000000.0/static_cast<double>(frameCount), lastFrame);
    debugFont.drawText(800 - 5, 5, white, text, ALIGN_TOP_RIGHT);
    sprintf(text, "FPS Lock: %s", 
        g_options.limitFramerate ? (g_options.highFramerate ? "250" : "50") : "NONE");
    debugFont.drawText(800 - 5, 23, white, text, ALIGN_TOP_RIGHT);

    long long scale = 1LL;
    int y = -13;
    sprintf(text, "Render - - - %-3lld us (%-3lld us avg.)", 
        dur_fullRenderPass/ scale,
        static_cast<long long>(avgRenderPass.getCurrentAverage()) / scale);
    debugFont.drawText(5, y+=18, white, text, ALIGN_TOP_LEFT);
    sprintf(text, " Shadow- - - %-3lld us", dur_shadowPass / scale);
    debugFont.drawText(5, y += 18, white, text, ALIGN_TOP_LEFT);
    sprintf(text, " Static- - - %-3lld us", dur_staticPass / scale);
    debugFont.drawText(5, y += 18, white, text, ALIGN_TOP_LEFT);
    sprintf(text, " Dynamic - - %-3lld us", dur_dynamicPass / scale);
    debugFont.drawText(5, y += 18, white, text, ALIGN_TOP_LEFT);
    sprintf(text, " Skybox- - - %-3lld us", dur_skyboxPass / scale);
    debugFont.drawText(5, y += 18, white, text, ALIGN_TOP_LEFT);
    sprintf(text, " Volume- - - %-3lld us", dur_lightVolumePass / scale);
    debugFont.drawText(5, y += 18, white, text, ALIGN_TOP_LEFT);
    sprintf(text, " ToneMap - - %-3lld us", dur_toneMap / scale);
    debugFont.drawText(5, y += 18, white, text, ALIGN_TOP_LEFT);
    sprintf(text, " Gamma - - - %-3lld us", dur_gammaCorrect / scale);
    debugFont.drawText(5, y += 18, white, text, ALIGN_TOP_LEFT);

    sprintf(text, "Draw Calls: %-3d", batch->numCalls);
    debugFont.drawText(5, 595, white, text, ALIGN_BOT_LEFT);


    if (debugMode) {
        glDisable(GL_DEPTH_TEST);
        m_debugMeshShader.use();
        m_debugMeshShader.setMat4("projectionViewMatrix", batch->cameraViewProjectionMatrix);
        m_debugMeshShader.setMat4("modelMatrix", batch->cameraModelMatrix);
        m_debugMeshShader.setVec3("sun.direction", batch->sun->direction);
        m_debugMeshShader.setVec4("sun.color", batch->sun->color);
        m_debugMeshShader.setFloat("sun.strength", batch->sun->strength);
        m_debugMeshShader.setVec3("objColor", vec3(1,1,1));
        m_debugMeshShader.setVec3("camPos", batch->camPos);

        glBindVertexArray(cameraMesh->VAO);
        glDrawElements(GL_TRIANGLES, cameraMesh->numFaces * 3, GL_UNSIGNED_SHORT, 0);
        glBindVertexArray(0);


        // Draw wireframe things
        m_debugLineShader.use();
        m_debugLineShader.setMat4("projectionViewMatrix", batch->cameraViewProjectionMatrix);

        vec3 camPos = batch->camPos;
        vec3 forward = vec3(batch->cameraView.row1());
        vec3 up = vec3(batch->cameraView.row2());
        vec3 right = vec3(batch->cameraView.row3());

        drawLine(camPos, camPos + forward, vec3(1, 0, 0), vec3(1, 0, 0));
        drawLine(camPos, camPos + up, vec3(0, 1, 0), vec3(0, 1, 0));
        drawLine(camPos, camPos + right, vec3(0, 0, 1), vec3(0, 0, 1));
        glBindVertexArray(0);

        glEnable(GL_DEPTH_TEST);
    }
}

void BatchRenderer::drawLine(vec3 A, vec3 B, vec3 colorA, vec3 colorB) {
    m_debugLineShader.setVec3("vertexA", A);
    m_debugLineShader.setVec3("vertexB", B);
    m_debugLineShader.setVec3("colorA", colorA);
    m_debugLineShader.setVec3("colorB", colorB);

    glBindVertexArray(debugLineVAO);
    glDrawArrays(GL_LINES, 0, 2);

    /*
    uniform mat4 viewMatrix;
    uniform mat4 projectionMatrix;

    uniform vec3 vertexA;
    uniform vec3 vertexB;
    uniform vec3 colorA;
    uniform vec3 colorB;
    */
}

void BatchRenderer::loadResources(ResourceManager* resource) {
    resource->loadModelFromFile("Data/Models/camera.glb", true);

    cameraMesh = resource->getMesh("Camera");
}







/* Render Profiling */
void BatchRenderer::beginProfile() {
    dur_fullRenderPass = 0;
    profileStart = _clock::now();
}
BatchRenderer::_dur BatchRenderer::profileRenderPass() {
    _dur duration =
        std::chrono::duration_cast<std::chrono::microseconds>(
            _clock::now() - profileStart).count();

    dur_fullRenderPass += duration;

    profileStart = _clock::now();
    return duration;
}
void BatchRenderer::endProfile() {
    avgRenderPass.addSample(dur_fullRenderPass);
}