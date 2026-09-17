#include "Mesh.h"

Mesh::Mesh(const std::vector<Vertex>& position,
           const std::vector<Vertex>& normal,
           const std::vector<Vertex>& indices,
           const std::vector<Vertex>& tangent,
           const std::vector<Vertex>& bitangent,
           const std::vector<UV> &uv,  MTL::Device *metalDevice,
           const std::string &material)
{

    this->metalDevice = metalDevice;
    this->material = material;
    unsigned long BufferSize = sizeof(Vertex) * position.size();
    vertexBuffer = metalDevice->newBuffer(position.data(), BufferSize, MTL::ResourceStorageModeShared);
    indexCount = indices.size();

    BufferSize = sizeof(uint32_t) * indices.size();
    indexBuffer = device->newBuffer(indices.data(), BufferSize, MTL::ResourceStorageModeShared);

    BufferSize = sizeof(UV) * uv.size();

    uvBuffer = device->newBuffer(uv.data(), BufferSize, MTL::ResourceStorageModeShared);

    BufferSize = sizeof(Vertex) * tangent.size();
    tanBuffer = device->newBuffer(tangent.data(), BufferSize, MTL::ResourceStorageModeManaged);

    BufferSize = sizeof(Vertex) * bitangent.size();
    tanBuffer = device->newBuffer(bitangent.data(), BufferSize, MTL::ResourceStorageModeManaged);
}
