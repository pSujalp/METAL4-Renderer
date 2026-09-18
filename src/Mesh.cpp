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

        dq.push_function([=]()
                         {
            if (Mesharg_table) Mesharg_table->release(); }
        );


}


 void Mesh::UpdateShaders(const MTL::Library *lib, DeletionQueue &dq, MTL4::Compiler *metal4Complier, const MTL::PixelFormat &pf)
    {
        using NS::StringEncoding::UTF8StringEncoding;
        ShaderFunctionDescriptor skyboxShaderFunctionDescriptor(lib, "skyboxVertex", "skyboxFragment");
        auto *skyboxPipelineDescriptor = MTL4::RenderPipelineDescriptor::alloc()->init();
        skyboxPipelineDescriptor->setLabel(NS::String::string("Skybox", NS::ASCIIStringEncoding));
        skyboxPipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(pf);
        skyboxPipelineDescriptor->setVertexFunctionDescriptor(skyboxShaderFunctionDescriptor.vertexShaderFunctionDescriptor);
        skyboxPipelineDescriptor->setFragmentFunctionDescriptor(skyboxShaderFunctionDescriptor.fragmentShaderFunctionDescriptor);
        NS::Error *pPipelineError = nullptr;
        MeshPSO = metal4Complier->newRenderPipelineState(skyboxPipelineDescriptor, (MTL4::CompilerTaskOptions *)nullptr, &pPipelineError);
        if (!MeshPSO)
        {
            if (pPipelineError)
            {
                std::cerr << "Pipeline compile error: " << pPipelineError->localizedDescription()->utf8String() << std::endl;
            }
            else
            {
                std::cerr << "newRenderPipelineState() returned null (no error object provided).\n";
            }
            exit(EXIT_FAILURE);
        }
        skyboxPipelineDescriptor->release();


    }



void Mesh::UpdateResidency(MTL::ResidencySet *residency_set)
    {
        residency_set->addAllocation(this->Mesh_Data);
        residency_set->addAllocation(this->MVP_Data);
        residency_set->addAllocation(this->material_Data);
    }


void Mesh::Draw(MTL4::RenderCommandEncoder *encoder, PBRMaterial &pbr_mat, MESHMVP & mvp)
    {


        memcpy(material_Data->contents(), &pbr_mat, sizeof(PBRMaterial));
        memcpy(MVP_Data->contents(), &mvp, sizeof(MESHMVP));

        encoder->setRenderPipelineState(MeshPSO);
        encoder->setDepthStencilState(MeshDepthStencilState);
        encoder->setArgumentTable(Mesharg_table, MTL::RenderStageVertex);
        encoder->setArgumentTable(Mesharg_table, MTL::RenderStageFragment);
        encoder->drawIndexedPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle, indexCount,
             MTL::IndexType::IndexTypeUInt32,index_Data->gpuAddress(), 
             sizeof(uint32_t) * this->indexCount);
    }