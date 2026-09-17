#include "Model.h"


Model::Model(const std::string & filePath, MTL::Device * metalDevice){

    ufbx_scene *scene = ufbx_load_file(filePath.c_str(), NULL, NULL);
    assert(scene);


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
                const ufbx_material_texture_list textures = material->textures;
                PBRMaterial pbrmat;
                for (const auto &tex : textures)
                {
                    if (tex.texture->content.data && tex.texture->content.size > 0){
                        Texture * texture = new Texture((stbi_uc*)tex.texture->content.data,tex.texture->content.size,  metalDevice); 
                        if(strcmp(tex.texture->element.name.data , "base_color_texture")==0) pbrmat.base_color_texture = texture->texture;
                        else if(strcmp(tex.texture->element.name.data , "normalmap_texture")==0) pbrmat.normalmap_texture = texture->texture;
                        else if(strcmp(tex.texture->element.name.data , "metallic_texture")==0) pbrmat.metallic_texture = texture->texture;
                        else if(strcmp(tex.texture->element.name.data , "roughness_texture")==0) pbrmat.roughness_texture = texture->texture;
                        else if(strcmp(tex.texture->element.name.data , "specular_texture")==0) pbrmat.specular_texture = texture->texture;
                    }
                }
                PBRmaterials_map[material->name.data] = std::move(pbrmat);
            }

            std::vector<Mesh_Vertices> meshV;
            meshV.reserve(part.num_triangles * 3);

            for (uint32_t face_index : part.face_indices)
            {
                ufbx_face face = mesh->faces[face_index];
                uint32_t num_tris = ufbx_triangulate_face(
                    tri_indices.data(), tri_indices.size(), mesh, face);
                for (size_t i = 0; i < num_tris * 3; i++)
                {
                    uint32_t index = tri_indices[i];
                    Mesh_Vertices mv{};

                    mv.position.x = mesh->vertex_position[index].x;
                    mv.position.y = mesh->vertex_position[index].y;
                    mv.position.z = mesh->vertex_position[index].z;
                    
                    mv.normal.x = mesh->vertex_normal[index].x;
                    mv.normal.y = mesh->vertex_normal[index].y;
                    mv.normal.z = mesh->vertex_normal[index].z;
                    
                    mv.tangent.x = mesh->vertex_tangent[index].x;
                    mv.tangent.y = mesh->vertex_tangent[index].y;
                    mv.tangent.z = mesh->vertex_tangent[index].z;
        
                    mv.bitangent.x = mesh->vertex_bitangent[index].x;
                    mv.bitangent.y = mesh->vertex_bitangent[index].y;
                    mv.bitangent.z = mesh->vertex_bitangent[index].z;
                    
                    mv.uv.U = mesh->vertex_uv[index].x;
                    mv.uv.V = mesh->vertex_uv[index].y;
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

            Mesh * meshy = new Mesh(meshV,std::string(material->name.data),indices,metalDevice);

            

            
            


        
            
        }

    }



    


}