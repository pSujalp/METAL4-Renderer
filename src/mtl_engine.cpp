#include "mtl_engine.hpp"
#include "metal_view_bridge.h"
#include "autorelease_pool.h"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/scalar_constants.hpp>

MTLEngine* MTLEngine::engine = nullptr;

void MTLEngine::init()
{
    initDevice();
    initWindow();
    createShaderLibrary();
    createTriangle();
    createSkybox();          
                              
                              
    createCommandQueue();
    createRenderPipeline();
    camera = Camera(glm::vec3(0,0,10.0f));
    engine = this;
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
    glfwTerminate();

    _mainDeletionQueue.flush();
    if (depthTexture)
        depthTexture->release();
    if (metalRenderPSO)
        metalRenderPSO->release();
    if (shaderLibrary)
        shaderLibrary->release();
    if (triangleVertexBuffer)
        triangleVertexBuffer->release();
}

void MTLEngine::initDevice()
{
    metalDevice = MTL::CreateSystemDefaultDevice();
    if (!metalDevice)
    {
        std::cerr << "MTL::CreateSystemDefaultDevice() returned null.\n";
        exit(EXIT_FAILURE);
    }
    _mainDeletionQueue.push_function([=](){
        metalDevice->release();
    });
}
void MTLEngine::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindow = glfwCreateWindow(800, 600, "Metal Engine", NULL, NULL);
    glfwSetMouseButtonCallback(glfwWindow, mouse_button_callback);

    glfwSetCursorPosCallback(glfwWindow, mouse_callback);
    if (!glfwWindow)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    int width, height;
    glfwGetFramebufferSize(glfwWindow, &width, &height);
    glfwSetFramebufferSizeCallback(glfwWindow,frameBufferSizeCallback);

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
    PrimitiveVerticesData primitiveVerticesData = PrimitiveVerticesData();
    triangleVertexBuffer = metalDevice->newBuffer(primitiveVerticesData.CubeVertices.data(), primitiveVerticesData.CubeVertices.size() * sizeof(VertexData), MTL::ResourceStorageModeShared);
    triangleVertexBuffer->setLabel(NS::String::string("Triangle Vertex Buffer", NS::ASCIIStringEncoding));
    grassTexture = new Texture("assets/mc_grass.jpeg", metalDevice);
    transformationBuffer = metalDevice->newBuffer(sizeof(MVP), MTL::ResourceStorageModeShared);
}

void MTLEngine::createSkybox()
{
    const char *facePaths[6] = {
        "assets/right.jpg",
        "assets/left.jpg",
        "assets/top.jpg",
        "assets/bottom.jpg",
        "assets/front.jpg",
        "assets/back.jpg"};

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

    for (auto &alloc : cmd_allocators){
        alloc = metalDevice->newCommandAllocator();
    }

    _mainDeletionQueue.push_function([=](){
    if(metal4CommandQueue)    metal4CommandQueue->release();
    for (auto *alloc : cmd_allocators) if (alloc) alloc->release();
    });

    multiCommandBuffer = MultiCommandBuffer(2 , metalDevice,_mainDeletionQueue);

    frame_available_shared_event = metalDevice->newSharedEvent();
    frame_available_shared_event->setSignaledValue(0);


    _mainDeletionQueue.push_function([=](){
        if (frame_available_shared_event) frame_available_shared_event->release();
    });
    auto *argTableDesc = MTL4::ArgumentTableDescriptor::alloc()->init();
    argTableDesc->setMaxBufferBindCount(magic_enum::enum_count<BUFFER_INDEX>() );
    argTableDesc->setMaxTextureBindCount(magic_enum::enum_count<TEX_INDEX>() );
    arg_table = metalDevice->newArgumentTable(argTableDesc, nullptr);
    argTableDesc->release();
    if (!arg_table)
    {
        std::cerr << "newArgumentTable() returned null.\n";
        exit(EXIT_FAILURE);
    }

    arg_table->setAddress(triangleVertexBuffer->gpuAddress(), (NS::UInteger)(BUFFER_INDEX::VERTEX_DATA));
    arg_table->setAddress(transformationBuffer->gpuAddress(), (NS::UInteger)(BUFFER_INDEX::Transformation_DATA));
    arg_table->setAddress(skybox.SkyBoxVertexBuffer->gpuAddress(), (NS::UInteger)(BUFFER_INDEX::SKYBOX_BUFFER_INDEX));
    arg_table->setAddress(skybox.MVPSkyBoxBuffer->gpuAddress(), (NS::UInteger)(BUFFER_INDEX::MVP_BUFFER_INDEX)); 

    MTL::ResourceID r_ID = grassTexture->texture->gpuResourceID();
    arg_table->setTexture(r_ID, (NS::UInteger)TEX_INDEX::COLTEXTURE_ID);

    r_ID = skybox.skyboxTexture->texture->gpuResourceID();
    arg_table->setTexture(r_ID, (NS::UInteger)TEX_INDEX::SKYTEX_TEXTURE_INDEX);

    auto *residencyDesc = MTL::ResidencySetDescriptor::alloc()->init();
    residency_set = metalDevice->newResidencySet(residencyDesc, nullptr);
    residencyDesc->release();
    residency_set->addAllocation(triangleVertexBuffer);
    residency_set->addAllocation(transformationBuffer);
    residency_set->addAllocation(grassTexture->texture);

    residency_set->addAllocation(skybox.SkyBoxVertexBuffer);
    residency_set->addAllocation(skybox.MVPSkyBoxBuffer);
    residency_set->addAllocation(skybox.skyboxTexture->texture);


    residency_set->commit();
    metal4CommandQueue->addResidencySet(residency_set);
    _mainDeletionQueue.push_function([=](){
        if (residency_set) residency_set->release();
        if (arg_table) arg_table->release();
    });

    CA::MetalLayer *metalLayerCpp = MetalViewBridge::AsMetalLayerCpp(metalLayerHandle);
    metal4CommandQueue->addResidencySet(metalLayerCpp->residencySet());
}

void MTLEngine::createRenderPipeline()
{
    using NS::StringEncoding::UTF8StringEncoding;

    MTL::PixelFormat pixelFormat = MetalViewBridge::GetPixelFormat(metalLayerHandle);
    auto *compilerDesc = MTL4::CompilerDescriptor::alloc()->init();
    metal4Compiler = metalDevice->newCompiler(compilerDesc, nullptr);
    compilerDesc->release();

    if (!metal4Compiler)
    {
        std::cerr << "newCompiler() returned null.\n";
        exit(EXIT_FAILURE);
    }

    _mainDeletionQueue.push_function([=](){metal4Compiler->release();});

    
    ShaderFunctionDescriptor cubeShaderFunctionDescriptor(shaderLibrary, "vertexShader", "fragmentShader");
    auto *cubePipelineDescriptor = MTL4::RenderPipelineDescriptor::alloc()->init(); 
    cubePipelineDescriptor->setLabel(NS::String::string("Triangle Rendering Pipeline (Metal 4)", NS::ASCIIStringEncoding));
    cubePipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(pixelFormat);
    cubePipelineDescriptor->setVertexFunctionDescriptor(cubeShaderFunctionDescriptor.vertexShaderFunctionDescriptor);
    cubePipelineDescriptor->setFragmentFunctionDescriptor(cubeShaderFunctionDescriptor.fragmentShaderFunctionDescriptor);

    NS::Error *pPipelineError = nullptr;
    metalRenderPSO = metal4Compiler->newRenderPipelineState(cubePipelineDescriptor, (MTL4::CompilerTaskOptions *)nullptr, &pPipelineError);
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

    
    
    
    ShaderFunctionDescriptor skyboxShaderFunctionDescriptor(shaderLibrary, "skyboxVertex", "skyboxFragment");
    skybox.shaderVertexFunctionDescriptor = skyboxShaderFunctionDescriptor;

    auto *skyboxPipelineDescriptor = MTL4::RenderPipelineDescriptor::alloc()->init(); 
    skyboxPipelineDescriptor->setLabel(NS::String::string("Skybox", NS::ASCIIStringEncoding));
    skyboxPipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(pixelFormat);
    skyboxPipelineDescriptor->setVertexFunctionDescriptor(skyboxShaderFunctionDescriptor.vertexShaderFunctionDescriptor);
    skyboxPipelineDescriptor->setFragmentFunctionDescriptor(skyboxShaderFunctionDescriptor.fragmentShaderFunctionDescriptor);

    pPipelineError = nullptr;
    skybox.SkyboxPSO = metal4Compiler->newRenderPipelineState(skyboxPipelineDescriptor, (MTL4::CompilerTaskOptions *)nullptr, &pPipelineError);
    if (!skybox.SkyboxPSO) 
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

    MTL::DepthStencilDescriptor *depthStencilDescriptor = MTL::DepthStencilDescriptor::alloc()->init();
    depthStencilDescriptor->setDepthCompareFunction(MTL::CompareFunctionLess);
    depthStencilDescriptor->setDepthWriteEnabled(true);
    depthStencilState = metalDevice->newDepthStencilState(depthStencilDescriptor);
    depthStencilDescriptor->release();

    cubePipelineDescriptor->release();
    skyboxPipelineDescriptor->release(); 
}

void MTLEngine::draw()
{
    sendRenderCommand();

    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    ProcessKeyboardInput(deltaTime);
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

    MVP mvp1;
    mvp1.MVP = *reinterpret_cast<matrix_float4x4*>(&MVP_GLM);
    memcpy(transformationBuffer->contents(), &mvp1, sizeof(MVP));

    glm::mat4 skyboxMVP_GLM = perspectiveMatrix * glm::mat4(glm::mat3(viewMatrix));
    MVP mvpSkybox;
    mvpSkybox.MVP = *reinterpret_cast<matrix_float4x4*>(&skyboxMVP_GLM);
    memcpy(skybox.MVPSkyBoxBuffer->contents(), &mvpSkybox, sizeof(MVP));

    CA::MetalLayer *metalLayerCpp = MetalViewBridge::AsMetalLayerCpp(metalLayerHandle);


    multiCommandBuffer.pushback_function([=](MTL4::CommandBuffer * cb){

    MTL4::RenderPassDescriptor *renderPassDescriptor = MTL4::RenderPassDescriptor::alloc()->init();
    MTL::RenderPassColorAttachmentDescriptor *cd = renderPassDescriptor->colorAttachments()->object(0);
    MTL::RenderPassDepthAttachmentDescriptor *depthAttachment = renderPassDescriptor->depthAttachment();

    depthAttachment->setTexture(depthTexture);
    depthAttachment->setLoadAction(MTL::LoadActionClear);
    depthAttachment->setStoreAction(MTL::StoreActionDontCare);
    depthAttachment->setClearDepth(1.0);
    cd->setTexture(surface->texture());
    cd->setLoadAction(MTL::LoadActionClear);
    cd->setClearColor(MTL::ClearColor(55.0f / 255.0f, 55.0f / 255.0f, 55.0f / 255.0f, 1.0));
    cd->setStoreAction(MTL::StoreActionStore);


    MTL4::RenderCommandEncoder *encoder = cb->renderCommandEncoder(renderPassDescriptor);
    encoder->setFrontFacingWinding(MTL::WindingCounterClockwise);
    encoder->setCullMode(MTL::CullModeBack);
    encoder->setLabel(NS::String::string("Triangle", NS::ASCIIStringEncoding));
    encoder->setRenderPipelineState(metalRenderPSO);
    encoder->setDepthStencilState(depthStencilState);
    encoder->setArgumentTable(arg_table, MTL::RenderStageVertex);
    encoder->setArgumentTable(arg_table, MTL::RenderStageFragment);
    encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, (NS::UInteger)0, (NS::UInteger)36);

    encoder->setRenderPipelineState(skybox.SkyboxPSO);
    encoder->setDepthStencilState(skybox.skyboxDepthStencilState);
    encoder->setArgumentTable(arg_table, MTL::RenderStageVertex);
    encoder->setArgumentTable(arg_table, MTL::RenderStageFragment);
    encoder->drawPrimitives(MTL::PrimitiveTypeTriangle,NS::UInteger(0), NS::UInteger(36));


    encoder->endEncoding();
    renderPassDescriptor->release();


    });
    metal4CommandQueue->wait(surface);
    multiCommandBuffer.Execute(cmd_alloc, metalLayerHandle, metal4CommandQueue);
    metal4CommandQueue->signalDrawable(surface);
    surface->present();
    metal4CommandQueue->signalEvent(frame_available_shared_event, frame_num);
    frame_num++;

}

void MTLEngine::encodeRenderCommand(MTL4::RenderCommandEncoder *encoder)
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

    if (glfwGetKey(glfwWindow, GLFW_KEY_M) == GLFW_PRESS) glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void MTLEngine::mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE){
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        double xpos = width / 2.0;
        double ypos = height / 2.0;
        glfwSetCursorPos(window, xpos, ypos);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

}



void MTLEngine::frameBufferSizeCallback(GLFWwindow *window, int width, int height){

    if(MTLEngine::engine){

        engine->resizeFrameBuffer(width, height);

    }

}


void MTLEngine::resizeFrameBuffer(int width, int height){

    if (depthTexture) {
        depthTexture->release();
        depthTexture = nullptr;
    }

    MTL::TextureDescriptor *TextureDescriptor = MTL::TextureDescriptor::alloc()->init();
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