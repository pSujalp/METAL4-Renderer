#include "Model.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>

Model::Model(const std::string &filePath, MTL::Device *metalDevice, DeletionQueue &dq)
{
    ufbx_error error;
    ufbx_scene *scene = ufbx_load_file(filePath.c_str(), NULL, &error);
    if (!scene)
    {
        fprintf(stderr, "ufbx load error: %s\n", error.description.data);
        return;
    }

    for (ufbx_node *node : scene->nodes)
    {
        ufbx_mesh *mesh = node->mesh;
        if (!mesh)
            continue;

        const bool hasUV = mesh->vertex_uv.exists;

        for (ufbx_mesh_part &part : mesh->material_parts)
        {
            if (part.num_triangles == 0)
                continue;

            std::vector<VertexData> vertices;
            std::vector<uint32_t> tri_indices;
            tri_indices.resize(mesh->max_face_triangles * 3);

            ufbx_material *material = NULL;

            std::string mat_name;


            if (part.index < mesh->materials.count)
            {
                material = mesh->materials.data[part.index];
                mat_name = material ? material->name.data : "";
                if(pbr_material_mapping.find(mat_name) == pbr_material_mapping.end()){
                    ExtractTextures(material,metalDevice);
                }
                
            }

            


            for (uint32_t face_index : part.face_indices)
            {
                ufbx_face face = mesh->faces[face_index];

                uint32_t num_tris = ufbx_triangulate_face(
                    tri_indices.data(), tri_indices.size(), mesh, face);
                for (size_t i = 0; i < num_tris * 3; i++)
                {
                    uint32_t index = tri_indices[i];

                    VertexData v;
                    std::memset(&v, 0, sizeof(v));

                    ufbx_vec3 p = ufbx_transform_position(&node->geometry_to_world, mesh->vertex_position[index]);
                    v.position = {(float)p.x, (float)p.y, (float)p.z, 1.0f};

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
                std::cerr << "ufbx_generate_indices failed: " << genErr.description.data << "\n";
                continue;
            }
            vertices.resize(num_vertices);

            Mesh *meshy = new Mesh(vertices, mat_name, indices, metalDevice, dq);

            meshy->base_color_texture = pbr_material_mapping[mat_name]->base_color_texture;
            meshy->normalmap_texture = pbr_material_mapping[mat_name]->normalmap_texture;
            meshy->specular_texture = pbr_material_mapping[mat_name]->specular_texture;
            meshy->metallic_texture = pbr_material_mapping[mat_name]->metallic_texture;
            meshy->roughness_texture = pbr_material_mapping[mat_name]->roughness_texture;
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
        i->Draw(encoder, mvp, pbr_material_mapping[i->material_name]);
    }
}

void Model::ExtractTextures(ufbx_material *mat, MTL::Device * metalDevice)
{
    const ufbx_material_texture_list textures = mat->textures;
    for (const auto &tex : textures)
    {
        PBR_Mat * pbr_mat = new PBR_Mat();
        if (tex.texture->content.data && tex.texture->content.size > 0)
        {
            std::cout << tex.texture->element.name.data << std::endl;

            if(tex.texture->element.name.data == "base_color_texture")
            pbr_mat->base_color_texture = new Texture((stbi_uc*)tex.texture->content.data,tex.texture->content.size,metalDevice);

            if(tex.texture->element.name.data == "normalmap_texture")
            pbr_mat->normalmap_texture = new Texture((stbi_uc*)tex.texture->content.data,tex.texture->content.size,metalDevice);

            if(tex.texture->element.name.data == "metallic_texture")
            pbr_mat->metallic_texture = new Texture((stbi_uc*)tex.texture->content.data,tex.texture->content.size,metalDevice);

            if(tex.texture->element.name.data == "roughness_texture")
            pbr_mat->roughness_texture = new Texture((stbi_uc*)tex.texture->content.data,tex.texture->content.size,metalDevice);

            if(tex.texture->element.name.data == "specular_texture")
            pbr_mat->specular_texture = new Texture((stbi_uc*)tex.texture->content.data,tex.texture->content.size,metalDevice);
        }
        pbr_material_mapping[mat->name.data] = std::move(pbr_mat);
    }
}