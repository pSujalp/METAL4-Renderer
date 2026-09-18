#include "Model.h"




void create_vertex_buffer(const void *vertices, size_t count)
{
    printf(".. %zu vertices\n", count);
}

void create_index_buffer(const uint32_t *indices, size_t count)
{
    printf(".. %zu indices\n", count);
}

void Model::convert_mesh_part(ufbx_mesh *mesh, ufbx_mesh_part *part)
{
    std::vector<VertexData> vertices;
    std::vector<uint32_t> tri_indices;
    tri_indices.resize(mesh->max_face_triangles * 3);

    for (uint32_t face_index : part->face_indices)
    {
        ufbx_face face = mesh->faces[face_index];

        uint32_t num_tris = ufbx_triangulate_face(
            tri_indices.data(), tri_indices.size(), mesh, face);

        for (size_t i = 0; i < num_tris * 3; i++)
        {
            uint32_t index = tri_indices[i];

            VertexData v;

            v.position = {(float)mesh->vertex_position[index].x , (float)mesh->vertex_position[index].y,(float) mesh->vertex_position[index].z };
            v.textureCoordinate = {(float)mesh->vertex_uv[index].x ,(float)mesh->vertex_uv[index].y};
            vertices.push_back(v);
        }
    }

    assert(vertices.size() == part->num_triangles * 3);

    ufbx_vertex_stream streams[1] = {
        {vertices.data(), vertices.size(), sizeof(VertexData)},
    };
    std::vector<uint32_t> indices;
    indices.resize(part->num_triangles * 3);

    size_t num_vertices = ufbx_generate_indices(
        streams, 1, indices.data(), indices.size(), nullptr, nullptr);

    vertices.resize(num_vertices);

    Mesh * meshy = new Mesh();

    meshy->vertexBufferdata = metalDevice->newBuffer(vertices.data(),vertices.size() * sizeof(VertexData), MTL::ResourceStorageModeManaged);
    meshy->indexBufferData = metalDevice->newBuffer(indices.data(), indices.size() * sizeof(uint32_t), MTL::ResourceStorageModeManaged);

    meshy->vertices = vertices;
    meshy->indices = indices;
    meshes.emplace_back(meshy);


}


Model::Model(std::string filepath, MTL::Device * metalDevice){

    this->metalDevice = metalDevice;
    ufbx_scene *scene = ufbx_load_file(filepath.c_str(), NULL, NULL);
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
            }
            convert_mesh_part(mesh, &part);
        }
    }
}