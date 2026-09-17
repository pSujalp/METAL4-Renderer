#include "Model.h"


Model::Model(const std::string & filePath, MTL::Device * metalDevice){

    ufbx_scene *scene = ufbx_load_file(filePath.c_str(), NULL, NULL);
    assert(scene);

    for (ufbx_mesh *mesh : scene->meshes)
    {
        printf("mesh '%s'\n", mesh->name.data);
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
                        if(tex.texture->element.name.data == "base_color_texture") pbrmat.base_color_texture = texture->texture;
                        else if(tex.texture->element.name.data == "normalmap_texture") pbrmat.normalmap_texture = texture->texture;
                        else if(tex.texture->element.name.data == "metallic_texture") pbrmat.metallic_texture = texture->texture;
                        else if(tex.texture->element.name.data == "roughness_texture") pbrmat.roughness_texture = texture->texture;
                        else if(tex.texture->element.name.data == "specular_texture") pbrmat.specular_texture = texture->texture;
                    }
                }
                PBRmaterials_map[material->name.data] = std::move(pbrmat);
            }


            
        }

    }



    


}