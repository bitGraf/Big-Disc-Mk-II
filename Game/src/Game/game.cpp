#include "Game.h"

#include <Engine/Core/Logger.h>
#include <Engine/Core/Timing.h>
#include <Engine/Core/Input.h>
#include <Engine/Core/Event.h>
#include <Engine/Core/String.h>
#include <Engine/Memory/Memory.h>
#include <Engine/Memory/Sorting.h>
#include <Engine/Memory/Memory_Arena.h>
#include <Engine/Renderer/Renderer.h>
#include <Engine/Renderer/Render_Types.h>
#include <Engine/Resources/Resource_Manager.h>
#include <Engine/Resources/Filetype/anim_file_reader.h>
#include <Engine/Collision/Collision.h>
#include <Engine/Collision/Character_Controller.h>
#include <Engine/Animation/Animation.h>
#include <Engine/Scene/Scene.h>

#include <Engine/Debug_UI/Debug_UI.h>

#include <Engine/Platform/Platform.h>
#include <Engine/Resources/Filetype/dds_file_reader.h>

struct game_state {
    memory_arena arena;

    // basic scene
    scene_3D scene;

    bool32 debug_mode;
    real32 sun_yp[2];
};

bool32 controller_key_press(uint16 code, void* sender, void* listener, event_context context);
bool32 controller_key_release(uint16 code, void* sender, void* listener, event_context context);

void init_game(game_state* state, game_memory* memory) {
    ////////////////////////////////////////
    RH_WARN("entity_static: %llu bytes", sizeof(entity_static));
    ///////////////////////////////////////

    //
    char full_path[256];
    platform_get_full_resource_path(full_path, 256, "Data/textures/metal.dds");

    RH_TRACE("Full filename: [%s]", full_path);

    file_handle file = platform_read_entire_file(full_path);
    if (!file.num_bytes) {
        RH_ERROR("Failed to read resource file");
        return;
    }

    memory_arena* arena = resource_get_arena();

    memory_index offset = sizeof(game_state);
    CreateArena(&state->arena, memory->GameStorageSize-offset, (uint8*)(memory->GameStorage)+offset);

    state->debug_mode = false;

    create_scene(&state->scene, "basic_scene", &state->arena);

    // define basic scene
    state->scene.sun.direction = laml::normalize(laml::Vec3(1.5f, -1.0f, -1.0f));
    state->scene.sun.enabled = true;
    state->scene.sun.cast_shadow = true;
    state->scene.sun.strength = 20.0f;
    state->scene.sun.shadowmap_projection_size = 15.0f;
    state->scene.sun.shadowmap_projection_depth = 25.0f;
    state->scene.sun.dist_from_origin = 12.5f;

    laml::Mat4 rot(1.0f);
    laml::transform::lookAt(rot, laml::Vec3(0.0f), state->scene.sun.direction, laml::Vec3(0.0f, 1.0f, 0.0f));
    //laml::transform::create_transform_rotation(rot, state->debug_camera.orientation);
    real32 roll;
    laml::transform::decompose(rot, state->sun_yp[0], state->sun_yp[0], roll);

    //resource_load_env_map("Data/env_maps/newport_loft.hdr", &state->scene.sky.environment);
    state->scene.sky.draw_skybox = false;

    resource_static_mesh* mesh = PushStruct(&state->arena, resource_static_mesh);
    resource_load_static_mesh("Data/Models/plane.mesh", mesh);
    mesh->materials[0].RoughnessFactor = 0.95f; // todo: pull materials out of mesh file? make it a separate thing entirely?
    mesh->materials[0].MetallicFactor = 0.0f;
    mesh->materials[0].DiffuseFactor = laml::Vec3(0.4f, 1.0f, 0.4f);
    entity_static* floor = create_static_entity(&state->scene, "floor", mesh);
    floor->color = laml::Vec3(1.0f);

	resource_texture_2D dds_tex;
	if (!resource_load_texture_file("Data/textures/metal.dds", &dds_tex)) {
		RH_ERROR("Failed to load dds.");
	}

    resource_static_mesh* mesh2 = PushStruct(&state->arena, resource_static_mesh);
    resource_load_static_mesh("Data/Models/box.mesh", mesh2);
    mesh2->materials[0].RoughnessFactor = 0.95f; // todo: pull materials out of mesh file? make it a separate thing entirely?
    mesh2->materials[0].MetallicFactor = 0.0f;
    //mesh2->materials[0].DiffuseFactor = laml::Vec3(1.0f, 0.0f, 0.0f);
    entity_static* box = create_static_entity(&state->scene, "box", mesh2);
    box->color = laml::Vec3(1.0f);
    box->position = laml::Vec3(0.0f, 2.0f, 0.0f);

    RH_INFO("Scene created. %d Static entities. %d Skinned entities.",
            GetArrayCount(state->scene.static_entities),
            GetArrayCount(state->scene.skinned_entities));

    RH_INFO("Game initialized");
}

GAME_API GAME_UPDATE_FUNC(GameUpdate) {
    game_state* state = (game_state*)memory->GameStorage;
    if (!memory->IsInitialized) {
        init_game(state, memory);

        memory->IsInitialized = true;

        RH_INFO("------ Scene Initialized -----------------------");
    }

    #if defined(RH_IMGUI)
    // Debug window
    char label_name[64];
    ImGui::Begin("Scene");
    ImGui::SeparatorText("Lighting");
    
    if (ImGui::TreeNode("Skybox")) {
        ImGui::DragFloat("Contribution", &state->scene.sky.strength, 0.01f, 0.0f, 1.0f);
        ImGui::Checkbox("Draw Skybox", &state->scene.sky.draw_skybox);
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Sun")) {
        ImGui::Checkbox("Enabled", &state->scene.sun.enabled);
        ImGui::SameLine();
        ImGui::Checkbox("Cast Shadows", &state->scene.sun.cast_shadow);
    //if (ImGui::CollapsingHeader("Sun")) {
        ImGui::DragFloat("Strength", &state->scene.sun.strength, 0.1f, 0.0f, 25.0f);
        ImGui::DragFloat2("Direction", state->sun_yp, 0.5f, -180.0f, 180.0f);
        ImGui::DragFloat("OrthoWidth", &state->scene.sun.shadowmap_projection_size, 0.1f, 1.0f, 50.0f);
        ImGui::DragFloat("OrthoDepth", &state->scene.sun.shadowmap_projection_depth, 0.1f, 1.0f, 100.0f);
        ImGui::DragFloat("DistFromOrigin", &state->scene.sun.dist_from_origin, 0.1f, 1.0f, 100.0f);
        ImGui::DragFloat3("Origin", state->scene.sun.origin_point._data, 0.1f, -10.0f, 10.0f);
        state->scene.sun.direction = laml::transform::dir_from_yp(state->sun_yp[0], state->sun_yp[1]);
        ImGui::ColorPicker3("Color", state->scene.sun.color._data);
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Point Lights")) {
        uint32 num_point_lights = (uint32)GetArrayCount(state->scene.pointlights);
        ImGui::Text("%u pointlights", num_point_lights);
        ImGui::SameLine();
        if (ImGui::Button("Add PointLight")) {
            ArrayAdd(state->scene.pointlights);
            scene_point_light* newlight = ArrayPeek(state->scene.pointlights);

            newlight->position = laml::Vec3(0.0f, 1.0f, 0.0f);
            newlight->color = laml::Vec3(1.0f, 1.0f, 1.0f);
            newlight->strength = 25.0f;
        }
        for (uint64 n = 0; n < num_point_lights; n++) {
            scene_point_light* pl = &state->scene.pointlights[n];
            string_build(label_name, 64, "pointlight #%u", n+1);
            if (ImGui::TreeNode(label_name)) {
                ImGui::Checkbox("Enabled", &pl->enabled);
                ImGui::DragFloat3("Position", pl->position._data, 0.01f, -FLT_MAX, FLT_MAX);
                ImGui::DragFloat3("Color",    pl->color._data,    0.01f,  0.0f, 1.0f);
                ImGui::DragFloat("Strength", &pl->strength,       0.5f,   0.0f, FLT_MAX);

                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Spot Lights")) {
        uint32 num_spot_lights = (uint32)GetArrayCount(state->scene.spotlights);
        ImGui::Text("%u spotlight", num_spot_lights);
        ImGui::SameLine();
        if (ImGui::Button("Add SpotLight")) {
            ArrayAdd(state->scene.spotlights);
            scene_spot_light* newlight = ArrayPeek(state->scene.spotlights);
            num_spot_lights = (uint32)GetArrayCount(state->scene.spotlights);

            newlight->position = laml::Vec3(0.0f, 1.0f, 0.0f);
            newlight->direction = laml::Vec3(0.0f, -1.0f, 0.0f);
            newlight->color = laml::Vec3(1.0f, 1.0f, 1.0f);
            newlight->strength = 25.0f;
            newlight->inner = 55.0f;
            newlight->outer = 65.0f;
        }
        for (uint64 n = 0; n < num_spot_lights; n++) {
            scene_spot_light* sl = &state->scene.spotlights[n];
            string_build(label_name, 64, "spotlight #%u", n+1);
            if (ImGui::TreeNode(label_name)) {
                ImGui::Checkbox("Enabled", &sl->enabled);
                ImGui::DragFloat3("Position",  sl->position._data,  0.01f, -FLT_MAX, FLT_MAX);
                ImGui::DragFloat3("Direction", sl->direction._data, 0.01f, -FLT_MAX, FLT_MAX);
                ImGui::DragFloat3("Color",     sl->color._data,     0.01f,  0.0f, 1.0f);
                ImGui::DragFloat("Strength",  &sl->strength,        0.5f,   0.0f, FLT_MAX);
                ImGui::DragFloat("Inner",     &sl->inner,           0.5f,   0.0f, sl->outer-0.5f);
                ImGui::DragFloat("Outer",     &sl->outer,           0.5f,   0.0f, 90.0f);
                if (sl->outer < sl->inner)
                    sl->inner = sl->outer;

                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }

    ImGui::SeparatorText("Static Entities");
    uint64 num_entities = GetArrayCount(state->scene.static_entities);
    ImGui::Text("%u static_entities", num_entities);
    for (uint64 n = 0; n < num_entities; n++) {
        entity_static* ent = &state->scene.static_entities[n];
        string_build(label_name, 64, "static_entity #%u [%s]", n, ent->name);
        if (ImGui::TreeNode(label_name)) {
            ImGui::DragFloat3("Position", ent->position._data,    0.01f, -FLT_MAX, FLT_MAX);
            ImGui::DragFloat3("Rotation", ent->euler_ypr, 0.5f, -180.0f, 180.0f);
            ent->orientation = laml::transform::quat_from_ypr(ent->euler_ypr[0], ent->euler_ypr[1], ent->euler_ypr[2]);
            ImGui::DragFloat3("Scale",    ent->scale._data,       0.01f, 0.01f, FLT_MAX);

            ImGui::TreePop();
        }
    }

    ImGui::SeparatorText("Skinned Entities");
    num_entities = GetArrayCount(state->scene.skinned_entities);
    ImGui::Text("%u skinned_entities", num_entities);
    for (uint64 n = 0; n < num_entities; n++) {
        entity_skinned* ent = &state->scene.skinned_entities[n];
        string_build(label_name, 64, "skinned_entity #%u [%s]", n, ent->name);
        if (ImGui::TreeNode(label_name)) {
            ImGui::DragFloat3("Position", ent->position._data,    0.01f, -FLT_MAX, FLT_MAX);
            ImGui::DragFloat3("Rotation", ent->euler_ypr, 0.5f, -180.0f, 180.0f);
            ent->orientation = laml::transform::quat_from_ypr(ent->euler_ypr[0], ent->euler_ypr[1], ent->euler_ypr[2]);
            ImGui::DragFloat3("Scale",    ent->scale._data,       0.01f, 0.01f, FLT_MAX);
            ImGui::Separator();
            ImGui::Text("Node: '%s'",       ent->controller->graph.nodes[ent->controller->current_node].name);
            ImGui::Text("Aimation: '%s'",   ent->controller->graph.nodes[ent->controller->current_node].anim->name);
            ImGui::Text("node_time = %.3f", ent->controller->node_time);
            ImGui::Text("anim_time = %.3f", ent->controller->anim_time);

            ImGui::TreePop();
        }
    }

    ImGui::End();
    #endif

    return &state->scene;
}

GAME_API GAME_KEY_EVENT_FUNC(GameKeyEvent) {
    game_state* state = (game_state*)memory->GameStorage;

    if (key_code == KEY_F1) {
        state->debug_mode = !state->debug_mode;
    }
}


bool32 controller_key_press(uint16 code, void* sender, void* listener, event_context context) {
    animation_controller* controller = (animation_controller*)listener;
    uint16 key_code = context.u16[0];

    controller_on_key_event(controller, key_code, true);

    return false;
}
bool32 controller_key_release(uint16 code, void* sender, void* listener, event_context context) {
    animation_controller* controller = (animation_controller*)listener;
    uint16 key_code = context.u16[0];

    controller_on_key_event(controller, key_code, false);

    return false;
}