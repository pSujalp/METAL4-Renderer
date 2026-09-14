#pragma once 

#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>
#include "simd/simd.h"
#include "VertexData.hpp"
#include "Texture.hpp"
#include "AAPLMathUtilities.h"
#include "Time.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

class Renderer
{
    public:
        Renderer( MTL::Device* pDevice );
        ~Renderer();
        void draw( MTK::View* pView );

        void buildShaders();
        void createDefaultLibrary(MTL::Device* pDevice );
        void CreateCube();

    private:
        MTL::Device* _pDevice = nullptr;
        MTL::CommandQueue* _pCommandQueue = nullptr;
        MTL::RenderPipelineState* _pPSO = nullptr;
        Texture* grassTexture = nullptr;
        MTL::Buffer* cubeVertexBuffer = nullptr;
        MTL::Buffer* UniformBuffer = nullptr;
        MTL::Buffer* transformationBuffer = nullptr;
        MTL::Library* metallibrary = nullptr;
        MTL::DepthStencilState* depthStencilState = nullptr;
};