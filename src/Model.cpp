#include "Model.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>

static Texture *MakeTexture(const ufbx_texture *t, MTL::Device *device, bool srgb)
{
    if (!t)
        return nullptr;

    if (!t->content.data || t->content.size == 0)
    {
        std::cerr << "Texture '" << t->element.name.data << "' has no pixel data (file: "
                  << t->filename.data << ")\n";
        return nullptr;
    }

    Texture *tex = new Texture((unsigned char*)t->content.data, t->content.size, device);
    if (!tex->texture)
    {
        delete tex;
        return nullptr;
    }
    return tex;
}

Model::Model(const std::string &filePath, MTL::Device *metalDevice, DeletionQueue &dq)
{
    ufbx_load_opts opts = {};
    opts.load_external_files = true;
    opts.ignore_missing_external_files = true;

    ufbx_error error;
    ufbx_scene *scene = ufbx_load_file(filePath.c_str(), &opts, &error);
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
            vertices.reserve(part.num_triangles * 3);
            std::vector<uint32_t> tri_indices;
            tri_indices.resize(mesh->max_face_triangles * 3);

            ufbx_material *material = nullptr;
            std::string mat_name;

            if (part.index < mesh->materials.count)
            {
                material = mesh->materials.data[part.index];
                mat_name = material ? material->name.data : "";
            }

            if (pbr_material_mapping.find(mat_name) == pbr_material_mapping.end())
            {
                if (material)
                    ExtractTextures(material, metalDevice);
                else
                    pbr_material_mapping[mat_name] = new PBR_Mat();
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

            meshy->SetMaterial(pbr_material_mapping[mat_name]);
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

        auto it = pbr_material_mapping.find(i->material_name);
        i->Draw(encoder, mvp, it != pbr_material_mapping.end() ? it->second : nullptr);
    }
}

void Model::ExtractTextures(ufbx_material *mat, MTL::Device *metalDevice)
{

    PBR_Mat *pbr_mat = new PBR_Mat();

    auto pick = [&](const char *slotName, const ufbx_material_map &fallback) -> const ufbx_texture *
    {
        for (const auto &tex : mat->textures)
        {
            if (tex.texture && std::string(tex.texture->element.name.data) == slotName)
                return tex.texture;
        }
        return fallback.texture;
    };

    pbr_mat->base_color_texture = MakeTexture(pick("base_color_texture", mat->pbr.base_color), metalDevice, true);
    pbr_mat->normalmap_texture = MakeTexture(pick("normalmap_texture", mat->pbr.normal_map), metalDevice, false);
    pbr_mat->metallic_texture = MakeTexture(pick("metallic_texture", mat->pbr.metalness), metalDevice, false);
    pbr_mat->roughness_texture = MakeTexture(pick("roughness_texture", mat->pbr.roughness), metalDevice, false);
    pbr_mat->specular_texture = MakeTexture(pick("specular_texture", mat->pbr.specular_color), metalDevice, false);

    std::cout << "Material '" << mat->name.data << "': base="
              << (pbr_mat->base_color_texture != nullptr) << " normal="
              << (pbr_mat->normalmap_texture != nullptr) << " metallic="
              << (pbr_mat->metallic_texture != nullptr) << " roughness="
              << (pbr_mat->roughness_texture != nullptr) << " specular="
              << (pbr_mat->specular_texture != nullptr) << "\n";

    pbr_material_mapping[mat->name.data] = pbr_mat;
}