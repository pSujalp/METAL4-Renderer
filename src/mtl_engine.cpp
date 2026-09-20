#include "mtl_engine.hpp"
#include "metal_view_bridge.h"
#include "autorelease_pool.h"

#include <cstring>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/scalar_constants.hpp>

MTLEngine* MTLEngine::engine = nullptr;

void MTLEngine::init()
{
    engine = this; // callbacks can fire as soon as we poll; set this before anything else

    initDevice();
    initWindow();
    createShaderLibrary();

    createSkybox();

    // FIX: the sphere must exist before createTriangle(), which reads
    // sphere->positions / sphere->uv. It was being created afterwards, so
    // createTriangle() dereferenced a null/garbage pointer (the segfault).
    sphere = new Sphere(1, 30, 30);

    createTriangle();
    model_3d = new Model("assets/Backpack_embedded.fbx", metalDevice, _mainDeletionQueue);

    createRenderPipeline();
    createCommandQueue();
    camera = Camera(glm::vec3(0, 0, 10.0f));
}

void MTLEngine::createShaderLibrary()
{
    using NS::StringEncoding::UTF8StringEncoding;
    shaderLibrary = metalDevice->newDefaultLibrary();
    if (!shaderLibrary)
    {
        std::cerr << "Failed to load default library.";
        std::exit(-1);
    }
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
    // Metal 4 doesn't retain resources for you: make sure the GPU is done
    // with the last submitted frame before anything gets released.
    if (frame_available_shared_event && frame_num > 0)
        frame_available_shared_event->waitUntilSignaledValue(frame_num - 1, UINT64_MAX);

    if (depthTexture) depthTexture->release();
    if (metalRenderPSO) metalRenderPSO->release();
    if (RenderPassPSO) RenderPassPSO->release();
    if (depthStencilState) depthStencilState->release();
    if (triangleVertexBuffer) triangleVertexBuffer->release();
    if (SphereVertexBuffer) SphereVertexBuffer->release();
    if (SphereIndexedBuffer) SphereIndexedBuffer->release();
    if (OffScreenVertexBuffer) OffScreenVertexBuffer->release();
    if (transformationBuffer) transformationBuffer->release();
    if (_offscreenDepthTexture) _offscreenDepthTexture->release();
    if (_renderTexture) _renderTexture->release();
    // FIX: OffScreenRenderPassDescriptor used to be released every frame *and* here (double release).
    // It's now a per-frame local, so there is nothing to release for it.

    _mainDeletionQueue.flush(); // queue, allocators, residency set, compiler, device, model/skybox stuff

    if (shaderLibrary) shaderLibrary->release();

    glfwTerminate();
}

void MTLEngine::initDevice()
{
    metalDevice = MTL::CreateSystemDefaultDevice();
    if (!metalDevice)
    {
        std::cerr << "MTL::CreateSystemDefaultDevice() returned null.\n";
        exit(EXIT_FAILURE);
    }
    _mainDeletionQueue.push_function([=]()
                                     { metalDevice->release(); });
}

void MTLEngine::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindow = glfwCreateWindow(800, 600, "Metal Engine", NULL, NULL);

    // FIX: check for failure *before* registering callbacks on the window.
    if (!glfwWindow)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetMouseButtonCallback(glfwWindow, mouse_button_callback);
    glfwSetCursorPosCallback(glfwWindow, mouse_callback);

    int width, height;
    glfwGetFramebufferSize(glfwWindow, &width, &height);
    glfwSetFramebufferSizeCallback(glfwWindow, frameBufferSizeCallback);

    windowWidth = width;
    windowHeight = height;

    metalLayerHandle = MetalViewBridge::CreateAndAttachLayer(
        glfwWindow, metalDevice, MTL::PixelFormatBGRA8Unorm, width, height);

    MTL::TextureDescriptor* depthTextureDescriptor = MTL::TextureDescriptor::alloc()->init();
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
    PrimitiveVerticesData primitiveVerticesData = PrimitiveVerticesData();
    triangleVertexBuffer = metalDevice->newBuffer(primitiveVerticesData.CubeVertices.data(), primitiveVerticesData.CubeVertices.size() * sizeof(VertexData), MTL::ResourceStorageModeShared);
    triangleVertexBuffer->setLabel(NS::String::string("Triangle Vertex Buffer", NS::ASCIIStringEncoding));

    grassTexture = new Texture("assets/mc_grass.jpeg", metalDevice);
    // A failed load leaves texture == nullptr, which crashes later at gpuResourceID().
    if (!grassTexture->texture)
    {
        std::cerr << "Failed to load assets/mc_grass.jpeg (check the working directory).\n";
        exit(EXIT_FAILURE);
    }

    transformationBuffer = metalDevice->newBuffer(sizeof(MVP), MTL::ResourceStorageModeShared);

    static const AAPLVertex quadVertices[] = {
        {{-1.0, -1.0}, {0.0, 1.0}},
        {{1.0, -1.0}, {1.0, 1.0}},
        {{1.0, 1.0}, {1.0, 0.0}},
        {{1.0, 1.0}, {1.0, 0.0}},
        {{-1.0, 1.0}, {0.0, 0.0}},
        {{-1.0, -1.0}, {0.0, 1.0}},
    };

    OffScreenVertexBuffer = metalDevice->newBuffer(&quadVertices, sizeof(quadVertices), MTL::ResourceStorageModeShared);

    std::vector<VertexData> vertexdata;
    vertexdata.reserve(sphere->positions.size());

    for (size_t i = 0; i < sphere->positions.size(); i++)
    {
        glm::vec4 t = glm::vec4(sphere->positions[i], 1.0f);
        glm::vec2 t1 = sphere->uv[i];
        VertexData vd;

        vd.position = *reinterpret_cast<float4*>(&t);
        vd.textureCoordinate = *reinterpret_cast<float2*>(&t1);
        vertexdata.emplace_back(vd);
    }

    SphereVertexBuffer = metalDevice->newBuffer(vertexdata.data(), vertexdata.size() * sizeof(VertexData), MTL::ResourceStorageModeShared);
    SphereIndexedBuffer = metalDevice->newBuffer(sphere->indices.data(),
                                                 sphere->indices.size() * sizeof(uint32_t),
                                                 MTL::ResourceStorageModeShared);
}

void MTLEngine::createSkybox()
{
    const char* facePaths[6] = {
        "assets/Standard-Cube-Map/right.jpg",
        "assets/Standard-Cube-Map/left.jpg",
        "assets/Standard-Cube-Map/top.jpg",
        "assets/Standard-Cube-Map/bottom.jpg",
        "assets/Standard-Cube-Map/front.jpg",
        "assets/Standard-Cube-Map/back.jpg"};

    skybox = Skybox(metalDevice, facePaths, shaderLibrary, _mainDeletionQueue);
}

void MTLEngine::createCommandQueue()
{
    metal4CommandQueue = metalDevice->newMTL4CommandQueue();
    if (!metal4CommandQueue)
    {
        std::cerr << "newMTL4CommandQueue() returned null -- this device/OS doesn't support Metal 4.\n";
        exit(EXIT_FAILURE);
    }

    for (auto& alloc : cmd_allocators)
    {
        alloc = metalDevice->newCommandAllocator();
    }

    _mainDeletionQueue.push_function([=]()
                                     {
        if (metal4CommandQueue) metal4CommandQueue->release();
        for (auto* alloc : cmd_allocators) if (alloc) alloc->release(); });

    multiCommandBuffer = MultiCommandBuffer(2, metalDevice, _mainDeletionQueue);

    frame_available_shared_event = metalDevice->newSharedEvent();
    frame_available_shared_event->setSignaledValue(0);
    _mainDeletionQueue.push_function([=]()
                                     {
        if (frame_available_shared_event) frame_available_shared_event->release(); });

    auto* argTableDesc = MTL4::ArgumentTableDescriptor::alloc()->init();
    // enum_count() is the number of enumerators, NOT (highest index + 1). If your
    // enums have gaps, setAddress()/setTexture() at a higher index writes out of
    // range. Use the API maximums so any index is valid.
    argTableDesc->setMaxBufferBindCount(31);
    argTableDesc->setMaxTextureBindCount(128);
    arg_table = metalDevice->newArgumentTable(argTableDesc, nullptr);
    argTableDesc->release();
    if (!arg_table)
    {
        std::cerr << "newArgumentTable() returned null.\n";
        exit(EXIT_FAILURE);
    }
    arg_table->setAddress(SphereVertexBuffer->gpuAddress(), (NS::UInteger)(BUFFER_INDEX::VERTEX_DATA));
    arg_table->setAddress(transformationBuffer->gpuAddress(), (NS::UInteger)(BUFFER_INDEX::Transformation_DATA));
    arg_table->setAddress(OffScreenVertexBuffer->gpuAddress(), (NS::UInteger)(BUFFER_INDEX::AAPL_Vertex_DATA));
    MTL::ResourceID r_ID = grassTexture->texture->gpuResourceID();
    arg_table->setTexture(r_ID, (NS::UInteger)TEX_INDEX::COLTEXTURE_ID);
    r_ID = _renderTexture->gpuResourceID();
    arg_table->setTexture(r_ID, (NS::UInteger)TEX_INDEX::AAPL_TEX_ID);

    auto* residencyDesc = MTL::ResidencySetDescriptor::alloc()->init();
    residency_set = metalDevice->newResidencySet(residencyDesc, nullptr);
    residencyDesc->release();

    // Everything reached through a raw GPU address / argument table has to be resident.
    // FIX: the index buffer and the full-screen quad buffer were missing; the GPU
    // faults when it touches non-resident memory.
    residency_set->addAllocation(SphereVertexBuffer);
    residency_set->addAllocation(SphereIndexedBuffer);
    residency_set->addAllocation(OffScreenVertexBuffer);
    residency_set->addAllocation(transformationBuffer);
    residency_set->addAllocation(grassTexture->texture);
    residency_set->addAllocation(_renderTexture);

    skybox.UpdateResidency(residency_set);
    model_3d->UpdateResidency(residency_set);

    residency_set->commit();
    metal4CommandQueue->addResidencySet(residency_set);
    _mainDeletionQueue.push_function([=]()
                                     {
        if (residency_set) residency_set->release();
        if (arg_table) arg_table->release(); });

    CA::MetalLayer* metalLayerCpp = MetalViewBridge::AsMetalLayerCpp(metalLayerHandle);
    metal4CommandQueue->addResidencySet(metalLayerCpp->residencySet());
}

void MTLEngine::createRenderPipeline()
{
    using NS::StringEncoding::UTF8StringEncoding;

    MTL::PixelFormat pixelFormat = MetalViewBridge::GetPixelFormat(metalLayerHandle);
    auto* compilerDesc = MTL4::CompilerDescriptor::alloc()->init();
    metal4Compiler = metalDevice->newCompiler(compilerDesc, nullptr);
    compilerDesc->release();

    if (!metal4Compiler)
    {
        std::cerr << "newCompiler() returned null.\n";
        exit(EXIT_FAILURE);
    }

    _mainDeletionQueue.push_function([=]()
                                     { metal4Compiler->release(); });

    ShaderFunctionDescriptor cubeShaderFunctionDescriptor(shaderLibrary, "vertexShader", "fragmentShader");
    auto* cubePipelineDescriptor = MTL4::RenderPipelineDescriptor::alloc()->init();
    cubePipelineDescriptor->setLabel(NS::String::string("Triangle Rendering Pipeline (Metal 4)", NS::ASCIIStringEncoding));
    cubePipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(pixelFormat);
    cubePipelineDescriptor->setVertexFunctionDescriptor(cubeShaderFunctionDescriptor.vertexShaderFunctionDescriptor);
    cubePipelineDescriptor->setFragmentFunctionDescriptor(cubeShaderFunctionDescriptor.fragmentShaderFunctionDescriptor);

    NS::Error* pPipelineError = nullptr;
    metalRenderPSO = metal4Compiler->newRenderPipelineState(cubePipelineDescriptor, (MTL4::CompilerTaskOptions*)nullptr, &pPipelineError);
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

    model_3d->UpdateShaders(shaderLibrary, _mainDeletionQueue, metal4Compiler, pixelFormat);
    skybox.UpdateShaders(shaderLibrary, _mainDeletionQueue, metal4Compiler, pixelFormat);

    MTL::DepthStencilDescriptor* depthStencilDescriptor = MTL::DepthStencilDescriptor::alloc()->init();
    depthStencilDescriptor->setDepthCompareFunction(MTL::CompareFunctionLess);
    depthStencilDescriptor->setDepthWriteEnabled(true);
    depthStencilState = metalDevice->newDepthStencilState(depthStencilDescriptor);
    depthStencilDescriptor->release();

    MTL::TextureDescriptor* colorDesc = MTL::TextureDescriptor::alloc()->init();
    colorDesc->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
    colorDesc->setWidth(512);
    colorDesc->setHeight(512);
    colorDesc->setStorageMode(MTL::StorageModeShared);
    colorDesc->setUsage(MTL::TextureUsageRenderTarget | MTL::TextureUsageShaderRead);
    _renderTexture = metalDevice->newTexture(colorDesc);
    colorDesc->release();

    MTL::TextureDescriptor* depthDesc = MTL::TextureDescriptor::alloc()->init();
    depthDesc->setPixelFormat(MTL::PixelFormatDepth32Float);
    depthDesc->setWidth(512);
    depthDesc->setHeight(512);
    depthDesc->setStorageMode(MTL::StorageModePrivate);
    depthDesc->setUsage(MTL::TextureUsageRenderTarget);
    _offscreenDepthTexture = metalDevice->newTexture(depthDesc);
    depthDesc->release();

    ShaderFunctionDescriptor OffScreenRenderPasssShaderFunctionDescriptor(shaderLibrary, "vertexRenderPass", "fragmentRenderPass");
    auto* OffScreenPipelineDescriptor = MTL4::RenderPipelineDescriptor::alloc()->init();
    OffScreenPipelineDescriptor->setLabel(NS::String::string("Triangle Rendering Pipeline (Metal 4)", NS::ASCIIStringEncoding));
    OffScreenPipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(pixelFormat);
    OffScreenPipelineDescriptor->setVertexFunctionDescriptor(OffScreenRenderPasssShaderFunctionDescriptor.vertexShaderFunctionDescriptor);
    OffScreenPipelineDescriptor->setFragmentFunctionDescriptor(OffScreenRenderPasssShaderFunctionDescriptor.fragmentShaderFunctionDescriptor);
    pPipelineError = nullptr;

    RenderPassPSO = metal4Compiler->newRenderPipelineState(OffScreenPipelineDescriptor, (MTL4::CompilerTaskOptions*)nullptr, &pPipelineError);
    if (!RenderPassPSO)
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
    OffScreenPipelineDescriptor->release();
    cubePipelineDescriptor->release();
}

void MTLEngine::draw()
{
    sendRenderCommand();
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    ProcessKeyboardInput(deltaTime);

    std::string fps = "Metal 4 Renderer \t\t\t\t" + std::to_string((int)1 / deltaTime);

    glfwSetWindowTitle(glfwWindow, fps.c_str());
}

void MTLEngine::sendRenderCommand()
{
    const size_t frame_idx = frame_num % kMaxFramesInFlight;
    if (frame_num >= kMaxFramesInFlight)
    {
        frame_available_shared_event->waitUntilSignaledValue(frame_num - kMaxFramesInFlight, UINT64_MAX);
    }

    MTL4::CommandAllocator* cmd_alloc = cmd_allocators[frame_idx];
    cmd_alloc->reset();

    CA::MetalDrawable* surface = MetalViewBridge::NextDrawable(metalLayerHandle);
    if (!surface)
    {
        std::cerr << "nextDrawable() returned null -- skipping this frame.\n";
        return;
    }

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
    glm::mat4 viewMatrix = camera.GetViewMatrix();
    float aspectRatio = (float)windowWidth / (float)windowHeight;
    float fov = camera.Zoom;
    float nearZ = 0.1f;
    float farZ = 100.0f;
    glm::mat4 perspectiveMatrix = glm::perspective(fov, aspectRatio, nearZ, farZ);
    glm::mat4 MVP_GLM = perspectiveMatrix * viewMatrix * model;

    // memcpy instead of reinterpret_cast: glm::mat4 isn't guaranteed 16-byte aligned like matrix_float4x4
    MVP mvp1;
    std::memcpy(&mvp1.MVP, &MVP_GLM, sizeof(MVP_GLM));
    std::memcpy(transformationBuffer->contents(), &mvp1, sizeof(MVP));

    glm::mat4 skyboxMVP_GLM = perspectiveMatrix * glm::mat4(glm::mat3(viewMatrix));
    MVP mvpSkybox;
    std::memcpy(&mvpSkybox.MVP, &skyboxMVP_GLM, sizeof(skyboxMVP_GLM));
    std::memcpy(skybox.MVPSkyBoxBuffer->contents(), &mvpSkybox, sizeof(MVP));

    multiCommandBuffer.pushback_function([=](MTL4::CommandBuffer* cb)
                                         {
        MTL4::RenderPassDescriptor* renderPassDescriptor = MTL4::RenderPassDescriptor::alloc()->init();
        MTL::RenderPassColorAttachmentDescriptor* cd = renderPassDescriptor->colorAttachments()->object(0);
        MTL::RenderPassDepthAttachmentDescriptor* depthAttachment = renderPassDescriptor->depthAttachment();

        depthAttachment->setTexture(depthTexture);
        depthAttachment->setLoadAction(MTL::LoadActionClear);
        depthAttachment->setStoreAction(MTL::StoreActionDontCare);
        depthAttachment->setClearDepth(1.0);
        cd->setTexture(surface->texture());
        cd->setLoadAction(MTL::LoadActionClear);
        cd->setClearColor(MTL::ClearColor(55.0f / 255.0f, 55.0f / 255.0f, 55.0f / 255.0f, 1.0));
        cd->setStoreAction(MTL::StoreActionStore);

        MTL4::RenderCommandEncoder* encoder = cb->renderCommandEncoder(renderPassDescriptor);
        encoder->setLabel(NS::String::string("RenderPass", NS::ASCIIStringEncoding));
        encoder->setRenderPipelineState(RenderPassPSO);
        encoder->setArgumentTable(arg_table, MTL::RenderStageVertex);
        encoder->setArgumentTable(arg_table, MTL::RenderStageFragment);
        encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, (NS::UInteger)0, (NS::UInteger)6);
        encoder->endEncoding();
        renderPassDescriptor->release(); });

    multiCommandBuffer.pushback_function([=](MTL4::CommandBuffer* cb)
                                         {
        // FIX: local descriptor. It used to be the member OffScreenRenderPassDescriptor,
        // released here every frame and again in cleanup() (double release -> crash on exit).
        MTL4::RenderPassDescriptor* offPass = MTL4::RenderPassDescriptor::alloc()->init();
        offPass->colorAttachments()->object(0)->setTexture(_renderTexture);
        offPass->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionClear);
        offPass->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
        offPass->colorAttachments()->object(0)->setClearColor(MTL::ClearColor(0.1, 0.1, 0.1, 1.0));
        offPass->depthAttachment()->setTexture(_offscreenDepthTexture);
        offPass->depthAttachment()->setLoadAction(MTL::LoadActionClear);
        offPass->depthAttachment()->setStoreAction(MTL::StoreActionDontCare);
        offPass->depthAttachment()->setClearDepth(1.0);

        MTL4::RenderCommandEncoder* encoder = cb->renderCommandEncoder(offPass);
        encoder->setFrontFacingWinding(MTL::WindingCounterClockwise);
        encoder->setCullMode(MTL::CullModeBack);
        encoder->setLabel(NS::String::string("Triangle", NS::ASCIIStringEncoding));
        encoder->setRenderPipelineState(metalRenderPSO);
        encoder->setDepthStencilState(depthStencilState);
        encoder->setArgumentTable(arg_table, MTL::RenderStageVertex);
        encoder->setArgumentTable(arg_table, MTL::RenderStageFragment);
        encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, (NS::UInteger)0, (NS::UInteger)36);

        encoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, sphere->indexCount, MTL::IndexTypeUInt32, SphereIndexedBuffer->gpuAddress(), SphereIndexedBuffer->length());
        skybox.Draw(encoder);

        MESHMVP Meshmvp;
        Meshmvp.MVP = mvp1.MVP;

        model_3d->Draw(encoder, Meshmvp);

        encoder->endEncoding();
        offPass->release(); });

    metal4CommandQueue->wait(surface);
    multiCommandBuffer.Execute(cmd_alloc, metalLayerHandle, metal4CommandQueue);
    metal4CommandQueue->signalDrawable(surface);
    surface->present();
    metal4CommandQueue->signalEvent(frame_available_shared_event, frame_num);
    frame_num++;
}

void MTLEngine::encodeRenderCommand(MTL4::RenderCommandEncoder* encoder)
{
}

void MTLEngine::ProcessKeyboardInput(float deltaTime)
{
    if (glfwGetKey(glfwWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(glfwWindow, true);
    if (glfwGetKey(glfwWindow, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(glfwWindow, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(glfwWindow, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(glfwWindow, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(glfwWindow, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(glfwWindow, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);

    if (glfwGetKey(glfwWindow, GLFW_KEY_M) == GLFW_PRESS)
        glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void MTLEngine::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        double xpos = width / 2.0;
        double ypos = height / 2.0;
        glfwSetCursorPos(window, xpos, ypos);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

void MTLEngine::frameBufferSizeCallback(GLFWwindow* window, int width, int height)
{
    if (MTLEngine::engine)
    {
        engine->resizeFrameBuffer(width, height);
    }
}

void MTLEngine::resizeFrameBuffer(int width, int height)
{
    // Minimizing reports 0x0; a zero-size texture comes back null and crashes later.
    if (width <= 0 || height <= 0)
        return;

    // The GPU may still be using the old depth texture (Metal 4 doesn't retain it).
    if (frame_available_shared_event && frame_num > 0)
        frame_available_shared_event->waitUntilSignaledValue(frame_num - 1, UINT64_MAX);

    // FIX: the new size was ignored; the texture was rebuilt at the old windowWidth/Height.
    windowWidth = width;
    windowHeight = height;

    if (depthTexture)
    {
        depthTexture->release();
        depthTexture = nullptr;
    }

    MTL::TextureDescriptor* TextureDescriptor = MTL::TextureDescriptor::alloc()->init();
    TextureDescriptor->setTextureType(MTL::TextureType2D);
    TextureDescriptor->setPixelFormat(MTL::PixelFormatDepth32Float);
    TextureDescriptor->setWidth((NS::UInteger)windowWidth);
    TextureDescriptor->setHeight((NS::UInteger)windowHeight);
    TextureDescriptor->setUsage(MTL::TextureUsageRenderTarget);
    TextureDescriptor->setStorageMode(MTL::StorageModePrivate);
    depthTexture = metalDevice->newTexture(TextureDescriptor);
    TextureDescriptor->release();
}

void MTLEngine::mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    if (!engine)
        return;

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (engine->firstMouse)
    {
        engine->lastX = xpos;
        engine->lastY = ypos;
        engine->firstMouse = false;
    }

    float xoffset = xpos - engine->lastX;
    float yoffset = engine->lastY - ypos;

    engine->lastX = xpos;
    engine->lastY = ypos;

    engine->camera.ProcessMouseMovement(xoffset, yoffset);
}