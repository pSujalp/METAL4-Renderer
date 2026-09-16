#pragma once
#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>
#include <QuartzCore/CAMetalLayer.h>
#include "Texture.hpp"
#include "SkyboxData.hpp"
#include "PrimitiveVerticesData.h"
#include "ShaderFunctionDescriptor.h"
#include "DeletionQueue.h"

class Skybox
{
public:
    Skybox() = default;

    Skybox(MTL::Device *metalDevice, const char *facepaths[6], MTL::Library *lib, DeletionQueue &dq)
    {
        PrimitiveVerticesData primitiveVerticesData;

        SkyBoxVertexBuffer = metalDevice->newBuffer(
            primitiveVerticesData.SkyboxVertices.data(),
            primitiveVerticesData.SkyboxVertices.size() * sizeof(SkyboxVertexData),
            MTL::ResourceStorageModeShared);

        MVPSkyBoxBuffer = metalDevice->newBuffer(sizeof(MVP), MTL::ResourceStorageModeShared);
        skyboxTexture = new CubeTexture(facepaths, metalDevice);

        MTL::DepthStencilDescriptor *depthDesc = MTL::DepthStencilDescriptor::alloc()->init();
        depthDesc->setDepthCompareFunction(MTL::CompareFunctionLessEqual);
        depthDesc->setDepthWriteEnabled(false);
        skyboxDepthStencilState = metalDevice->newDepthStencilState(depthDesc);
        depthDesc->release();

        auto *argTableDesc = MTL4::ArgumentTableDescriptor::alloc()->init();
        argTableDesc->setMaxBufferBindCount(magic_enum::enum_count<SKYBUFFER_INDEX>());
        argTableDesc->setMaxTextureBindCount(magic_enum::enum_count<SKYTEX_INDEX>());
        arg_table = metalDevice->newArgumentTable(argTableDesc, nullptr);
        argTableDesc->release();
        if (!arg_table)
        {
            std::cerr << "newArgumentTable() returned null.\n";
            exit(EXIT_FAILURE);
        }

        arg_table->setAddress(SkyBoxVertexBuffer->gpuAddress(), (NS::UInteger)(SKYBUFFER_INDEX::SKYBOX_BUFFER_INDEX));
        arg_table->setAddress(MVPSkyBoxBuffer->gpuAddress(), (NS::UInteger)(SKYBUFFER_INDEX::MVP_BUFFER_INDEX));
        MTL::ResourceID r_ID = skyboxTexture->texture->gpuResourceID();
        arg_table->setTexture(r_ID, (NS::UInteger)SKYTEX_INDEX::SKYTEX_TEXTURE_INDEX);


        dq.push_function([=](){
            if (arg_table) arg_table->release(); });
    }
    void UpdateShaders(MTL::Library *lib, DeletionQueue &dq, MTL4::Compiler* metal4Complier,MTL::PixelFormat  pf){
        using NS::StringEncoding::UTF8StringEncoding;

        ShaderFunctionDescriptor skyboxShaderFunctionDescriptor(lib, "skyboxVertex", "skyboxFragment");
    
        auto *skyboxPipelineDescriptor = MTL4::RenderPipelineDescriptor::alloc()->init();
        skyboxPipelineDescriptor->setLabel(NS::String::string("Skybox", NS::ASCIIStringEncoding));
        skyboxPipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(pf);
        skyboxPipelineDescriptor->setVertexFunctionDescriptor(skyboxShaderFunctionDescriptor.vertexShaderFunctionDescriptor);
        skyboxPipelineDescriptor->setFragmentFunctionDescriptor(skyboxShaderFunctionDescriptor.fragmentShaderFunctionDescriptor);

        NS::Error * pPipelineError = nullptr;
        SkyboxPSO = metal4Complier->newRenderPipelineState(skyboxPipelineDescriptor, (MTL4::CompilerTaskOptions *)nullptr, &pPipelineError);
        if (!SkyboxPSO)
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

    MTL::Buffer *SkyBoxVertexBuffer;
    MTL::Buffer *MVPSkyBoxBuffer;
    CubeTexture *skyboxTexture;
    MTL::RenderPipelineState *SkyboxPSO;
    MTL::DepthStencilState *skyboxDepthStencilState;
    MTL4::ArgumentTable *arg_table;
};