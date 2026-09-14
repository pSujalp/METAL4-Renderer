#pragma once

#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>
#include <QuartzCore/CAMetalLayer.h>
#include "simd/simd.h"
#include "VertexData.hpp"
#include "Texture.hpp"
#include "AAPLMathUtilities.h"
#include "Time.hpp"

// Metal's clip-space depth range is [0,1], not OpenGL's [-1,1] which GLM
// defaults to. Without this, glm::perspective() produces z/w values that
// fall outside Metal's expected range for most geometry, and the hardware
// rasterizer clips it before the depth-stencil state even runs -- this is
// why the cube wasn't appearing at all. Must be defined before glm.hpp is
// included, here or anywhere else in the project.
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <array>

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
        static constexpr size_t kMaxFramesInFlight = 3;

        MTL::Device* _pDevice = nullptr;

        // Metal 4 command submission objects (replace MTL::CommandQueue /
        // MTL::CommandBuffer / setVertexBuffer-style classic encoding).
        MTL4::CommandQueue*  _pCommandQueue4  = nullptr;
        MTL4::CommandBuffer* _pCommandBuffer  = nullptr;
        MTL4::Compiler*      _pCompiler       = nullptr;

        std::array<MTL4::CommandAllocator*, kMaxFramesInFlight> _cmdAllocators{};
        MTL4::ArgumentTable* _argTable      = nullptr;
        MTL::ResidencySet*   _residencySet  = nullptr;
        MTL::SharedEvent*    _frameEvent    = nullptr;
        size_t               _frameNum      = 0;

        MTL::RenderPipelineState* _pPSO = nullptr;
        MTL::DepthStencilState* depthStencilState = nullptr;

        Texture* grassTexture = nullptr;
        MTL::Buffer* cubeVertexBuffer = nullptr;
        MTL::Buffer* transformationBuffer = nullptr;
        MTL::Library* metallibrary = nullptr;

        MTL::Buffer * UniformBuffer;

        // Manually-managed depth texture. MTKView's own
        // currentMTL4RenderPassDescriptor convenience isn't available in
        // this metal-cpp binding, so the MTL4::RenderPassDescriptor is
        // built by hand each frame in draw() -- this backs its depth
        // attachment.
        MTL::Texture* depthTexture = nullptr;
        double _depthTexWidth = 0.0;
        double _depthTexHeight = 0.0;
        void ensureDepthTexture(double width, double height);
};