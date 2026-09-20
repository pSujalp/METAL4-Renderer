#include "Model.h"
#include <cmath>
#include <cstring>
#include <filesystem>
#include <unordered_map>

namespace
{
    
    
    
    void GenerateMissingTangentSpace(std::vector<Mesh_Vertices> &verts,
                                     const std::vector<uint32_t> &indices,
                                     bool needNormals, bool needTangents)
    {
        const size_t n = verts.size();
        const float3 zero = float3{0.0f, 0.0f, 0.0f};
        std::vector<float3> nAcc(n, zero), tAcc(n, zero), bAcc(n, zero);

        for (size_t i = 0; i + 2 < indices.size(); i += 3)
        {
            const uint32_t i0 = indices[i], i1 = indices[i + 1], i2 = indices[i + 2];

            const float3 e1 = verts[i1].position - verts[i0].position;
            const float3 e2 = verts[i2].position - verts[i0].position;

            
            const float3 faceN = simd::cross(e1, e2);
            nAcc[i0] += faceN;
            nAcc[i1] += faceN;
            nAcc[i2] += faceN;

            if (needTangents)
            {
                const float2 d1 = verts[i1].uv - verts[i0].uv;
                const float2 d2 = verts[i2].uv - verts[i0].uv;
                const float det = d1.x * d2.y - d2.x * d1.y;
                if (std::fabs(det) > 1e-20f)
                {
                    const float r = 1.0f / det;
                    const float3 t = (e1 * d2.y - e2 * d1.y) * r;
                    const float3 b = (e2 * d1.x - e1 * d2.x) * r;
                    tAcc[i0] += t;
                    tAcc[i1] += t;
                    tAcc[i2] += t;
                    bAcc[i0] += b;
                    bAcc[i1] += b;
                    bAcc[i2] += b;
                }
            }
        }

        for (size_t v = 0; v < n; v++)
        {
            Mesh_Vertices &mv = verts[v];

            if (needNormals)
            {
                const float len = simd::length(nAcc[v]);
                mv.normal = len > 1e-12f ? nAcc[v] / len : float3{0.0f, 1.0f, 0.0f};
            }

            if (needTangents)
            {
                const float3 nrm = mv.normal;

                
                float3 t = tAcc[v] - nrm * simd::dot(nrm, tAcc[v]);
                float tl = simd::length(t);
                if (tl < 1e-8f)
                {
                    
                    const float3 axis = std::fabs(nrm.x) < 0.9f ? float3{1.0f, 0.0f, 0.0f}
                                                                : float3{0.0f, 1.0f, 0.0f};
                    t = axis - nrm * simd::dot(nrm, axis);
                    tl = simd::length(t);
                }
                t = t / tl;

                
                float3 b = simd::cross(nrm, t);
                if (simd::dot(b, bAcc[v]) < 0.0f)
                    b = -b;

                mv.tangent = t;
                mv.bitangent = b;
            }
        }
    }
} 

Model::Model(const std::string &filePath, MTL::Device *metalDevice, DeletionQueue &dq)
{

    ufbx_load_opts opts = {};
    ufbx_error error;
    ufbx_scene *scene = ufbx_load_file(filePath.c_str(), &opts, &error);
    if (!scene)
    {
        fprintf(stderr, "ufbx load error: %s\n", error.description.data);
        assert(scene);
    }

    const std::filesystem::path modelDir = std::filesystem::path(filePath).parent_path();

    
    std::unordered_map<const ufbx_texture *, MTL::Texture *> textureCache;

    auto loadTexture = [&](const ufbx_texture *ut) -> MTL::Texture *
    {
        auto it = textureCache.find(ut);
        if (it != textureCache.end())
            return it->second;

        MTL::Texture *result = nullptr;

        if (ut->content.data && ut->content.size > 0)
        {
            
            Texture *t = new Texture((stbi_uc *)ut->content.data, ut->content.size, metalDevice);
            result = t->texture;
        }
        else
        {
            
            const std::filesystem::path candidates[] = {
                modelDir / std::filesystem::path(ut->relative_filename.data),
                std::filesystem::path(ut->absolute_filename.data),
                modelDir / std::filesystem::path(ut->filename.data).filename()};

            for (const auto &c : candidates)
            {
                if (!c.empty() && std::filesystem::is_regular_file(c))
                {
                    const std::string pathStr = c.string();
                    Texture *t = new Texture(pathStr.c_str(), metalDevice);
                    result = t->texture;
                    break;
                }
            }

            if (!result)
                fprintf(stderr, "Model: couldn't find texture '%s'\n", ut->filename.data);
        }

        textureCache[ut] = result;
        return result;
    };

    std::vector<uint32_t> tri_indices;

    for (ufbx_mesh *mesh : scene->meshes)
    {
        tri_indices.resize(mesh->max_face_triangles * 3);

        
        
        
        
        for (const auto &mat : mesh->materials)
        {
            if (PBRmaterials_map.find(mat->name.data) != PBRmaterials_map.end())
                continue;

            PBRMaterial pbrmat{}; 

            for (const auto &tex : mat->textures)
            {
                MTL::Texture *mtlTex = loadTexture(tex.texture);
                if (!mtlTex)
                    continue;

                const char *texName = tex.texture->element.name.data;
                if (strcmp(texName, "base_color_texture") == 0)
                    pbrmat.base_color_texture = mtlTex;
                else if (strcmp(texName, "normalmap_texture") == 0)
                    pbrmat.normalmap_texture = mtlTex;
                else if (strcmp(texName, "metallic_texture") == 0)
                    pbrmat.metallic_texture = mtlTex;
                else if (strcmp(texName, "roughness_texture") == 0)
                    pbrmat.roughness_texture = mtlTex;
                else if (strcmp(texName, "specular_texture") == 0)
                    pbrmat.specular_texture = mtlTex;
            }

            PBRmaterials_map[mat->name.data] = pbrmat;
        }

        for (ufbx_mesh_part &part : mesh->material_parts)
        {
            ufbx_material *material = NULL;
            if (part.index < mesh->materials.count)
                material = mesh->materials.data[part.index];
            else
                continue;

            std::vector<Mesh_Vertices> meshV;
            meshV.reserve(part.num_triangles * 3);

            const bool hasNormal = mesh->vertex_normal.exists;
            const bool hasTangent = mesh->vertex_tangent.exists;
            const bool hasBitangent = mesh->vertex_bitangent.exists;
            const bool hasUV = mesh->vertex_uv.exists;

            for (uint32_t face_index : part.face_indices)
            {
                ufbx_face face = mesh->faces[face_index];
                uint32_t num_tris = ufbx_triangulate_face(
                    tri_indices.data(), tri_indices.size(), mesh, face);
                for (size_t i = 0; i < num_tris * 3; i++)
                {
                    uint32_t index = tri_indices[i];

                    
                    
                    Mesh_Vertices mv{};
                    std::memset(&mv, 0, sizeof(mv));

                    ufbx_vec3 pos = mesh->vertex_position[index];
                    mv.position = float3{(float)pos.x, (float)pos.y, (float)pos.z};

                    if (hasNormal)
                    {
                        ufbx_vec3 n = mesh->vertex_normal[index];
                        mv.normal = float3{(float)n.x, (float)n.y, (float)n.z};
                    }
                    if (hasTangent)
                    {
                        ufbx_vec3 t = mesh->vertex_tangent[index];
                        mv.tangent = float3{(float)t.x, (float)t.y, (float)t.z};
                    }
                    if (hasBitangent)
                    {
                        ufbx_vec3 b = mesh->vertex_bitangent[index];
                        mv.bitangent = float3{(float)b.x, (float)b.y, (float)b.z};
                    }
                    if (hasUV)
                    {
                        ufbx_vec2 uv = mesh->vertex_uv[index];
                        mv.uv = float2{(float)uv.x, (float)uv.y};
                    }

                    meshV.emplace_back(mv);
                }
            }

            ufbx_vertex_stream streams[1] = {
                {meshV.data(), meshV.size(), sizeof(Mesh_Vertices)},
            };
            std::vector<uint32_t> indices;
            indices.resize(part.num_triangles * 3);
            ufbx_error genErr;
            size_t num_vertices = ufbx_generate_indices(streams, 1, indices.data(), indices.size(), nullptr, &genErr);
            if (num_vertices == 0)
            {
                std::cerr << "ufbx_generate_indices failed: " << genErr.description.data << "\n";
                continue; 
            }
            meshV.resize(num_vertices);

            
            const bool needNormals = !hasNormal;
            const bool needTangents = hasUV && !(hasTangent && hasBitangent);
            if (needNormals || needTangents)
                GenerateMissingTangentSpace(meshV, indices, needNormals, needTangents);

            Mesh *meshy = new Mesh(meshV, std::string(material->name.data), indices, metalDevice, dq);
            meshes.emplace_back(std::move(meshy));
        }
    }

    ufbx_free_scene(scene);
}

void Model::UpdateShaders(const MTL::Library *lib, DeletionQueue &dq, MTL4::Compiler *metal4Complier, const MTL::PixelFormat &pf)
{
    for (auto const &i : meshes){
        i->UpdateShaders(lib, dq, metal4Complier, pf);
    }
}

void Model::UpdateResidency(MTL::ResidencySet *residency_set)
{
    for (auto const &i : meshes){
        i->UpdateResidency(residency_set);
    }
}

void Model::Draw(MTL4::RenderCommandEncoder *encoder, MESHMVP & mvp)
{
    for (auto const &i : meshes){
        i->Draw(encoder, PBRmaterials_map[i->material_name],mvp);
    }
}