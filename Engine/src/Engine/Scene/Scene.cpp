#include "Scene.h"

#include "Engine/Memory/Memory.h"
#include "Engine/Memory/Memory_Arena.h"
#include "Engine/Core/String.h"
#include "Engine/Core/Logger.h"

void create_scene(scene_3D* scene, const char* name, memory_arena* arena) {
    scene->name = copy_string_to_arena(name, arena);
    scene->arena = arena;

    // define default lights
    scene->sun.direction = laml::Vec3(0.0f, -1.0f, 0.0f);
    scene->sun.color = laml::rgb8_to_rgba32f(201, 226, 255);
    scene->sun.strength = 1.0f;

    scene->sun.enabled = true;
    scene->sun.cast_shadow = false;
    scene->sun.shadowmap_projection_size = 5.0f;
    scene->sun.shadowmap_projection_depth = 20.0f;
    scene->sun.origin_point = laml::Vec3(0.0f);
    scene->sun.dist_from_origin = 10.0f;

    scene->pointlights = CreateArray(arena, scene_point_light, 10);
    scene->spotlights  = CreateArray(arena, scene_spot_light,  10);

    scene->sky.strength = 1.0f;
    scene->sky.draw_skybox = true;

    // reserve dynarrays for entity types
    scene->static_entities  = CreateArray(arena, entity_static,  10);
    scene->skinned_entities = CreateArray(arena, entity_skinned, 10);
}


entity_static* create_static_entity(scene_3D* scene, const char* name, resource_static_mesh* mesh) {
    entity_static entity;
    entity.scene = scene;
    entity.name = copy_string_to_arena(name, scene->arena);
    entity.static_mesh = mesh;

    entity.position    = laml::Vec3(0.0f, 0.0f, 0.0f);
    entity.orientation = laml::Quat(0.0f, 0.0f, 0.0f, 1.0f);
    entity.scale       = laml::Vec3(1.0f, 1.0f, 1.0f);

    entity.color = laml::Vec3(-1.0f); // tmp

    scene->static_entities = (entity_static*)ArrayPushPtr(scene->static_entities, &entity, sizeof(entity_static));
    entity_static* new_entity_ptr = ArrayPeek(scene->static_entities);

    return new_entity_ptr;
}

entity_skinned* create_skinned_entity(scene_3D* scene, const char* name, 
                                      resource_skinned_mesh* mesh,
                                      animation_controller* controller) {
    entity_skinned entity;
    entity.scene = scene;
    entity.name = copy_string_to_arena(name, scene->arena);
    entity.skinned_mesh = mesh;
    entity.controller = controller;
    
    entity.position    = laml::Vec3(0.0f, 0.0f, 0.0f);
    entity.orientation = laml::Quat(0.0f, 0.0f, 0.0f, 1.0f);
    entity.scale       = laml::Vec3(1.0f, 1.0f, 1.0f);

    scene->skinned_entities = (entity_skinned*)ArrayPushPtr(scene->skinned_entities, &entity, sizeof(entity_skinned));
    entity_skinned* new_entity_ptr = ArrayPeek(scene->skinned_entities);

    return new_entity_ptr;
}

// TODO: STD library
#include <stdio.h>
bool32 serialize_scene(const char* filename, const scene_3D* scene) {
    FILE* fid = fopen(filename, "w");
    if (fid == NULL) {
        RH_ERROR("Could not open file for writing: '%s'", filename);
        return false;
    }

    /*
        char* name;

        entity_static*  static_entities;  // dynarray
        entity_skinned* skinned_entities; // dynarray

        scene_dir_light sun;
        scene_sky_light sky;
        scene_point_light* pointlights; // dynarray
        scene_spot_light*  spotlights;  // dynarray
    */

    uint32 num_static_entities  = (uint32)GetArrayCount(scene->static_entities);
    uint32 num_skinned_entities = (uint32)GetArrayCount(scene->skinned_entities);
    
    uint32 num_point_lights = (uint32)GetArrayCount(scene->pointlights);
    uint32 num_spot_lights = (uint32)GetArrayCount(scene->spotlights);

    fprintf(fid, "scene: \n");
    fprintf(fid, "    name: %s\n", scene->name);
    fprintf(fid, "\n");

    // static_entities array
    fprintf(fid, "    static_entities: []\n");
    for (uint32 n = 0; n < num_static_entities; n++) {
        entity_static* ent = &scene->static_entities[n];

        fprintf(fid, "    -:\n");
        fprintf(fid, "        id: %d\n", n);
        fprintf(fid, "        name: %s\n", ent->name);
        fprintf(fid, "        mesh: %s\n", "mesh_name");
        fprintf(fid, "        position: vec3(%f,%f,%f)\n", ent->position.x, ent->position.y, ent->position.z);
        fprintf(fid, "        orientation: quat(%f,%f,%f,%f)\n", ent->orientation.x, ent->orientation.y, ent->orientation.z, ent->orientation.w);
        fprintf(fid, "        scale: vec3(%f,%f,%f)\n",          ent->scale.x, ent->scale.y, ent->scale.z);
        fprintf(fid, "\n");
    }

    // skinned entities array
    fprintf(fid, "    skinned_entities: []\n");
    for (uint32 n = 0; n < num_skinned_entities; n++) {
        entity_skinned* ent = &scene->skinned_entities[n];

        fprintf(fid, "    -:\n");
        fprintf(fid, "        id: %d\n", n);
        fprintf(fid, "        name: %s\n", ent->name);
        fprintf(fid, "        mesh: %s\n", "mesh_name");
        fprintf(fid, "        controller: %s\n", "controller_name");
        fprintf(fid, "        position: vec3(%f,%f,%f)\n",       ent->position.x,    ent->position.y,    ent->position.z);
        fprintf(fid, "        orientation: quat(%f,%f,%f,%f)\n", ent->orientation.x, ent->orientation.y, ent->orientation.z, ent->orientation.w);
        fprintf(fid, "        scale: vec3(%f,%f,%f)\n",          ent->scale.x,       ent->scale.y,       ent->scale.z);
        fprintf(fid, "\n");
    }

    // point lights array
    fprintf(fid, "    pointlights: []\n");
    for (uint32 n = 0; n < num_point_lights; n++) {
        scene_point_light* ent = &scene->pointlights[n];

        fprintf(fid, "    -:\n");
        fprintf(fid, "        id: %d\n", n);
        fprintf(fid, "        position: vec3(%f,%f,%f)\n", ent->position.x, ent->position.y, ent->position.z);
        fprintf(fid, "        color: vec3(%f,%f,%f)\n",    ent->color.x,    ent->color.y,    ent->color.z);
        fprintf(fid, "        strength: %f\n",    ent->strength);
        fprintf(fid, "        cast_shadow: %s\n", ent->cast_shadow ? "Yes" : "No");
        fprintf(fid, "\n");
    }

    // spot lights array
    fprintf(fid, "    spotlights: []\n");
    for (uint32 n = 0; n < num_spot_lights; n++) {
        scene_spot_light* ent = &scene->spotlights[n];

        fprintf(fid, "    -:\n");
        fprintf(fid, "        id: %d\n", n);
        fprintf(fid, "        position: vec3(%f,%f,%f)\n",  ent->position.x,  ent->position.y,  ent->position.z);
        fprintf(fid, "        direction: vec3(%f,%f,%f)\n", ent->direction.x, ent->direction.y, ent->direction.z);
        fprintf(fid, "        color: vec3(%f,%f,%f)\n",     ent->color.x,     ent->color.y,     ent->color.z);
        fprintf(fid, "        strength: %f\n",    ent->strength);
        fprintf(fid, "        inner: %f # deg\n", ent->inner);
        fprintf(fid, "        outer: %f # deg\n", ent->outer);
        fprintf(fid, "        cast_shadow: %s\n", ent->cast_shadow ? "Yes" : "No");
        fprintf(fid, "\n");
    }

    fclose(fid);
    return true;
}