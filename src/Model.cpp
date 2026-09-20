#include "Model.h"
#include <cmath>
#include <cstring>
#include <iostream>

Model::Model(const std::string &filePath, MTL::Device *metalDevice, DeletionQueue &dq)
{
    ufbx_error error;
    ufbx_scene *scene = ufbx_load_file(filePath.c_str(), NULL, &error);
    if (!scene)
    {
        fprintf(stderr, "ufbx load error: %s\n", error.description.data);
        assert(scene);
    }

    for (ufbx_mesh *mesh : scene->meshes)
    {
        // Reading mesh->vertex_uv[...] when the mesh has no UVs dereferences a null
        // index array and segfaults, so check first.
        const bool hasUV = mesh->vertex_uv.exists;

        for (ufbx_mesh_part &part : mesh->material_parts)
        {
            if (part.num_triangles == 0)
                continue;

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

                    // Zero everything (including padding): ufbx_generate_indices compares raw bytes.
                    VertexData v;
                    std::memset(&v, 0, sizeof(v));

                    const ufbx_vec3 p = mesh->vertex_position[index];
                    v.position = float4{(float)p.x, (float)p.y, (float)p.z, 1.0f};

                    if (hasUV)
                    {
                        const ufbx_vec2 uv = mesh->vertex_uv[index];
                        v.textureCoordinate = {(float)uv.x, (float)uv.y};
                    }
                    vertices.push_back(v);
                }
            }
            ufbx_vertex_stream streams[1] = {
                {vertices.data(), vertices.size(), sizeof(VertexData)},
            };
            std::vector<uint32_t> indices;
            indices.resize(part.num_triangles * 3);

            ufbx_error genErr;
            size_t num_vertices = ufbx_generate_indices(streams, 1, indices.data(), indices.size(), nullptr, &genErr);
            if (num_vertices == 0)
            {
                // Building a Mesh from zero vertices gives a null MTL::Buffer later on.
                std::cerr << "ufbx_generate_indices failed: " << genErr.description.data << "\n";
                continue;
            }
            vertices.resize(num_vertices);

            // (The extra `new Mesh()` before this leaked an object; removed.)
            Mesh *meshy = new Mesh(vertices, mat_name, indices, metalDevice, dq);
            meshes.emplace_back(meshy);
        }
    }

    ufbx_free_scene(scene);
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