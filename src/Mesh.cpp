#include "Mesh.h"

Mesh::Mesh(std::vector<VertexData> &meshv, const std::string &material_name,
           const std::vector<uint32_t> &indices, MTL::Device *metalDevice, DeletionQueue &dq)
{

    this->metalDevice = metalDevice;
    this->material_name = material_name;
    this->indexCount = indices.size();

    Mesh_Data = metalDevice->newBuffer(meshv.data(), sizeof(VertexData) * meshv.size(), MTL::StorageModeShared);
    
    index_Data = metalDevice->newBuffer(indices.data(), sizeof(uint32_t) * indices.size(), MTL::StorageModeShared);
    MVP_Data = metalDevice->newBuffer(sizeof(MESHMVP), MTL::StorageModeShared);

    if (!Mesh_Data  || !index_Data || !MVP_Data)
    {
        std::cerr << "newBuffer() returned null.\n";
        exit(EXIT_FAILURE);
    }

    MTL::DepthStencilDescriptor *depthDesc = MTL::DepthStencilDescriptor::alloc()->init();
    depthDesc->setDepthCompareFunction(MTL::CompareFunctionLess);
    depthDesc->setDepthWriteEnabled(true);
    MeshDepthStencilState = metalDevice->newDepthStencilState(depthDesc);
    depthDesc->release();

    auto *argTableDesc = MTL4::ArgumentTableDescriptor::alloc()->init();
    argTableDesc->setMaxBufferBindCount(magic_enum::enum_count<MESH_BUFFER_INDEX>());
    argTableDesc->setMaxTextureBindCount((NS::UInteger)5);
    Mesharg_table = metalDevice->newArgumentTable(argTableDesc, nullptr);
    argTableDesc->release();
    if (!Mesharg_table)
    {
        std::cerr << "newArgumentTable() returned null.\n";
        exit(EXIT_FAILURE);
    }

    Mesharg_table->setAddress(Mesh_Data->gpuAddress(), (NS::UInteger)(MESH_BUFFER_INDEX::MESH_VERTEX_DATA));
    Mesharg_table->setAddress(MVP_Data->gpuAddress(), (NS::UInteger)(MESH_BUFFER_INDEX::MVP_DATA));

    dq.push_function([=]()
                     {
        if (Mesharg_table) Mesharg_table->release();
        if (MeshDepthStencilState) MeshDepthStencilState->release();
        if (MeshPSO) MeshPSO->release();
        if (Mesh_Data) Mesh_Data->release();
        if (index_Data) index_Data->release();
        if (MVP_Data) MVP_Data->release(); });
}
void Mesh::UpdateShaders(const MTL::Library *lib, DeletionQueue &dq, MTL4::Compiler *metal4Complier, const MTL::PixelFormat &pf)
{
    using NS::StringEncoding::UTF8StringEncoding;
    ShaderFunctionDescriptor modelShaderFunctionDescriptor(lib, "modelVertexShader", "modelFragmentShader");
    auto *ModePipelineDescriptor = MTL4::RenderPipelineDescriptor::alloc()->init();
    ModePipelineDescriptor->setLabel(NS::String::string("Model", NS::ASCIIStringEncoding));
    ModePipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(pf);
    ModePipelineDescriptor->setVertexFunctionDescriptor(modelShaderFunctionDescriptor.vertexShaderFunctionDescriptor);
    ModePipelineDescriptor->setFragmentFunctionDescriptor(modelShaderFunctionDescriptor.fragmentShaderFunctionDescriptor);
    NS::Error *pPipelineError = nullptr;
    MeshPSO = metal4Complier->newRenderPipelineState(ModePipelineDescriptor, (MTL4::CompilerTaskOptions *)nullptr, &pPipelineError);
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
    ModePipelineDescriptor->release();
}

void Mesh::UpdateResidency(MTL::ResidencySet *residency_set)
{
    residency_set->addAllocation(this->Mesh_Data);
    residency_set->addAllocation(this->MVP_Data);
    residency_set->addAllocation(this->index_Data);
}

void Mesh::Draw(MTL4::RenderCommandEncoder *encoder, MESHMVP &mvp)
{
    
    memcpy(MVP_Data->contents(), &mvp, sizeof(MESHMVP));

    encoder->setRenderPipelineState(MeshPSO);
    encoder->setDepthStencilState(MeshDepthStencilState);
    encoder->setArgumentTable(Mesharg_table, MTL::RenderStageVertex);
    encoder->setArgumentTable(Mesharg_table, MTL::RenderStageFragment);

    encoder->drawIndexedPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle, indexCount,
                               MTL::IndexType::IndexTypeUInt32, index_Data->gpuAddress(),
                               index_Data->length());
}