#include <enpch.hpp>

#include "ExampleLevels.hpp"
#include "Engine/Scene/Scene3D.hpp"

#include "Engine/GameObject/Components.hpp"
#include "Engine/Resources/MeshCatalog.hpp"
#include "Engine/Core/Application.hpp"
#include "Engine/Sound/SoundEngine.hpp"

namespace Engine {

    void load_level_1(Scene3D* scene);
    void load_level_2(Scene3D* scene);
    void load_level_3(Scene3D* scene);

    bool LoadExampleLevel(const std::string& levelName, Scene3D* scene) {
        if (levelName.compare("Level_1") == 0) {
            load_level_1(scene);
            return true;
        } else if (levelName.compare("Level_2") == 0) {
            load_level_2(scene);
            return true;
        } else if (levelName.compare("Level_3") == 0) {
            load_level_3(scene);
            return true;
        }

        // could not find hard-coded level
        return false;
    }


    void load_level_1(Scene3D* scene) {
        Engine::MeshCatalog::Register("mesh_guard", "Data/Models/guard.nbt", true);
        Engine::MeshCatalog::Register("mesh_plane", "Data/Models/plane.nbt", true);
        auto& cWorld = scene->GetCollisionWorld();

        { // Player
            auto player = scene->CreateGameObject("Player");
            auto mesh = MeshCatalog::Get("mesh_guard");
            player.AddComponent<MeshRendererComponent>(mesh);
            //player.AddComponent<NativeScriptComponent>().Bind<PlayerController>(player);
            BindGameScript("script_player_controller", scene, player);

            mesh->GetSubmeshes()[0].Transform = math::createYawPitchRollMatrix(90, 0, 0);

            auto& trans = player.GetComponent<TransformComponent>().Transform;
            trans = math::mat4();
            trans.translate(math::vec3(0, 1, 0));

            UID_t hull = cWorld.CreateNewCapsule(math::vec3(0, 1, 0) + math::vec3(0, .5, 0), 1, 0.5f);
            player.AddComponent<ColliderComponent>(hull);
        }
        { // Platform
            float platformSize = 20.0f;
            float platformThickness = 3.0f;

            auto platform = scene->CreateGameObject("Platform");

            auto rectMesh = MeshCatalog::Get("mesh_plane");
            auto material = rectMesh->GetMaterial(0);
            material->Set<float>("u_TextureScale", platformSize);
            material->Set("u_AlbedoTexture", Texture2D::Create("Data/Images/grid/PNG/Dark/texture_07.png"));
            platform.AddComponent<MeshRendererComponent>(rectMesh);

            auto& trans = platform.GetComponent<TransformComponent>().Transform;
            trans = math::mat4();
            trans.scale(math::vec3(platformSize, 1, platformSize));
            trans.translate(math::vec3(0, 0.0f, 0));

            UID_t floor = cWorld.CreateNewCubeHull(math::vec3(0, -platformThickness / 2.0f, 0), 2 * platformSize, platformThickness, 2 * platformSize);
        }

        { // Lights
            auto light = scene->CreateGameObject("Sun");
            light.AddComponent<LightComponent>(LightType::Directional, math::vec3(1.0f, 236.0f / 255.0f, 225.0f / 255.0f), 5, 0, 0);
            auto& trans = light.GetComponent<TransformComponent>().Transform;
            trans = math::createYawPitchRollMatrix(45, 0, -80);
        }

        { // Camera
            math::vec2 viewportSize = {
                (float)Application::Get().GetWindow().GetWidth(),
                (float)Application::Get().GetWindow().GetHeight() };

            auto Camera = scene->CreateGameObject("Camera");
            auto& camera = Camera.AddComponent<CameraComponent>().camera;
            camera.SetViewportSize(viewportSize.x, viewportSize.y);
            camera.SetPerspective(75, .01, 100);
            //Camera.AddComponent<NativeScriptComponent>().Bind<CameraController>(Camera);
            BindGameScript("script_camera_controller", scene, Camera);

            auto& trans = Camera.GetComponent<TransformComponent>().Transform;
            trans = math::mat4();
            trans.translate(math::vec3(0, 4, 5));
            trans *= math::createYawPitchRollMatrix(0, 0, -45);
        }

        // Ramps at various angles
        cWorld.getHullFromID(cWorld.CreateNewCubeHull(math::vec3(5, 0, -5), 10, 1, 3))
            ->rotation.toYawPitchRoll(0, 10, 0);
        cWorld.getHullFromID(cWorld.CreateNewCubeHull(math::vec3(5, 1, -2), 10, 1, 3))
            ->rotation.toYawPitchRoll(0, 20, 0);
        cWorld.getHullFromID(cWorld.CreateNewCubeHull(math::vec3(5, 2, 1), 10, 1, 3))
            ->rotation.toYawPitchRoll(0, 30, 0);
        cWorld.getHullFromID(cWorld.CreateNewCubeHull(math::vec3(5, 3, 4), 10, 1, 3))
            ->rotation.toYawPitchRoll(0, 40, 0);
        cWorld.getHullFromID(cWorld.CreateNewCubeHull(math::vec3(5, 3.5, 7), 10, 1, 3))
            ->rotation.toYawPitchRoll(0, 50, 0);

        // Sound stuff
        {
            SoundEngine::CreateSoundCue("guard_death", { "Data/Sounds/death.ogg", 0.02f });
            SoundEngine::CreateSoundCue("golem", { "Data/Sounds/golem.ogg", 0.1f }); //MONO, has 3D sound
            SoundEngine::CreateSoundCue("protector", { "Data/Sounds/sound.wav", 0.2f });
            SoundEngine::CreateSoundCue("ahhh", { "Data/Sounds/ahhh.ogg", 0.1f, 15.0f });
        }
    }

    void load_level_2(Scene3D* scene) {
        Engine::MeshCatalog::Register("mesh_guard",      "Data/Models/guard.nbt", true);
        Engine::MeshCatalog::Register("mesh_breakroom",  "../../Assets/Blender/Level 1/breakroom.nbt", true);
        Engine::MeshCatalog::Register("mesh_floor",      "../../Assets/Blender/Level 1/floor.nbt", true);
        Engine::MeshCatalog::Register("mesh_backrooms",  "../../Assets/Blender/Level 1/backrooms.nbt", true);
        Engine::MeshCatalog::Register("mesh_outerwalls", "../../Assets/Blender/Level 1/outerwalls.nbt", true);
        Engine::MeshCatalog::Register("mesh_safe",       "../../Assets/Blender/Level 1/safe.nbt", true);
        Engine::MeshCatalog::Register("mesh_saferoom",   "../../Assets/Blender/Level 1/saferoom.nbt", true);
        auto& cWorld = scene->GetCollisionWorld();

        { // Player
            auto player = scene->CreateGameObject("Player");
            auto mesh = MeshCatalog::Get("mesh_guard");
            player.AddComponent<MeshRendererComponent>(mesh);
            BindGameScript("script_player_controller", scene, player);

            mesh->GetSubmeshes()[0].Transform = math::createYawPitchRollMatrix(90, 0, 0);

            auto& trans = player.GetComponent<TransformComponent>().Transform;
            trans = math::mat4();
            trans.translate(math::vec3(-8, 1, 12));

            UID_t hull = cWorld.CreateNewCapsule(math::vec3(-8, 1, 12) + math::vec3(0, .5, 0), 1, 0.5f);
            player.AddComponent<ColliderComponent>(hull);
        }

        std::vector<std::string> level_meshes{
            "mesh_breakroom",
            "mesh_floor",
            "mesh_backrooms",
            "mesh_outerwalls",
            "mesh_safe",
            "mesh_saferoom"
        };

        for (const auto& m : level_meshes) {
            auto go = scene->CreateGameObject(m);
            auto mesh = MeshCatalog::Get(m);
            go.AddComponent<MeshRendererComponent>(mesh);
        }
        UID_t floor = cWorld.CreateNewCubeHull(math::vec3(0, -1.5f, 0), 40, 3, 40);

        std::vector<math::vec3> level_point_lights{
            {-8.0f, 3, 12.0f},
            {-10.0f, 3, 10.0f},
            {-14.75f, 3, 6.25f},
            {-0.5f, 3, 11.75f},
            {-4.0f, 3, 6.75f},
            {5.0f, 3, 10.25f},
            {5.0f, 3, 4.75f},
            {5.0f, 3, -2.0f},
            {-4.25f, 3, 1.75f},
            {-1.75f, 3, 1.75f},
            {-12.0f, 3, -1.0f}
        };

        int n = 0;
        for (const auto& light_pos : level_point_lights) {
            auto light = scene->CreateGameObject("Light " + std::to_string(n));
            light.AddComponent<LightComponent>(LightType::Point, math::vec3(1, 1, 1), 3, 0, 0);
            auto& trans = light.GetComponent<TransformComponent>().Transform;
            trans = math::mat4();
            trans.translate(light_pos);
        }

        { // Lights
            auto light = scene->CreateGameObject("Sun");
            light.AddComponent<LightComponent>(LightType::Directional, math::vec3(.8f, .95f, .9f), 5, 0, 0);
            auto& trans = light.GetComponent<TransformComponent>().Transform;
            trans = math::createYawPitchRollMatrix(15, 0, -80);
        }

        { // Camera
            math::vec2 viewportSize = {
                (float)Application::Get().GetWindow().GetWidth(),
                (float)Application::Get().GetWindow().GetHeight() };

            auto Camera = scene->CreateGameObject("Camera");
            auto& camera = Camera.AddComponent<CameraComponent>().camera;
            camera.SetViewportSize(viewportSize.x, viewportSize.y);
            camera.SetPerspective(75, .01, 100);
            //Camera.AddComponent<NativeScriptComponent>().Bind<CameraController>(Camera);
            BindGameScript("script_camera_controller", scene, Camera);

            auto& trans = Camera.GetComponent<TransformComponent>().Transform;
            trans = math::mat4();
            trans.translate(math::vec3(0, 4, 5));
            trans *= math::createYawPitchRollMatrix(0, 0, -45);
        }
    }

    void load_level_3(Scene3D* scene) {
        ENGINE_LOG_WARN("Level 3 is empty!");
    }
}