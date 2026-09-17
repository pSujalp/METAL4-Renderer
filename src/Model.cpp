#include "Model.h"
Model::Model(const std::string &filePath, MTL::Device *metalDevice)
{

    ufbx_load_opts opts = {};
    ufbx_error error;
    ufbx_scene *scene = ufbx_load_file(filePath.c_str(), &opts, &error);
    if (!scene)
    {
        fprintf(stderr, "ufbx load error: %s\n", error.description.data);
        assert(scene);
    }

    std::vector<uint32_t> tri_indices;
    

    for (ufbx_mesh *mesh : scene->meshes)
    {
        tri_indices.resize(mesh->max_face_triangles * 3);

        for (ufbx_mesh_part &part : mesh->material_parts)
        {
            ufbx_material *material = NULL;
            if (part.index < mesh->materials.count)
            {
                material = mesh->materials.data[part.index];
                const ufbx_material_list materiallist = mesh->materials;
                printf("size of material list --- >%zu \n", materiallist.count);
                for (const auto &mat : materiallist)
                {
                    if(PBRmaterials_map.contains(mat->name.data)) continue;
                    const ufbx_material_texture_list materiallist_textures = mat->textures;
                    PBRMaterial pbrmat;
                    for (const auto &tex : materiallist_textures)
                    {
                        if (tex.texture->content.data && tex.texture->content.size > 0)
                        {
                            Texture *texture = new Texture((stbi_uc *)tex.texture->content.data, tex.texture->content.size, metalDevice);
                            if (strcmp(tex.texture->element.name.data, "base_color_texture") == 0)
                                pbrmat.base_color_texture = texture->texture;
                            else if (strcmp(tex.texture->element.name.data, "normalmap_texture") == 0)
                                pbrmat.normalmap_texture = texture->texture;
                            else if (strcmp(tex.texture->element.name.data, "metallic_texture") == 0)
                                pbrmat.metallic_texture = texture->texture;
                            else if (strcmp(tex.texture->element.name.data, "roughness_texture") == 0)
                                pbrmat.roughness_texture = texture->texture;
                            else if (strcmp(tex.texture->element.name.data, "specular_texture") == 0)
                                pbrmat.specular_texture = texture->texture;
                        }
                        PBRmaterials_map[mat->name.data] = std::move(pbrmat);
                    }
                }
            }
            else
            {
                continue;
            }

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

                    ufbx_vec3 pos = mesh->vertex_position[index];
                    mv.position = {(float)pos.x, (float)pos.y, (float)pos.z};

                    if (hasNormal)
                    {
                        ufbx_vec3 n = mesh->vertex_normal[index];
                        mv.normal = {(float)n.x, (float)n.y, (float)n.z};
                    }
                    if (hasTangent)
                    {
                        ufbx_vec3 t = mesh->vertex_tangent[index];
                        mv.tangent = {(float)t.x, (float)t.y, (float)t.z};
                    }
                    if (hasBitangent)
                    {
                        ufbx_vec3 b = mesh->vertex_bitangent[index];
                        mv.bitangent = {(float)b.x, (float)b.y, (float)b.z};
                    }
                    if (hasUV)
                    {
                        ufbx_vec2 uv = mesh->vertex_uv[index];
                        mv.uv = {(float)uv.x, (float)uv.y};
                    }

                    meshV.emplace_back(mv);
                }
            }
            ufbx_vertex_stream streams[1] = {
                {meshV.data(), meshV.size(), sizeof(Mesh_Vertices)},
            };
            std::vector<uint32_t> indices;
            indices.resize(part.num_triangles * 3);
            size_t num_vertices = ufbx_generate_indices(
                streams, 1, indices.data(), indices.size(), nullptr, nullptr);
            meshV.resize(num_vertices);

            Mesh *meshy = new Mesh(meshV, std::string(material->name.data), indices, metalDevice);
            meshes.emplace_back(std::move(meshy));
        }
    }

    ufbx_free_scene(scene);
}