#include "mtl_engine.hpp"
#include "metal_view_bridge.h"
#include "autorelease_pool.h"


// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec3.hpp> 
#include <glm/vec4.hpp> 
#include <glm/mat4x4.hpp> 
#include <glm/ext/matrix_transform.hpp> 
#include <glm/ext/matrix_clip_space.hpp> 
#include <glm/ext/scalar_constants.hpp> 

void MTLEngine::init()
{
    initDevice();
    initWindow();
    createTriangle();
    createCommandQueue();
    createRenderPipeline();
}

void MTLEngine::run()
{
    while (!glfwWindowShouldClose(glfwWindow))
    {
        {
            AutoreleasePoolGuard pool;
            draw();
        }
        glfwPollEvents();
    }
}

void MTLEngine::cleanup()
{
    glfwTerminate();

    if (depthTexture)
        depthTexture->release();
    if (residency_set)
        residency_set->release();
    if (arg_table)
        arg_table->release();
    for (auto *alloc : cmd_allocators)
    {
        if (alloc)
            alloc->release();
    }
    if (frame_available_shared_event)
        frame_available_shared_event->release();
    if (metal4CommandBuffer)
        metal4CommandBuffer->release();
    if (metal4CommandQueue)
        metal4CommandQueue->release();
    if (metal4Compiler)
        metal4Compiler->release();
    if (metalRenderPSO)
        metalRenderPSO->release();
    if (shaderLibrary)
        shaderLibrary->release();
    if (triangleVertexBuffer)
        triangleVertexBuffer->release();

    metalDevice->release();
}

void MTLEngine::initDevice()
{
    metalDevice = MTL::CreateSystemDefaultDevice();
    if (!metalDevice)
    {
        std::cerr << "MTL::CreateSystemDefaultDevice() returned null.\n";
        exit(EXIT_FAILURE);
    }
}

void MTLEngine::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindow = glfwCreateWindow(800, 600, "Metal Engine", NULL, NULL);

    if (!glfwWindow)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    int width, height;
    glfwGetFramebufferSize(glfwWindow, &width, &height);

    windowWidth = width;
    windowHeight = height;

    metalLayerHandle = MetalViewBridge::CreateAndAttachLayer(
        glfwWindow, metalDevice, MTL::PixelFormatBGRA8Unorm, width, height);

    MTL::TextureDescriptor *depthTextureDescriptor = MTL::TextureDescriptor::alloc()->init();
    depthTextureDescriptor->setTextureType(MTL::TextureType2D);
    depthTextureDescriptor->setPixelFormat(MTL::PixelFormatDepth32Float);
    depthTextureDescriptor->setWidth((NS::UInteger)windowWidth);
    depthTextureDescriptor->setHeight((NS::UInteger)windowHeight);
    depthTextureDescriptor->setUsage(MTL::TextureUsageRenderTarget);

    depthTextureDescriptor->setStorageMode(MTL::StorageModePrivate);

    depthTexture = metalDevice->newTexture(depthTextureDescriptor);
    depthTextureDescriptor->release();

    if (!depthTexture)
    {
        std::cerr << "Failed to create depth texture.\n";
        exit(EXIT_FAILURE);
    }
    depthTexture->setLabel(NS::String::string("Depth Texture", NS::ASCIIStringEncoding));
}

void MTLEngine::createTriangle()
{
    VertexData squareVertices[]{

        {{-0.5, -0.5, 0.5, 1.0}, {0.0, 0.0}},
        {{0.5, -0.5, 0.5, 1.0}, {1.0, 0.0}},
        {{0.5, 0.5, 0.5, 1.0}, {1.0, 1.0}},
        {{0.5, 0.5, 0.5, 1.0}, {1.0, 1.0}},
        {{-0.5, 0.5, 0.5, 1.0}, {0.0, 1.0}},
        {{-0.5, -0.5, 0.5, 1.0}, {0.0, 0.0}},

        {{0.5, -0.5, -0.5, 1.0}, {0.0, 0.0}},
        {{-0.5, -0.5, -0.5, 1.0}, {1.0, 0.0}},
        {{-0.5, 0.5, -0.5, 1.0}, {1.0, 1.0}},
        {{-0.5, 0.5, -0.5, 1.0}, {1.0, 1.0}},
        {{0.5, 0.5, -0.5, 1.0}, {0.0, 1.0}},
        {{0.5, -0.5, -0.5, 1.0}, {0.0, 0.0}},

        {{-0.5, 0.5, 0.5, 1.0}, {0.0, 0.0}},
        {{0.5, 0.5, 0.5, 1.0}, {1.0, 0.0}},
        {{0.5, 0.5, -0.5, 1.0}, {1.0, 1.0}},
        {{0.5, 0.5, -0.5, 1.0}, {1.0, 1.0}},
        {{-0.5, 0.5, -0.5, 1.0}, {0.0, 1.0}},
        {{-0.5, 0.5, 0.5, 1.0}, {0.0, 0.0}},

        {{-0.5, -0.5, -0.5, 1.0}, {0.0, 0.0}},
        {{0.5, -0.5, -0.5, 1.0}, {1.0, 0.0}},
        {{0.5, -0.5, 0.5, 1.0}, {1.0, 1.0}},
        {{0.5, -0.5, 0.5, 1.0}, {1.0, 1.0}},
        {{-0.5, -0.5, 0.5, 1.0}, {0.0, 1.0}},
        {{-0.5, -0.5, -0.5, 1.0}, {0.0, 0.0}},

        {{-0.5, -0.5, -0.5, 1.0}, {0.0, 0.0}},
        {{-0.5, -0.5, 0.5, 1.0}, {1.0, 0.0}},
        {{-0.5, 0.5, 0.5, 1.0}, {1.0, 1.0}},
        {{-0.5, 0.5, 0.5, 1.0}, {1.0, 1.0}},
        {{-0.5, 0.5, -0.5, 1.0}, {0.0, 1.0}},
        {{-0.5, -0.5, -0.5, 1.0}, {0.0, 0.0}},

        {{0.5, -0.5, 0.5, 1.0}, {0.0, 0.0}},
        {{0.5, -0.5, -0.5, 1.0}, {1.0, 0.0}},
        {{0.5, 0.5, -0.5, 1.0}, {1.0, 1.0}},
        {{0.5, 0.5, -0.5, 1.0}, {1.0, 1.0}},
        {{0.5, 0.5, 0.5, 1.0}, {0.0, 1.0}},
        {{0.5, -0.5, 0.5, 1.0}, {0.0, 0.0}},
    };

    triangleVertexBuffer = metalDevice->newBuffer(&squareVertices, sizeof(squareVertices), MTL::ResourceStorageModeShared);
    triangleVertexBuffer->setLabel(NS::String::string("Triangle Vertex Buffer", NS::ASCIIStringEncoding));
    grassTexture = new Texture("assets/mc_grass.jpeg", metalDevice);
    transformationBuffer = metalDevice->newBuffer(sizeof(MVP), MTL::ResourceStorageModeShared);
}

void MTLEngine::createCommandQueue()
{
    metal4CommandQueue = metalDevice->newMTL4CommandQueue();
    if (!metal4CommandQueue)
    {
        std::cerr << "newMTL4CommandQueue() returned null -- this device/OS doesn't support Metal 4.\n";
        exit(EXIT_FAILURE);
    }

    for (auto &alloc : cmd_allocators)
    {
        alloc = metalDevice->newCommandAllocator();
    }
    metal4CommandBuffer = metalDevice->newCommandBuffer();

    frame_available_shared_event = metalDevice->newSharedEvent();
    frame_available_shared_event->setSignaledValue(0);
    auto *argTableDesc = MTL4::ArgumentTableDescriptor::alloc()->init();
    argTableDesc->setMaxBufferBindCount(3);
    argTableDesc->setMaxTextureBindCount(1);
    arg_table = metalDevice->newArgumentTable(argTableDesc, nullptr);
    argTableDesc->release();
    if (!arg_table)
    {
        std::cerr << "newArgumentTable() returned null.\n";
        exit(EXIT_FAILURE);
    }
    arg_table->setAddress(triangleVertexBuffer->gpuAddress(), (NS::UInteger)BUFFER_INDEX::VERTEX_DATA);

    arg_table->setAddress(transformationBuffer->gpuAddress(), (NS::UInteger)BUFFER_INDEX::Transformation_DATA);

    MTL::ResourceID r_ID = grassTexture->texture->gpuResourceID();
    arg_table->setTexture(r_ID, (NS::UInteger)BUFFER_INDEX::COLTEXTURE_ID);

    auto *residencyDesc = MTL::ResidencySetDescriptor::alloc()->init();
    residency_set = metalDevice->newResidencySet(residencyDesc, nullptr);

    residencyDesc->release();
    residency_set->addAllocation(triangleVertexBuffer);
    residency_set->addAllocation(grassTexture->texture);
    residency_set->addAllocation(transformationBuffer);

    residency_set->addAllocation(depthTexture);

    residency_set->commit();
    metal4CommandQueue->addResidencySet(residency_set);

    CA::MetalLayer *metalLayerCpp = MetalViewBridge::AsMetalLayerCpp(metalLayerHandle);
    metal4CommandQueue->addResidencySet(metalLayerCpp->residencySet());
}

void MTLEngine::createRenderPipeline()
{
    using NS::StringEncoding::UTF8StringEncoding;
    shaderLibrary = metalDevice->newDefaultLibrary();
    if (!shaderLibrary)
    {
        std::cerr << "Failed to load default library.";
        std::exit(-1);
    }

    MTL::PixelFormat pixelFormat = MetalViewBridge::GetPixelFormat(metalLayerHandle);

    auto *compilerDesc = MTL4::CompilerDescriptor::alloc()->init();
    metal4Compiler = metalDevice->newCompiler(compilerDesc, nullptr);
    compilerDesc->release();

    if (!metal4Compiler)
    {
        std::cerr << "newCompiler() returned null.\n";
        exit(EXIT_FAILURE);
    }
    auto *vertexFunctionDescriptor = MTL4::LibraryFunctionDescriptor::alloc()->init();
    vertexFunctionDescriptor->setLibrary(shaderLibrary);
    vertexFunctionDescriptor->setName(NS::String::string("vertexShader", NS::ASCIIStringEncoding));
    auto *fragmentFunctionDescriptor = MTL4::LibraryFunctionDescriptor::alloc()->init();
    fragmentFunctionDescriptor->setLibrary(shaderLibrary);
    fragmentFunctionDescriptor->setName(NS::String::string("fragmentShader", NS::ASCIIStringEncoding));

    auto *pipelineDescriptor = MTL4::RenderPipelineDescriptor::alloc()->init();
    pipelineDescriptor->setLabel(NS::String::string("Triangle Rendering Pipeline (Metal 4)", NS::ASCIIStringEncoding));
    pipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(pixelFormat);
    pipelineDescriptor->setVertexFunctionDescriptor(vertexFunctionDescriptor);
    pipelineDescriptor->setFragmentFunctionDescriptor(fragmentFunctionDescriptor);

    MTL::DepthStencilDescriptor *depthStencilDescriptor = MTL::DepthStencilDescriptor::alloc()->init();
    depthStencilDescriptor->setDepthCompareFunction(MTL::CompareFunctionLess);
    depthStencilDescriptor->setDepthWriteEnabled(true);
    depthStencilState = metalDevice->newDepthStencilState(depthStencilDescriptor);
    depthStencilDescriptor->release();

    NS::Error *pPipelineError = nullptr;
    metalRenderPSO = metal4Compiler->newRenderPipelineState(pipelineDescriptor, (MTL4::CompilerTaskOptions *)nullptr, &pPipelineError);
    if (!metalRenderPSO)
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

    pipelineDescriptor->release();
    vertexFunctionDescriptor->release();
    fragmentFunctionDescriptor->release();
}

void MTLEngine::draw()
{
    sendRenderCommand();
}

void MTLEngine::sendRenderCommand()
{
    const size_t frame_idx = frame_num % kMaxFramesInFlight;

    if (frame_num >= kMaxFramesInFlight)
    {
        frame_available_shared_event->waitUntilSignaledValue(frame_num - kMaxFramesInFlight, UINT64_MAX);
    }

    MTL4::CommandAllocator *cmd_alloc = cmd_allocators[frame_idx];
    cmd_alloc->reset();

    CA::MetalDrawable *surface = MetalViewBridge::NextDrawable(metalLayerHandle);
    if (!surface)
    {
        std::cerr << "nextDrawable() returned null -- skipping this frame.\n";
        return;
    }

    MTL4::RenderPassDescriptor *renderPassDescriptor = MTL4::RenderPassDescriptor::alloc()->init();
    MTL::RenderPassColorAttachmentDescriptor *cd = renderPassDescriptor->colorAttachments()->object(0);
    MTL::RenderPassDepthAttachmentDescriptor *depthAttachment = renderPassDescriptor->depthAttachment();

    depthAttachment->setTexture(depthTexture);
    depthAttachment->setLoadAction(MTL::LoadActionClear);
    depthAttachment->setStoreAction(MTL::StoreActionDontCare);
    depthAttachment->setClearDepth(1.0);

    cd->setTexture(surface->texture());
    cd->setLoadAction(MTL::LoadActionClear);
    cd->setClearColor(MTL::ClearColor(255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 1.0));
    cd->setStoreAction(MTL::StoreActionStore);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 1.0f, -1.0f));
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

    static float lastTime = static_cast<float>(glfwGetTime());
    float currentTime = static_cast<float>(glfwGetTime());
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    static float accumulatedDegrees = 0.0f;
    const float rotationSpeedDegreesPerSecond = 45.0f;
    accumulatedDegrees += rotationSpeedDegreesPerSecond * deltaTime;
    if (accumulatedDegrees >= 360.0f)
        accumulatedDegrees -= 360.0f;

    float angleInRadians = accumulatedDegrees * (M_PI / 180.0f);
    model = glm::rotate(model, angleInRadians, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 viewMatrix = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 5.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

    float aspectRatio = (float)windowWidth / (float)windowHeight;
    float fov = glm::radians(60.0f);
    float nearZ = 0.1f;
    float farZ = 100.0f;
    glm::mat4 perspectiveMatrix = glm::perspective(fov, aspectRatio, nearZ, farZ);
    glm::mat4 MVP_GLM = perspectiveMatrix * viewMatrix * model;

    MVP mvp1;
    mvp1.MVP = matrix_float4x4({
        simd::float4{MVP_GLM[0][0], MVP_GLM[0][1], MVP_GLM[0][2], MVP_GLM[0][3]},
        simd::float4{MVP_GLM[1][0], MVP_GLM[1][1], MVP_GLM[1][2], MVP_GLM[1][3]},
        simd::float4{MVP_GLM[2][0], MVP_GLM[2][1], MVP_GLM[2][2], MVP_GLM[2][3]},
        simd::float4{MVP_GLM[3][0], MVP_GLM[3][1], MVP_GLM[3][2], MVP_GLM[3][3]},
    });
    memcpy(transformationBuffer->contents(), &mvp1, sizeof(MVP));

    metal4CommandBuffer->beginCommandBuffer(cmd_alloc);

    MTL4::RenderCommandEncoder *encoder = metal4CommandBuffer->renderCommandEncoder(renderPassDescriptor);
    encoder->setFrontFacingWinding(MTL::WindingCounterClockwise);
    encoder->setCullMode(MTL::CullModeBack);
    encodeRenderCommand(encoder);
    encoder->endEncoding();

    CA::MetalLayer *metalLayerCpp = MetalViewBridge::AsMetalLayerCpp(metalLayerHandle);

    metal4CommandBuffer->useResidencySet(metalLayerCpp->residencySet());

    metal4CommandBuffer->endCommandBuffer();

    metal4CommandQueue->wait(surface);
    metal4CommandQueue->commit(&metal4CommandBuffer, 1);
    metal4CommandQueue->signalDrawable(surface);
    surface->present();

    metal4CommandQueue->signalEvent(frame_available_shared_event, frame_num);
    frame_num++;

    renderPassDescriptor->release();
}

void MTLEngine::encodeRenderCommand(MTL4::RenderCommandEncoder *encoder)
{
    encoder->setLabel(NS::String::string("Triangle", NS::ASCIIStringEncoding));

    encoder->setRenderPipelineState(metalRenderPSO);
    encoder->setDepthStencilState(depthStencilState);
    encoder->setArgumentTable(arg_table, MTL::RenderStageVertex);
    encoder->setArgumentTable(arg_table, MTL::RenderStageFragment);

    encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, (NS::UInteger)0, (NS::UInteger)36);
}