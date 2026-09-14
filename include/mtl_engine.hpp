//
//  mtl_engine.hpp
//  MetalTutorial
//

#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>

#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>
#include <QuartzCore/CAMetalLayer.h>
#include "Shader.h"
#include "VertexData.hpp"
#include "simd/simd.h"

#include <Texture.hpp>
#include "utils.hpp"

#include <iostream>
#include "stb_image.h"

class MTLEngine {
public:
    void init();
    void run();
    void cleanup();

private:
    void initDevice();
    void initWindow();

    void createTriangle();
    void createCommandQueue();
    void createRenderPipeline();

    void draw();
    void sendRenderCommand();
    void encodeRenderCommand(MTL4::RenderCommandEncoder* renderCommandEncoder);

    static constexpr size_t kMaxFramesInFlight = 3;

    MTL::Device*        metalDevice  = nullptr;
    GLFWwindow*         glfwWindow   = nullptr;
    void*               metalLayerHandle = nullptr;

    MTL::Buffer*  triangleVertexBuffer = nullptr;
    MTL::Library* shaderLibrary        = nullptr;

    MTL4::CommandQueue*       metal4CommandQueue  = nullptr;
    MTL4::CommandBuffer*      metal4CommandBuffer = nullptr;
    MTL::RenderPipelineState* metalRenderPSO      = nullptr;
    MTL::DepthStencilState * depthStencilState = nullptr;
    MTL4::Compiler*           metal4Compiler      = nullptr;

    Array<MTL4::CommandAllocator*, kMaxFramesInFlight> cmd_allocators{};
    MTL4::ArgumentTable* arg_table     = nullptr;
    MTL::ResidencySet*   residency_set = nullptr;
    MTL::SharedEvent*    frame_available_shared_event = nullptr;
    size_t frame_num = 0;


    Texture* grassTexture;
    MTL::Texture * depthTexture;
    float windowHeight, windowWidth;

    MTL::Buffer * transformationBuffer;

  
};