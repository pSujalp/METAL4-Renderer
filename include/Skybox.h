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