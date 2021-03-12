//#define RUN_TEST_CODE
//#define RUN_MESH_CODE

#ifndef RUN_TEST_CODE
#include <Engine.hpp>
#include <Engine/EntryPoint.h>

#include "GameLayer.hpp"
#include "MenuLayer.hpp"

class Game : public Engine::Application {
public:
    Game() {
        m_GameLayer = new GameLayer();
        PushLayer(m_GameLayer);

        //m_MainMenuLayer = new MenuLayer();
        //m_MainMenuLayer->createMenu();
        //PushLayer(m_MainMenuLayer);
    }

    ~Game() {
    }

private:
    MenuLayer* m_MainMenuLayer;
    GameLayer* m_GameLayer;
};

Engine::Application* Engine::CreateApplication() {
    return new Game();
}

#endif

#ifdef RUN_TEST_CODE
#ifndef RUN_MESH_CODE
#include "Engine\Resources\nbt\nbt.hpp"

#include "Engine\Core\GameMath.hpp"
#include <assert.h>
//#include "Engine\Resources\nbt\test.hpp"

int main(int argc, char** argv) {
    //nbtTest::test5();
    
    // try to generate a level file in .nbt format
    using namespace nbt;
    tag_compound comp{
        {"name", "Level NBT Test"},
        {"meshes", tag_list::of<tag_compound>({ //TODO: add tag_string_list possibly?
            {{"mesh_name", "cube_mesh"}, {"mesh_path", "run_tree/Data/Models/cube.mesh" }},
            {{"mesh_name", "rect_mesh"}, {"mesh_path", "run_tree/Data/Models/cube.mesh" }},
            {{"mesh_name", "guard_mesh"}, {"mesh_path", "run_tree/Data/Models/guard.nbt" }, {"nbt", true}} // flag as an nbt file
            })},
        {"entities", tag_list::of<tag_compound>({
            { // entity 1
                {"name", "Platform"}, 
                {"transform", tag_compound{
                    {"position", math::vec3(0,-1,0)},
                    //{"rotation", math::vec4(0,0,0,1)},
                    {"scale", math::vec3(5,.25,5)}
                }},
                {"components", tag_list::of<tag_compound>({
                    {{"type", "MeshRenderer"}, {"mesh_name", "rect_mesh"}}
                    })
                }},
            { // entity 2
                {"name", "Frog Cube 1"},
                {"transform", tag_compound{
                    {"position", math::vec3(2,1,-2)}
                }},
                {"components", tag_list::of<tag_compound>({
                    {{"type", "MeshRenderer"},{"mesh_name", "cube_mesh"}},
                    {{"type", "NativeScript"},{"script_tag", "script_gem"}}
                    })
                }},
            {  // entity 3
                { "name", "Frog Cube 2" },
                { "transform", tag_compound{
                    { "position", math::vec3(-2,1,-2) }
                } },
                { "components", tag_list::of<tag_compound>({
                    { { "type", "MeshRenderer" },{ "mesh_name", "cube_mesh" } },
                    { { "type", "NativeScript" },{ "script_tag", "script_gem" } }
                    })
                } },
            {  // entity 4
                { "name", "Frog Cube 3" },
                { "transform", tag_compound{
                    { "position", math::vec3(2,1,2) }
                } },
                { "components", tag_list::of<tag_compound>({
                    { { "type", "MeshRenderer" },{ "mesh_name", "guard_mesh" } },
                    { { "type", "NativeScript" },{ "script_tag", "script_gem" } }
                    })
                } },
            { // entity 5
                { "name", "Camera" },
                { "transform", tag_compound{
                    { "position", math::vec3(0, 1, 5) }
                } },
                { "components", tag_list::of<tag_compound>({
                    { { "type", "Camera" } },
                    { { "type", "NativeScript" },{ "script_tag", "script_camera_controller" } }
                    })
                }}
            })}
    };

    auto name = comp["name"].as<tag_string>().get();
    std::cout << "Level name: " << name << std::endl;

    auto meshList = comp["meshes"].as<tag_list>();
    assert(meshList.el_type() == tag_type::Compound);
    for (const auto& m : meshList) {
        auto c = m.as<tag_compound>();
        std::cout << c["mesh_name"] << ", " << c["mesh_path"] << std::endl;
    }


    nbt::write_to_file("run_tree/Data/Levels/nbtTest.scene", nbt::file_data({ "level_data", std::make_unique<tag_compound>(comp) }), 0, 1);

    system("pause");
}

#endif

#ifdef RUN_MESH_CODE
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <utility>
#include <assert.h>
#include "Engine/Core/GameMath.hpp"
#include "Engine/Core/DataFile.hpp"
using namespace math;

struct Vertex
{
    vec3 Position;
    vec3 Normal;
    vec3 Tangent;
    vec3 Binormal;
    vec2 Texcoord;
};

int main(int argc, char** argv) {
    std::cout << "Running..." << std::endl;
    DataFile file;
    DataFile fin;

    /* Write to the file */
    {
        /* Create mesh data */
        std::vector<Vertex> vertices;
        vertices.resize(24);
        std::vector<u32> indices;
        {
            indices = {
                0, 1, 2, //front
                0, 2, 3,

                4, 5, 6, //top
                4, 6, 7,

                8, 9, 10, //right
                8, 10, 11,

                12, 13, 14, //left
                12, 14, 15,

                16, 17, 18, //bot
                16, 18, 19,

                20, 21, 22, //back
                20, 22, 23
            };

            scalar s = 0.5f;
            vertices[0].Position =  { -s, -s,  s }; //front
            vertices[1].Position =  {  s, -s,  s };
            vertices[2].Position =  {  s,  s,  s };
            vertices[3].Position =  { -s,  s,  s };
                                    
            vertices[4].Position =  { -s,  s,  s }; //top
            vertices[5].Position =  {  s,  s,  s };
            vertices[6].Position =  {  s,  s, -s };
            vertices[7].Position =  { -s,  s, -s };

            vertices[8].Position =  {  s, -s,  s }; //right
            vertices[9].Position =  {  s, -s, -s };
            vertices[10].Position = {  s,  s, -s };
            vertices[11].Position = {  s,  s,  s };

            vertices[12].Position = { -s, -s, -s }; //left
            vertices[13].Position = { -s, -s,  s };
            vertices[14].Position = { -s,  s,  s };
            vertices[15].Position = { -s,  s, -s };
                                       
            vertices[16].Position = { -s, -s, -s }; //bot
            vertices[17].Position = {  s, -s, -s };
            vertices[18].Position = {  s, -s,  s };
            vertices[19].Position = { -s, -s,  s };
                                       
            vertices[20].Position = {  s, -s, -s }; //back
            vertices[21].Position = { -s, -s, -s };
            vertices[22].Position = { -s,  s, -s };
            vertices[23].Position = {  s,  s, -s };

            // Normals
            vertices[0].Normal= { 0, 0, 1 }; //front
            vertices[1].Normal= { 0, 0, 1 };
            vertices[2].Normal= { 0, 0, 1 };
            vertices[3].Normal= { 0, 0, 1 };
                       
            vertices[4].Normal= { 0, 1, 0 }; //top
            vertices[5].Normal= { 0, 1, 0 };
            vertices[6].Normal= { 0, 1, 0 };
            vertices[7].Normal= { 0, 1, 0 };
                       
            vertices[8].Normal= { 1, 0, 0 }; //right
            vertices[9].Normal= { 1, 0, 0 };
            vertices[10].Normal = { 1, 0, 0 };
            vertices[11].Normal = { 1, 0, 0 };
                        
            vertices[12].Normal = { -1, 0, 0 }; //left
            vertices[13].Normal = { -1, 0, 0 };
            vertices[14].Normal = { -1, 0, 0 };
            vertices[15].Normal = { -1, 0, 0 };
                        
            vertices[16].Normal = { 0, -1, 0 }; //bot
            vertices[17].Normal = { 0, -1, 0 };
            vertices[18].Normal = { 0, -1, 0 };
            vertices[19].Normal = { 0, -1, 0 };
                        
            vertices[20].Normal = { 0, 0, -1 }; //back
            vertices[21].Normal = { 0, 0, -1 };
            vertices[22].Normal = { 0, 0, -1 };
            vertices[23].Normal = { 0, 0, -1 };

            // Tangents
            vertices[0].Tangent = { 1, 0, 0 }; //front
            vertices[1].Tangent = { 1, 0, 0 };
            vertices[2].Tangent = { 1, 0, 0 };
            vertices[3].Tangent = { 1, 0, 0 };

            vertices[4].Tangent = { 0, 0, 1 }; //top
            vertices[5].Tangent = { 0, 0, 1 };
            vertices[6].Tangent = { 0, 0, 1 };
            vertices[7].Tangent = { 0, 0, 1 };

            vertices[8].Tangent = { 0, 1, 0 }; //right
            vertices[9].Tangent = { 0, 1, 0 };
            vertices[10].Tangent = { 0, 1, 0 };
            vertices[11].Tangent = { 0, 1, 0 };
                         
            vertices[12].Tangent = { 0, -1, 0 }; //left
            vertices[13].Tangent = { 0, -1, 0 };
            vertices[14].Tangent = { 0, -1, 0 };
            vertices[15].Tangent = { 0, -1, 0 };
                         
            vertices[16].Tangent = { 0, 0, -1 }; //bot
            vertices[17].Tangent = { 0, 0, -1 };
            vertices[18].Tangent = { 0, 0, -1 };
            vertices[19].Tangent = { 0, 0, -1 };
                         
            vertices[20].Tangent = { -1, 0, 0 }; //back
            vertices[21].Tangent = { -1, 0, 0 };
            vertices[22].Tangent = { -1, 0, 0 };
            vertices[23].Tangent = { -1, 0, 0 };

            for (auto& v : vertices) {
                //v.Normal = v.Position.get_unit();
                //v.Tangent = vec3(0, 1, 0);
                v.Binormal = v.Normal.cross(v.Tangent);
                v.Tangent = v.Binormal.cross(v.Normal);
            }

            vertices[0].Texcoord =  { 0, 0 };
            vertices[1].Texcoord =  { 1, 0 };
            vertices[2].Texcoord =  { 1, 1 };
            vertices[3].Texcoord =  { 0, 1 };
                                    
            vertices[4].Texcoord =  { 0, 1 };
            vertices[5].Texcoord =  { 0, 0 };
            vertices[6].Texcoord =  { 1, 0 };
            vertices[7].Texcoord =  { 1, 1 };
                                    
            vertices[8].Texcoord =  { 0, 1 };
            vertices[9].Texcoord =  { 0, 0 };
            vertices[10].Texcoord = { 1, 0 };
            vertices[11].Texcoord = { 1, 1 };

            vertices[12].Texcoord = { 1, 0 };
            vertices[13].Texcoord = { 1, 1 };
            vertices[14].Texcoord = { 0, 1 };
            vertices[15].Texcoord = { 0, 0 };

            vertices[16].Texcoord = { 1, 0 };
            vertices[17].Texcoord = { 1, 1 };
            vertices[18].Texcoord = { 0, 1 };
            vertices[19].Texcoord = { 0, 0 };

            vertices[20].Texcoord = { 1, 1 };
            vertices[21].Texcoord = { 0, 1 };
            vertices[22].Texcoord = { 0, 0 };
            vertices[23].Texcoord = { 1, 0 };
        }

        // Materials
        DataFile::Block matBlock("Materials");
        {
            // base material data
            matBlock.push<vec3>(vec3(1,1,1)); // u_AlbedoColor
            matBlock.push<float>(0.0f); // u_Metalness
            matBlock.push<float>(0.75f); // u_Roughness

            u8 numTextures = 4;
            matBlock.push_byte(numTextures);

            // AlbedoTexture
            std::string albedoPath =    "run_tree/Data/Images/waffle/WaffleSlab2_albedo.png";
            std::string normalPath =    "run_tree/Data/Images/waffle/WaffleSlab2_normal.png";
            std::string metalPath =     "run_tree/Data/Images/frog.png"; // not yet
            std::string roughnessPath = "run_tree/Data/Images/waffle/WaffleSlab2_roughness.png";
            
            matBlock.push_string("u_AlbedoTexture");
            matBlock.push_string(albedoPath);
            matBlock.push_string("u_NormalTexture");
            matBlock.push_string(normalPath);
            matBlock.push_string("u_MetalnessTexture");
            matBlock.push_string(metalPath);
            matBlock.push_string("u_RoughnessTexture");
            matBlock.push_string(roughnessPath);

            u8 numMaterials = 2;
            matBlock.push_byte(numMaterials);

            /// mat1
            std::string mat1Name = "mat1";
            matBlock.push_string(mat1Name);

            matBlock.push<vec3>(vec3(1, .5, .5)); // u_AlbedoColor
            matBlock.push<float>(0.0f); // u_Metalness
            matBlock.push<float>(0.75f); // u_Roughness

            matBlock.push_byte(0); // numTextures

            /// mat2
            std::string mat2Name = "mat2";
            matBlock.push_string(mat2Name);

            matBlock.push<vec3>(vec3(.5, 1, .5)); // u_AlbedoColor
            matBlock.push<float>(0.0f); // u_Metalness
            matBlock.push<float>(0.75f); // u_Roughness

            matBlock.push_byte(0); // numTextures
        }

        // Mesh
        DataFile::Block meshBlock("Mesh");
        {
            std::string meshName = "Cube";
            meshBlock.push_string(meshName);

            u8 numSubmeshes = 2;
            u16 numVerts = vertices.size();
            u16 numInds = indices.size();
            meshBlock.push_byte(numSubmeshes);
            meshBlock.push_short(numVerts);
            meshBlock.push_short(numInds);
            meshBlock.push_data(reinterpret_cast<u8*>(&vertices[0]), vertices.size() * sizeof(vertices[0]));
            meshBlock.push_data(reinterpret_cast<u8*>(&indices[0]), indices.size() * sizeof(indices[0]));

            u8 meshFlag = 0;
            meshBlock.push_byte(meshFlag);

            /// submesh 1
            std::string sm1Name = "subcube1";
            u8 sm1Index = 0;
            mat4 sm1Transform({ 1,0,0,0 }, { 0, 1,0,0 }, { 0,0,1,0 }, { 0,0,0,1 });
            u16 sm1startIndex = 0;
            u16 sm1indexCount = 18;
            meshBlock.push_string(sm1Name);
            meshBlock.push_byte(sm1Index);
            meshBlock.push_data(reinterpret_cast<u8*>(&sm1Transform._11), sizeof(sm1Transform));
            meshBlock.push_short(sm1startIndex);
            meshBlock.push_short(sm1indexCount);

            /// submesh 2
            std::string sm2Name = "subcube2";
            u16 sm2Index = 1;
            mat4 sm2Transform({ 1,0,0,0 }, { 0,1,0,0 }, { 0,0,1,0 }, { 0,0,0,1 });
            u16 sm2startIndex = 18;
            u16 sm2indexCount = 18;
            meshBlock.push_string(sm2Name);
            meshBlock.push_byte(sm2Index);
            meshBlock.push_data(reinterpret_cast<u8*>(&sm2Transform._11), sizeof(sm2Transform));
            meshBlock.push_short(sm2startIndex);
            meshBlock.push_short(sm2indexCount);
        }

        // Skeleton
        DataFile::Block skelBlock("Skeleton");
        {
            std::string skelName = "CubeSkeleton";
            skelBlock.push_string(skelName);
            u8 numBones = 77;
            mat4 invTransform({ 1,2,3,4 }, { 5, 6, 7, 8 }, { 9, 10, 11, 12 }, { 13, 14, 15, 16 });
            skelBlock.push_byte(numBones);
            skelBlock.push_data(reinterpret_cast<u8*>(&invTransform._11), sizeof(invTransform));
        }

        // Prepare for writing
        file.AddBlock(matBlock);
        file.AddBlock(meshBlock);
        file.AddBlock(skelBlock);
        file.ResolveOffsets();
        file.WriteToFile("run_tree/Data/Models/cube.mesh");
    }

    /* Read from the file and extract data */
    {
        fin.ReadFromFile("run_tree/Data/Models/cube.mesh");
        if (fin.fileLength > 0) {

            {
                auto& block = fin.GetBlock("Skeleton");
                auto skelName = block.read_string();
                auto numBones = block.read_byte();
                mat4 invTransform = block.read<mat4>();
            }

            /*
            {
                auto& block = fin.GetBlock("Materials");
                auto baseColor = block.read<vec4>();
                auto numTextures = block.read_byte();
                for (int n = 0; n < numTextures; n++) {
                    auto texName = block.read_string();
                    auto texPath = block.read_string();
                }
                auto numMats = block.read_byte();
                for (int n = 0; n < numMats; n++) {
                    auto matName = block.read_string();
                    vec4 u_color = block.read<vec4>();
                    auto numTextures = block.read_byte();
                    for (int n = 0; n < numTextures; n++) {
                        auto texName = block.read_string();
                        auto texPath = block.read_string();
                    }
                }
            }
            */

            {
                auto& block = fin.GetBlock("Mesh");
                auto meshName = block.read_string();
                auto numSubmeshes = block.read_byte();
                auto numVerts = block.read_short();
                auto numInds = block.read_short();

                Vertex* __new_vertices_ = new Vertex[numVerts];
                block.read_data(reinterpret_cast<u8*>(__new_vertices_), numVerts * sizeof(Vertex));
                std::vector<Vertex> verts;
                verts.reserve(numVerts);
                verts.assign(__new_vertices_, __new_vertices_ + numVerts);
                delete[] __new_vertices_;

                u32* __new_indices_ = new u32[numInds];
                block.read_data(reinterpret_cast<u8*>(__new_indices_), numInds * sizeof(u32));
                std::vector<u32> inds;
                inds.reserve(numInds);
                inds.assign(__new_indices_, __new_indices_ + numInds);
                delete[] __new_indices_;

                auto meshFlag = block.read_byte();

                // load submeshes
                for (int n = 0; n < numSubmeshes; n++) {
                    auto smName = block.read_string();
                    auto smMatIdx = block.read_byte();
                    auto smTransf = block.read<mat4>();
                    auto smStartIdx = block.read_short();
                    auto smIdxCount = block.read_short();
                }
            }
        }
    }

    return 0;
}

#endif
#endif