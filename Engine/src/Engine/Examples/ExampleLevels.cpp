#include <enpch.hpp>

#include "ExampleLevels.hpp"
#include "Engine/Scene/Scene3D.hpp"

#include "Engine/GameObject/Components.hpp"
#include "Engine/Resources/MeshCatalog.hpp"
#include "Engine/Resources/MaterialCatalog.hpp"
#include "Engine/Core/Application.hpp"
#include "Engine/Sound/SoundEngine.hpp"
#include "Engine/Core/Utils.hpp"

#include "Engine/Resources/ResourceManager.hpp"

#include "Engine/Scripts/CameraController.hpp"
#include "Engine/Scripts/Player.hpp"

namespace rh {

    void load_level_1(Scene3D* scene);
    void load_level_2(Scene3D* scene);
    void load_level_3(Scene3D* scene);
    void load_level_4(Scene3D* scene);

    bool LoadExampleLevel(const std::string& levelName, Scene3D* scene) {
        if (levelName.compare("Level_1") == 0) {
            load_level_1(scene);
            return true;
        }

        if (levelName.compare("Level_2") == 0) {
            load_level_2(scene);
            return true;
        }

        if (levelName.compare("nbtTest") == 0) {
            load_level_3(scene);
            return true;
        }

        if (levelName.compare("Level_4") == 0) {
            load_level_4(scene);
            return true;
        }

        // could not find hard-coded level
        return false;
    }

    void load_level_1(Scene3D* scene) {
        auto& cWorld = scene->GetCollisionWorld();

        // load materials
        // ANIM_HOOK std::unordered_map<std::string, md5::Material> mats;
        // ANIM_HOOK md5::LoadMD5MaterialDefinitionFile("Data/Models/tentacle/tentacle.md5material", mats);
        // ANIM_HOOK MaterialCatalog::RegisterMaterial(mats);

        MeshCatalog::Register("mesh_plane", "Data/Models/plane.nbt", FileFormat::NBT_Basic);
        MeshCatalog::Register("dancer", "Data/Models/dance.mesh", FileFormat::MESH_File);
        MeshCatalog::Register("mesh_cube", "Data/Models/cube.nbt", FileFormat::NBT_Basic);

        // Tentacle
        {
            auto tentacle = scene->CreateGameObject("dancer");
            auto mesh = MeshCatalog::Get("dancer");
            //mesh->GetSubmeshes()[0].Transform = laml::mul(laml::Mat4(1.0f, 1.0f, 1.0f, 1.0f), mesh->GetSubmeshes()[0].Transform);
            //mesh->GetSubmeshes()[0].Transform.c_24 += 0.5f;
            tentacle.AddComponent<MeshRendererComponent>(mesh);
            auto& trans = tentacle.GetComponent<TransformComponent>().Transform;
            laml::transform::create_transform_translate(trans, 0.0f, 1.0f, 0.0f);
        }

        // Player
        {
            auto player = scene->CreateGameObject("Player");

            auto mesh = MeshCatalog::Get("mesh_cube");
            player.AddComponent<MeshRendererComponent>(mesh);
            player.AddComponent<rh::NativeScriptComponent>().Bind<PlayerController>(player);

            auto& trans = player.GetComponent<TransformComponent>().Transform;
            //laml::transform::create_transform(trans, laml::Mat3(1.0f), laml::Vec3(0.0f, 1.0f, 0.0f), laml::Vec3(1.0f, 1.0f, 1.0f));
            laml::transform::create_transform_translate(trans, laml::Vec3(0.0f, 1.0f, 0.0f));

            UID_t hull = cWorld.CreateNewCapsule(laml::Vec3(0, 1, 0) + laml::Vec3(0, .5, 0), 1, 0.5f);
            player.AddComponent<ColliderComponent>(hull);
        }

        // Platform
        {
            float platformSize = 20.0f;
            float platformThickness = 3.0f;

            auto platform = scene->CreateGameObject("Platform");

            auto rectMesh = MeshCatalog::Get("mesh_plane");
            auto material = rectMesh->GetMaterial(0);
            material->Set<float>("u_TextureScale", platformSize);
            material->Set("u_AlbedoTexture", MaterialCatalog::GetTexture("Data/Images/grid/PNG/Dark/texture_07.png"));

            platform.AddComponent<MeshRendererComponent>(rectMesh);

            auto& trans = platform.GetComponent<TransformComponent>().Transform;
            //math::CreateScale(trans, platformSize, 1, platformSize);
            laml::transform::create_transform_scale(trans, platformSize, 1.0f, platformSize);

            UID_t floor = cWorld.CreateNewCubeHull(laml::Vec3(0, -platformThickness / 2.0f, 0), 2 * platformSize, platformThickness, 2 * platformSize);
        }

        // Lights
        {
            auto light = scene->CreateGameObject("Sun");

            light.AddComponent<LightComponent>(LightType::Directional, laml::Vec3(1.0f, 236.0f / 255.0f, 225.0f / 255.0f), 5, 0, 0);

            auto& trans = light.GetComponent<TransformComponent>().Transform;
            laml::transform::create_transform_rotation(trans, 45.0f, -80.0f, 0.0f);
        }

        // Camera
        {
            laml::Vec2 viewportSize = {
                (float)Application::Get().GetWindow().GetWidth(),
                (float)Application::Get().GetWindow().GetHeight() };

            auto Camera = scene->CreateGameObject("Camera");

            auto& camera = Camera.AddComponent<CameraComponent>().camera;
            camera.SetViewportSize(viewportSize.x, viewportSize.y);
            camera.SetPerspective(75, .01, 100);

            Camera.AddComponent<rh::NativeScriptComponent>().Bind<CameraController>(Camera);

            auto& trans = Camera.GetComponent<TransformComponent>().Transform;
            laml::transform::create_transform_translate(trans, 0.0f, 4.0f, 5.0f);
            laml::Mat4 rotM;
            laml::transform::create_transform_rotation(rotM, 0.0f, -45.0f, 0.0f);
            trans = laml::mul(trans, rotM);
        }

        // Sound stuff
        {
            SoundEngine::CreateSoundCue("guard_death", { "Data/Sounds/death.ogg", 0.02f });
            SoundEngine::CreateSoundCue("golem", { "Data/Sounds/golem.ogg", 0.1f }); //MONO, has 3D sound
            SoundEngine::CreateSoundCue("protector", { "Data/Sounds/sound.wav", 0.2f });
            SoundEngine::CreateSoundCue("ahhh", { "Data/Sounds/ahhh.ogg", 0.1f, 15.0f });
        }

        /*
        // read md5mesh files
        MeshCatalog::Register("mesh_plane", "Data/Models/plane.nbt", FileFormat::NBT_Basic);
        MeshCatalog::Register("mesh_test", "Data/Models/guard/guard.md5mesh", FileFormat::MD5_Text);
        MeshCatalog::Register("test2", "Data/Models/output.mesh", FileFormat::MESH_File);

        // read animation files
        AnimCatalog::Register("anim_test", "Data/Models/guard/Walking.md5anim", FileFormat::MD5_Text);

        // Player
        {
            auto player = scene->CreateGameObject("Player");
            auto mesh = MeshCatalog::Get("mesh_test");
            auto anim = AnimCatalog::Get("anim_test");

            player.AddComponent<MeshRendererComponent>(mesh);
            player.AddComponent<MeshAnimationComponent>(anim);
            player.AddComponent<rh::NativeScriptComponent>().Bind<PlayerController>(player);

            auto& trans = player.GetComponent<TransformComponent>().Transform;
            //math::CreateTranslation(trans, laml::Vec3(0, 1, 0));
            math::CreateTransform(trans, laml::Mat3(), laml::Vec3(0, 1, 0), laml::Vec3(1, 1, 1));

            UID_t hull = cWorld.CreateNewCapsule(laml::Vec3(0, 1, 0) + laml::Vec3(0, .5, 0), 1, 0.5f);
            player.AddComponent<ColliderComponent>(hull);
        }

        // Platform
        {
            float platformSize = 20.0f;
            float platformThickness = 3.0f;

            auto platform = scene->CreateGameObject("Platform");

            auto rectMesh = MeshCatalog::Get("mesh_plane");
            auto material = rectMesh->GetMaterial(0);
            material->Set<float>("u_TextureScale", platformSize);
            material->Set("u_AlbedoTexture", MaterialCatalog::GetTexture("Data/Images/grid/PNG/Dark/texture_07.png"));

            platform.AddComponent<MeshRendererComponent>(rectMesh);

            auto& trans = platform.GetComponent<TransformComponent>().Transform;
            math::CreateScale(trans, platformSize, 1, platformSize);

            UID_t floor = cWorld.CreateNewCubeHull(laml::Vec3(0, -platformThickness / 2.0f, 0), 2 * platformSize, platformThickness, 2 * platformSize);
        }

        // Lights
        {
            auto light = scene->CreateGameObject("Sun");

            light.AddComponent<LightComponent>(LightType::Directional, laml::Vec3(1.0f, 236.0f / 255.0f, 225.0f / 255.0f), 5, 0, 0);

            auto& trans = light.GetComponent<TransformComponent>().Transform;
            math::CreateRotationFromYawPitchRoll(trans, 45, -80, 0);
        }

        // Camera
        {
            laml::Vec2 viewportSize = {
                (float)Application::Get().GetWindow().GetWidth(),
                (float)Application::Get().GetWindow().GetHeight() };

            auto Camera = scene->CreateGameObject("Camera");

            auto& camera = Camera.AddComponent<CameraComponent>().camera;
            camera.SetViewportSize(viewportSize.x, viewportSize.y);
            camera.SetPerspective(75, .01, 100);

            Camera.AddComponent<rh::NativeScriptComponent>().Bind<CameraController>(Camera);

            auto& trans = Camera.GetComponent<TransformComponent>().Transform;
            math::CreateTranslation(trans, laml::Vec3(0, 4, 5));
            laml::Mat4 rotM;
            math::CreateRotationFromYawPitchRoll(rotM, 0, -45, 0);
            trans *= rotM;
        }
        */
    }

    void load_level_2(Scene3D* scene) {
        rh::MeshCatalog::Register("mesh_guard",      "Data/Models/guard.nbt", FileFormat::NBT_Basic);
        rh::MeshCatalog::Register("mesh_breakroom",  "../../Assets/Blender/Level 1/breakroom.nbt", FileFormat::NBT_Basic);
        rh::MeshCatalog::Register("mesh_floor",      "../../Assets/Blender/Level 1/floor.nbt", FileFormat::NBT_Basic);
        rh::MeshCatalog::Register("mesh_backrooms",  "../../Assets/Blender/Level 1/backrooms.nbt", FileFormat::NBT_Basic);
        rh::MeshCatalog::Register("mesh_outerwalls", "../../Assets/Blender/Level 1/outerwalls.nbt", FileFormat::NBT_Basic);
        rh::MeshCatalog::Register("mesh_safe",       "../../Assets/Blender/Level 1/safe.nbt", FileFormat::NBT_Basic);
        rh::MeshCatalog::Register("mesh_saferoom",   "../../Assets/Blender/Level 1/saferoom.nbt", FileFormat::NBT_Basic);
        auto& cWorld = scene->GetCollisionWorld();

        { // Player
            auto player = scene->CreateGameObject("Player");
            auto mesh = MeshCatalog::Get("mesh_guard");
            player.AddComponent<MeshRendererComponent>(mesh);
            player.AddComponent<rh::NativeScriptComponent>().Bind<PlayerController>(player);

            laml::transform::create_transform_rotation(mesh->GetSubmeshes()[0].Transform, 90.0f, 0.0f, 0.0f);

            auto& trans = player.GetComponent<TransformComponent>().Transform;
            laml::transform::create_transform_translate(trans, -8.0f, 1.0f, 12.0f);

            UID_t hull = cWorld.CreateNewCapsule(laml::Vec3(-8, 1, 12) + laml::Vec3(0, .5, 0), 1, 0.5f);
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
        UID_t floor = cWorld.CreateNewCubeHull(laml::Vec3(0, -1.5f, 0), 40, 3, 40);

        std::vector<laml::Vec3> level_point_lights{
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
            light.AddComponent<LightComponent>(LightType::Point, laml::Vec3(1, 1, 1), 3, 0, 0);
            auto& trans = light.GetComponent<TransformComponent>().Transform;
            laml::transform::create_transform_translate(trans, light_pos);
        }

        { // Lights
            auto light = scene->CreateGameObject("Sun");
            light.AddComponent<LightComponent>(LightType::Directional, laml::Vec3(.8f, .95f, .9f), 5, 0, 0);
            auto& trans = light.GetComponent<TransformComponent>().Transform;
            laml::transform::create_transform_rotation(trans, 15.0f, -80.0f, 0.0f);
        }

        { // Camera
            laml::Vec2 viewportSize = {
                (float)Application::Get().GetWindow().GetWidth(),
                (float)Application::Get().GetWindow().GetHeight() };

            auto Camera = scene->CreateGameObject("Camera");
            auto& camera = Camera.AddComponent<CameraComponent>().camera;
            camera.SetViewportSize(viewportSize.x, viewportSize.y);
            camera.SetPerspective(75, .01, 100);
            Camera.AddComponent<NativeScriptComponent>().Bind<CameraController>(Camera);

            auto& trans = Camera.GetComponent<TransformComponent>().Transform;
            laml::transform::create_transform_translate(trans, 0.0f, 4.0f, 5.0f);
            laml::Mat4 rotM;
            laml::transform::create_transform_rotation(rotM, 0.0f, -45.0f, 0.0f);
            trans = laml::mul(trans, rotM);
        }
    }

    void load_level_3(Scene3D* scene) {
        rh::MeshCatalog::Register("mesh_guard", "Data/Models/guard.nbt", FileFormat::NBT_Basic);
        rh::MeshCatalog::Register("mesh_plane", "Data/Models/plane.nbt", FileFormat::NBT_Basic);
        auto& cWorld = scene->GetCollisionWorld();

        { // Player
            auto player = scene->CreateGameObject("Player");
            auto mesh = MeshCatalog::Get("mesh_guard");
            player.AddComponent<MeshRendererComponent>(mesh);
            player.AddComponent<NativeScriptComponent>().Bind<PlayerController>(player);

            laml::transform::create_transform_rotation(mesh->GetSubmeshes()[0].Transform, 90.0f, 0.0f, 0.0f);

            auto& trans = player.GetComponent<TransformComponent>().Transform;
            laml::transform::create_transform_translate(trans, 0.0f, 1.0f, 0.0f);

            UID_t hull = cWorld.CreateNewCapsule(laml::Vec3(0, 1, 0) + laml::Vec3(0, .5, 0), 1, 0.5f);
            player.AddComponent<ColliderComponent>(hull);
        }
        { // Platform
            float platformSize = 20.0f;
            float platformThickness = 3.0f;

            auto platform = scene->CreateGameObject("Platform");

            auto rectMesh = MeshCatalog::Get("mesh_plane");
            auto material = rectMesh->GetMaterial(0);
            material->Set<float>("u_TextureScale", platformSize);
            material->Set("u_AlbedoTexture", MaterialCatalog::GetTexture("Data/Images/grid/PNG/Dark/texture_07.png"));
            platform.AddComponent<MeshRendererComponent>(rectMesh);

            auto& trans = platform.GetComponent<TransformComponent>().Transform;
            laml::transform::create_transform_scale(trans, platformSize, 1.0f, platformSize);

            UID_t floor = cWorld.CreateNewCubeHull(laml::Vec3(0, -platformThickness / 2.0f, 0), 2 * platformSize, platformThickness, 2 * platformSize);
        }

        { // Lights
            auto light = scene->CreateGameObject("Sun");
            light.AddComponent<LightComponent>(LightType::Directional, laml::Vec3(1.0f, 236.0f / 255.0f, 225.0f / 255.0f), 5, 0, 0);
            auto& trans = light.GetComponent<TransformComponent>().Transform;
            laml::transform::create_transform_rotation(trans, 45.0f, -80.0f, 0.0f);
        }

        { // Camera
            laml::Vec2 viewportSize = {
                (float)Application::Get().GetWindow().GetWidth(),
                (float)Application::Get().GetWindow().GetHeight() };

            auto Camera = scene->CreateGameObject("Camera");
            auto& camera = Camera.AddComponent<CameraComponent>().camera;
            camera.SetViewportSize(viewportSize.x, viewportSize.y);
            camera.SetPerspective(75, .01, 100);
            Camera.AddComponent<NativeScriptComponent>().Bind<CameraController>(Camera);

            auto& trans = Camera.GetComponent<TransformComponent>().Transform;
            laml::transform::create_transform_translate(trans, 0.0f, 4.0f, 5.0f);
            laml::Mat4 rotM;
            laml::transform::create_transform_rotation(rotM, 0.0f, -45.0f, 0.0f);
            trans = laml::mul(trans, rotM);
        }

        // Ramps at various angles
        laml::Mat3& rot = cWorld.getHullFromID(cWorld.CreateNewCubeHull(laml::Vec3(5, 0, -5), 10, 1, 3))->rotation;
        laml::transform::create_transform_rotation(rot, 0.0f, 10.0f, 0.0f);

        rot = cWorld.getHullFromID(cWorld.CreateNewCubeHull(laml::Vec3(5, 1, -2), 10, 1, 3))->rotation;
        laml::transform::create_transform_rotation(rot, 0.0f, 20.0f, 0.0f);

        rot = cWorld.getHullFromID(cWorld.CreateNewCubeHull(laml::Vec3(5, 2, 1), 10, 1, 3))->rotation;
        laml::transform::create_transform_rotation(rot, 0.0f, 30.0f, 0.0f);

        rot = cWorld.getHullFromID(cWorld.CreateNewCubeHull(laml::Vec3(5, 3, 4), 10, 1, 3))->rotation;
        laml::transform::create_transform_rotation(rot, 0.0f, 40.0f, 0.0f);

        rot = cWorld.getHullFromID(cWorld.CreateNewCubeHull(laml::Vec3(5, 3.5, 7), 10, 1, 3))->rotation;
        laml::transform::create_transform_rotation(rot, 0.0f, 50.0f, 0.0f);

        // Sound stuff
        {
            SoundEngine::CreateSoundCue("guard_death", { "Data/Sounds/death.ogg", 0.02f });
            SoundEngine::CreateSoundCue("golem", { "Data/Sounds/golem.ogg", 0.1f }); //MONO, has 3D sound
            SoundEngine::CreateSoundCue("protector", { "Data/Sounds/sound.wav", 0.2f });
            SoundEngine::CreateSoundCue("ahhh", { "Data/Sounds/ahhh.ogg", 0.1f, 15.0f });
        }
    }

    void load_level_4(Scene3D* scene) {
        ResourceManager::RegisterTexture("texture/frog.png");
        auto frog_tex = ResourceManager::GetTexture("texture/frog.png");
    }
}