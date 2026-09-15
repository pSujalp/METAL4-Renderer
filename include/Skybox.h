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
        argTableDesc->setMaxBufferBindCount(magic_enum::enum_count<BUFFER_INDEX>());
        argTableDesc->setMaxTextureBindCount(magic_enum::enum_count<TEX_INDEX>());
        arg_table = metalDevice->newArgumentTable(argTableDesc, nullptr);
        argTableDesc->release();
        if (!arg_table)
        {
            std::cerr << "newArgumentTable() returned null.\n";
            exit(EXIT_FAILURE);
        }

        arg_table->setAddress(SkyBoxVertexBuffer->gpuAddress(), (NS::UInteger)(BUFFER_INDEX::SKYBOX_BUFFER_INDEX));
        arg_table->setAddress(MVPSkyBoxBuffer->gpuAddress(), (NS::UInteger)(BUFFER_INDEX::MVP_BUFFER_INDEX)); 
        MTL::ResourceID r_ID = skyboxTexture->texture->gpuResourceID();
        arg_table->setTexture(r_ID, (NS::UInteger)TEX_INDEX::SKYTEX_TEXTURE_INDEX);

        dq.push_function([=]()
                         {
            if (arg_table) arg_table->release(); });
    }

    MTL::Buffer *SkyBoxVertexBuffer;
    MTL::Buffer *MVPSkyBoxBuffer;
    CubeTexture *skyboxTexture;
    MTL::RenderPipelineState *SkyboxPSO;
    MTL::DepthStencilState *skyboxDepthStencilState;
    ShaderFunctionDescriptor shaderVertexFunctionDescriptor;
    MTL4::ArgumentTable *arg_table;
};