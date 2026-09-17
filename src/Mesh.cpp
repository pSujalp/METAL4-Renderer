#include "Mesh.h"

Mesh::Mesh(std::vector<Mesh_Vertices> &meshv ,const std::string &material_name, 
    const std::vector<uint32_t> &indices,MTL::Device * metalDevice){

    this->metalDevice = metalDevice;
    this->material_name = material_name;
    this->indexCount = indices.size();
    Mesh_Data = metalDevice->newBuffer(meshv.data(), sizeof(Mesh_Vertices) * meshv.size(), MTL::StorageModeShared);
    index_Data = metalDevice->newBuffer(indices.data(), sizeof(uint32_t) * this->indexCount, MTL::StorageModeShared);


    
}



