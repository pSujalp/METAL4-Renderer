#include "Model.h"
#include <cmath>
#include <cstring>
#include <filesystem>
#include <unordered_map>

Model::Model(const std::string &filePath, MTL::Device *metalDevice, DeletionQueue &dq)
{

    ufbx_scene *scene = ufbx_load_file(filePath.c_str(), NULL, NULL);
    assert(scene);

    for (ufbx_mesh *mesh : scene->meshes)
    {

        for (ufbx_mesh_part &part : mesh->material_parts)
        {
            std::vector<VertexData> vertices;
            std::vector<uint32_t> tri_indices;
            tri_indices.resize(mesh->max_face_triangles * 3);

            ufbx_material *material = NULL;

            if (part.index < mesh->materials.count) {
                material = mesh->materials.data[part.index];
            }

            std::string mat_name = material ? material->name.data : "";

            for (uint32_t face_index : part.face_indices)
            {
                ufbx_face face = mesh->faces[face_index];

                
                uint32_t num_tris = ufbx_triangulate_face(
                    tri_indices.data(), tri_indices.size(), mesh, face);
                for (size_t i = 0; i < num_tris * 3; i++)
                {
                    uint32_t index = tri_indices[i];

                    VertexData v;
                    v.position = float4{(float)mesh->vertex_position[index].x, (float)mesh->vertex_position[index].y,
                                        (float)mesh->vertex_position[index].z, 1.0f};

                    v.textureCoordinate = {(float)mesh->vertex_uv[index].x, (float)mesh->vertex_uv[index].y};
                    vertices.push_back(v);
                }
            }
            ufbx_vertex_stream streams[1] = {
                {vertices.data(), vertices.size(), sizeof(VertexData)},
            };
            std::vector<uint32_t> indices;
            indices.resize(part.num_triangles * 3);

            size_t num_vertices = ufbx_generate_indices(streams, 1, indices.data(), indices.size(), nullptr, nullptr);
            vertices.resize(num_vertices);

            Mesh * meshy = new Mesh();
            meshy = new Mesh(vertices,mat_name,indices,metalDevice,dq);
            meshes.emplace_back(meshy);

            
        }
    }
}

void Model::UpdateShaders(const MTL::Library *lib, DeletionQueue &dq, MTL4::Compiler *metal4Complier, const MTL::PixelFormat &pf)
{
    for (auto const &i : meshes)
    {
        i->UpdateShaders(lib, dq, metal4Complier, pf);
    }
}

void Model::UpdateResidency(MTL::ResidencySet *residency_set)
{
    for (auto const &i : meshes)
    {
        i->UpdateResidency(residency_set);
    }
}

void Model::Draw(MTL4::RenderCommandEncoder *encoder, MESHMVP &mvp)
{
    for (auto const &i : meshes)
    {
        i->Draw(encoder, mvp);
    }
}