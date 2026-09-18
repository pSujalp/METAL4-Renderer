#include "Mesh.h"

Mesh::Mesh(std::vector<Mesh_Vertices> &meshv ,const std::string &material_name, 
    const std::vector<uint32_t> &indices,MTL::Device * metalDevice, DeletionQueue &dq){

    this->metalDevice = metalDevice;
    this->material_name = material_name;
    this->indexCount = indices.size();
    Mesh_Data = metalDevice->newBuffer(meshv.data(), sizeof(Mesh_Vertices) * meshv.size(), MTL::StorageModeShared);
    material_Data = metalDevice->newBuffer(sizeof(PBRMaterial), MTL::StorageModeShared);
    index_Data = metalDevice->newBuffer(indices.data(), sizeof(uint32_t) * this->indexCount, MTL::StorageModeShared);
    MVP_Data = metalDevice->newBuffer(sizeof(MESHMVP), MTL::StorageModeShared);


    MTL::DepthStencilDescriptor *depthDesc = MTL::DepthStencilDescriptor::alloc()->init();
        depthDesc->setDepthCompareFunction(MTL::CompareFunctionLessEqual);
        depthDesc->setDepthWriteEnabled(false);
        MeshDepthStencilState = metalDevice->newDepthStencilState(depthDesc);
        depthDesc->release();

        auto *argTableDesc = MTL4::ArgumentTableDescriptor::alloc()->init();
        argTableDesc->setMaxBufferBindCount(magic_enum::enum_count<MESH_BUFFER_INDEX>());
        argTableDesc->setMaxTextureBindCount(magic_enum::enum_count<MESH_MAT_INDEX>());
        Mesharg_table = metalDevice->newArgumentTable(argTableDesc, nullptr);
        argTableDesc->release();
        if (!Mesharg_table)
        {
            std::cerr << "newArgumentTable() returned null.\n";
            exit(EXIT_FAILURE);
        }

        Mesharg_table->setAddress(Mesh_Data->gpuAddress(), (NS::UInteger)(MESH_BUFFER_INDEX::MESH_VERTEX_DATA));
        Mesharg_table->setAddress(material_Data->gpuAddress(), (NS::UInteger)(MESH_MAT_INDEX::PBR_MAT));
        Mesharg_table->setAddress(MVP_Data->gpuAddress(), (NS::UInteger) (MESH_BUFFER_INDEX::MVP_DATA));

        


}



