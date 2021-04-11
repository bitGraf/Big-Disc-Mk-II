#include <enpch.hpp>

#include "MaterialCatalog.hpp"
#include "nbt\nbt.hpp"

namespace Engine {

    std::unordered_map<std::string, MaterialSpec> MaterialMap;
    std::unordered_map<std::string, Ref<Texture2D>> LoadedTextures;

    Ref<Texture2D> MaterialCatalog::GetTexture(const std::string& path) {
        if (LoadedTextures.find(path) == LoadedTextures.end()) {
            // not currently loaded
            LoadedTextures.emplace(path, Texture2D::Create(path));
            ///ENGINE_LOG_TRACE("texture {1}[{0}] loaded!", path, s_Data.LoadedTextures[path]->GetID());
        }
        
        return LoadedTextures.at(path);
    }

    void MaterialCatalog::RegisterMaterial(const std::string& mat_name, const nbt::tag_compound& data) {
        if (MaterialMap.find(mat_name) != MaterialMap.end()) return; // already loaded thia material, don't bother

        MaterialSpec spec;
        spec.Name = data.at("name").as<nbt::tag_string>().get();
        if (data.has_key("albedo_path")) {
            auto albedo_path = data.at("albedo_path").as<nbt::tag_string>().get();
            spec.Albedo = GetTexture(albedo_path);
        }
        if (data.has_key("normal_path")) {
            auto normal_path = data.at("normal_path").as<nbt::tag_string>().get();
            spec.Normal = GetTexture(normal_path);
        }
        if (data.has_key("ambient_path")) {
            auto ambient_path = data.at("ambient_path").as<nbt::tag_string>().get();
            spec.Ambient = GetTexture(ambient_path);
        }
        if (data.has_key("metalness_path")) {
            auto metalness_path = data.at("metalness_path").as<nbt::tag_string>().get();
            spec.Metalness = GetTexture(metalness_path);
        }
        if (data.has_key("roughness_path")) {
            auto roughness_path = data.at("roughness_path").as<nbt::tag_string>().get();
            spec.Roughness = GetTexture(roughness_path);
        }
        if (data.has_key("emissive_path")) {
            auto emissive_path = data.at("emissive_path").as<nbt::tag_string>().get();
            spec.Emissive = GetTexture(emissive_path);
        }

        MaterialMap.emplace(mat_name, spec);
    }

    void MaterialCatalog::Init() {
        BENCHMARK_FUNCTION();

        nbt::file_data data;
        nbt::nbt_byte major, minor;
        endian::endian endianness;
        bool result = nbt::read_from_file("Data/Materials/materials.nbt", data, major, minor, endianness);
        ENGINE_LOG_ASSERT(result, "Failed to load material catalog");

        for (const auto& material : data.second->as<nbt::tag_compound>()) {
            const auto& material_name = material.first;
            const auto& material_data = material.second.as<nbt::tag_compound>();

            RegisterMaterial(material_name, material_data);
            ENGINE_LOG_INFO("Material [{0}] loaded.", material_name);
        }

        // Load some textures manually that I know we'll need
        MaterialCatalog::GetTexture("Data/Images/frog.png");
        MaterialCatalog::GetTexture("Data/Images/normal.png");
        MaterialCatalog::GetTexture("Data/Images/white.png");
        MaterialCatalog::GetTexture("Data/Images/black.png");
        MaterialCatalog::GetTexture("Data/Images/green.png");
    }

    void MaterialCatalog::Shutdown() {
        ENGINE_LOG_INFO("Shutting MaterialCatalog down");
    }

    MaterialSpec MaterialCatalog::GetMaterial(const std::string& material_name) {
        if (MaterialMap.find(material_name) == MaterialMap.end()) {
            MaterialSpec spec;
            __debugbreak();
            return spec;
        }

        return MaterialMap.at(material_name);
    }


    // MD5 related
    void MaterialCatalog::RegisterMaterial(const std::unordered_map<std::string, md5::Material>& materialMap) {
        for (const auto& matEntry : materialMap) {
            if (MaterialMap.find(matEntry.first) != MaterialMap.end()) continue;
            
            // new material to load
            const auto& mat_name = matEntry.first;
            const auto& md5mat = matEntry.second;
            MaterialSpec spec;
            spec.Name = md5mat.name;
            spec.Albedo = GetTexture(md5mat.diffusemap);
            spec.Normal = GetTexture(md5mat.normalmap);

            MaterialMap.emplace(mat_name, spec);
        }
    }
}