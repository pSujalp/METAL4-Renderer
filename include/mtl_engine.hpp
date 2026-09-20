




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
#include <vector>

#include <Texture.hpp>
#include "utils.hpp"

#include <iostream>
#include "stb_image.h"
#include "Camera.h"
#include "PrimitiveVerticesData.h"
#include <magic_enum/magic_enum.hpp>
#include "DeletionQueue.h"
#include "ShaderFunctionDescriptor.h"
#include "MultiCommandBuffer.h"

#include "SkyboxData.hpp"
#include "Skybox.h"
#include <string>
#include "Model.h"

#include "Sphere.h"




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
    void ProcessKeyboardInput(float deltaTime);

    void createShaderLibrary();

    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

    // static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);

    static void frameBufferSizeCallback(GLFWwindow *window, int width, int height);


    static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);


    void resizeFrameBuffer(int width, int height);

    void draw();
    void sendRenderCommand();
    void encodeRenderCommand(MTL4::RenderCommandEncoder* renderCommandEncoder);
    void createSkybox();

    static constexpr size_t kMaxFramesInFlight = 3;

    MTL::Device*        metalDevice  = nullptr;
    GLFWwindow*         glfwWindow   = nullptr;
    void*               metalLayerHandle = nullptr;

    MTL::Buffer*  triangleVertexBuffer = nullptr;
    MTL::Library* shaderLibrary        = nullptr;

    MTL4::CommandQueue*       metal4CommandQueue  = nullptr;

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
    MTL::Buffer * SphereVertexBuffer;
    MTL::Buffer * SphereIndexedBuffer;
    
    DeletionQueue _mainDeletionQueue;

    MultiCommandBuffer multiCommandBuffer;


    static MTLEngine* engine;
    Skybox skybox;

    MTL4::RenderPassDescriptor *OffScreenRenderPassDescriptor;
    MTL::Texture * _renderTexture;
    MTL::Texture * _offscreenDepthTexture;
    MTL::RenderPipelineState * RenderPassPSO;
    MTL::Buffer * OffScreenVertexBuffer;


    Camera  camera;
    float deltaTime = 0.0f;	
    float lastFrame = 0.0f;
    float lastX;
    float lastY;
    bool firstMouse = true;
    bool rightMouseButtonPressed = false;

    Model * model_3d;
    Sphere * sphere;

  
};