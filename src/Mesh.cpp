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

    if (!Mesh_Data || !index_Data || !MVP_Data)
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
    argTableDesc->setMaxTextureBindCount(magic_enum::enum_count<MESH_TEXTURE_INDEX>());
    Mesharg_table = metalDevice->newArgumentTable(argTableDesc, nullptr);
    argTableDesc->release();
    if (!Mesharg_table)
    {
        std::cerr << "newArgumentTable() returned null.\n";
        exit(EXIT_FAILURE);
    }

    Mesharg_table->setAddress(Mesh_Data->gpuAddress(), (NS::UInteger)(MESH_BUFFER_INDEX::MESH_VERTEX_DATA));
    Mesharg_table->setAddress(MVP_Data->gpuAddress(), (NS::UInteger)(MESH_BUFFER_INDEX::MVP_DATA));

    

    
    
    dq.push_function([argTable = Mesharg_table,
                      depthState = MeshDepthStencilState,
                      meshData = Mesh_Data,
                      indexData = index_Data,
                      mvpData = MVP_Data]()
                     {
        if (argTable) argTable->release();
        if (depthState) depthState->release();
        if (meshData) meshData->release();
        if (indexData) indexData->release();
        if (mvpData) mvpData->release(); });
}

void Mesh::UpdateShaders(const MTL::Library *lib, DeletionQueue &dq, MTL4::Compiler *metal4Complier, const MTL::PixelFormat &pf)
{
    ShaderFunctionDescriptor modelShaderFunctionDescriptor(lib, "modelVertexShader", "modelFragmentShader");

    auto *ModePipelineDescriptor = MTL4::RenderPipelineDescriptor::alloc()->init();
    ModePipelineDescriptor->setLabel(NS::String::string("Model", NS::ASCIIStringEncoding));
    ModePipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(pf);
    ModePipelineDescriptor->setVertexFunctionDescriptor(modelShaderFunctionDescriptor.vertexShaderFunctionDescriptor);
    ModePipelineDescriptor->setFragmentFunctionDescriptor(modelShaderFunctionDescriptor.fragmentShaderFunctionDescriptor);

    NS::Error *pPipelineError = nullptr;
    MTL::RenderPipelineState *newPSO = metal4Complier->newRenderPipelineState(
        ModePipelineDescriptor, (MTL4::CompilerTaskOptions *)nullptr, &pPipelineError);
    ModePipelineDescriptor->release();

    if (!newPSO)
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


    auto bind = [this](Texture *t, MESH_TEXTURE_INDEX idx)
    {
        if (t && t->texture)
            Mesharg_table->setTexture(t->texture->gpuResourceID(), (NS::UInteger)idx);
    };
    bind(base_color_texture, MESH_TEXTURE_INDEX::base_color_texture);
    bind(normalmap_texture,  MESH_TEXTURE_INDEX::normalmap_texture);
    bind(specular_texture,   MESH_TEXTURE_INDEX::specular_texture);
    bind(metallic_texture,   MESH_TEXTURE_INDEX::metallic_texture);
    bind(roughness_texture,  MESH_TEXTURE_INDEX::roughness_texture);

    
    
    
    MeshPSO = newPSO;
    dq.push_function([newPSO]()
                     { newPSO->release(); });
}



void Mesh::UpdateResidency(MTL::ResidencySet *residency_set)
{
    if (!residency_set)
        return;

    residency_set->addAllocation(Mesh_Data);
    residency_set->addAllocation(MVP_Data);
    residency_set->addAllocation(index_Data);

    
    for (Texture *t : {base_color_texture, normalmap_texture, specular_texture,
                       metallic_texture, roughness_texture})
    {
        if (t && t->texture)
            residency_set->addAllocation(t->texture);
    }
}

void Mesh::Draw(MTL4::RenderCommandEncoder *encoder, const MESHMVP &mvp, const PBR_Mat *pbr_mat)
{
    if (!MeshPSO)
    {
        std::cerr << "Mesh::Draw called before UpdateShaders().\n";
        return;
    }

    memcpy(MVP_Data->contents(), &mvp, sizeof(MESHMVP));

    


    encoder->setRenderPipelineState(MeshPSO);
    encoder->setDepthStencilState(MeshDepthStencilState);
    encoder->setArgumentTable(Mesharg_table, MTL::RenderStageVertex);
    encoder->setArgumentTable(Mesharg_table, MTL::RenderStageFragment);
    encoder->drawIndexedPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle, indexCount,
                                   MTL::IndexType::IndexTypeUInt32, index_Data->gpuAddress(),
                                   index_Data->length());
}