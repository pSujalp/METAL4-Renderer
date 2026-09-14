#include "Renderer.hpp"
#include "VertexData.hpp"
#include <cassert>
#include <cstring>

// Buffer/texture indices used by shaders/square.metal, bound through the
// MTL4 argument table instead of setVertexBuffer/setFragmentTexture.
namespace {
    constexpr NS::UInteger kBufferIndexVertex          = 0;
    constexpr NS::UInteger kBufferIndexUniforms        = 1;
    constexpr NS::UInteger kBufferIndexTransformation  = 2;
    constexpr NS::UInteger kTextureIndexColor          = 0;
}

Renderer::Renderer(MTL::Device* pDevice)
: _pDevice(pDevice->retain())
{
    __builtin_printf("Step 1: creating Metal 4 command queue\n");
    _pCommandQueue4 = _pDevice->newMTL4CommandQueue();
    if (!_pCommandQueue4) {
        __builtin_printf("ERROR: newMTL4CommandQueue() returned null -- this device/OS doesn't support Metal 4.\n");
        assert(false);
    }

    for (auto& alloc : _cmdAllocators) {
        alloc = _pDevice->newCommandAllocator();
    }
    _pCommandBuffer = _pDevice->newCommandBuffer();

    _frameEvent = _pDevice->newSharedEvent();
    _frameEvent->setSignaledValue(0);

    __builtin_printf("Step 2: createDefaultLibrary\n");
    createDefaultLibrary(pDevice);

    __builtin_printf("Step 3: buildShaders\n");
    buildShaders();

    __builtin_printf("Step 4: CreateCube\n");
    CreateCube();

    __builtin_printf("Step 5: argument table + residency set\n");
    auto* argTableDesc = MTL4::ArgumentTableDescriptor::alloc()->init();
    argTableDesc->setMaxBufferBindCount(3);
    argTableDesc->setMaxTextureBindCount(1);
    _argTable = _pDevice->newArgumentTable(argTableDesc, nullptr);
    argTableDesc->release();
    if (!_argTable) {
        __builtin_printf("ERROR: newArgumentTable() returned null.\n");
        assert(false);
    }

    _argTable->setAddress(cubeVertexBuffer->gpuAddress(), kBufferIndexVertex);
    _argTable->setAddress(UniformBuffer->gpuAddress(), kBufferIndexUniforms);
    _argTable->setAddress(transformationBuffer->gpuAddress(), kBufferIndexTransformation);
    _argTable->setTexture(grassTexture->texture->gpuResourceID(), kTextureIndexColor);

    auto* residencyDesc = MTL::ResidencySetDescriptor::alloc()->init();
    _residencySet = _pDevice->newResidencySet(residencyDesc, nullptr);
    residencyDesc->release();

    _residencySet->addAllocation(cubeVertexBuffer);
    _residencySet->addAllocation(UniformBuffer);
    _residencySet->addAllocation(transformationBuffer);
    _residencySet->addAllocation(grassTexture->texture);
    _residencySet->commit();
    _pCommandQueue4->addResidencySet(_residencySet);

    __builtin_printf("Step 6: constructor done\n");
}

Renderer::~Renderer()
{
    if (depthTexture) depthTexture->release();
    if (_residencySet) _residencySet->release();
    if (_argTable) _argTable->release();
    for (auto* alloc : _cmdAllocators) {
        if (alloc) alloc->release();
    }
    if (_frameEvent) _frameEvent->release();
    if (_pCommandBuffer) _pCommandBuffer->release();
    if (_pCommandQueue4) _pCommandQueue4->release();
    if (_pCompiler) _pCompiler->release();

    if (cubeVertexBuffer) cubeVertexBuffer->release();
    delete grassTexture;
    if (_pPSO) _pPSO->release();
    if (depthStencilState) depthStencilState->release();
    if (UniformBuffer) UniformBuffer->release();
    if (transformationBuffer) transformationBuffer->release();
    if (metallibrary) metallibrary->release();
    if (_pDevice) _pDevice->release();
}

void Renderer::createDefaultLibrary(MTL::Device* pDevice) {
    metallibrary = pDevice->newDefaultLibrary();
    if (!metallibrary) {
        __builtin_printf("ERROR: newDefaultLibrary() returned null -- "
                          "make sure your .metal shader files are included "
                          "in the target's build phases so they compile "
                          "into default.metallib.\n");
        assert(false);
    }
    __builtin_printf("Default library loaded OK\n");

    NS::Array* fnames = metallibrary->functionNames();
    __builtin_printf("Function count: %lu\n", fnames->count());
    for (uint32_t i = 0; i < fnames->count(); ++i) {
        __builtin_printf("  fn: %s\n", ((NS::String*)fnames->object(i))->utf8String());
    }
}

void Renderer::CreateCube() {
    VertexData cubeVertices[] = {

        {{-0.5, -0.5,  0.5, 1.0}, {0.0, 0.0}},
        {{ 0.5, -0.5,  0.5, 1.0}, {1.0, 0.0}},
        {{ 0.5,  0.5,  0.5, 1.0}, {1.0, 1.0}},
        {{ 0.5,  0.5,  0.5, 1.0}, {1.0, 1.0}},
        {{-0.5,  0.5,  0.5, 1.0}, {0.0, 1.0}},
        {{-0.5, -0.5,  0.5, 1.0}, {0.0, 0.0}},


        {{ 0.5, -0.5, -0.5, 1.0}, {0.0, 0.0}},
        {{-0.5, -0.5, -0.5, 1.0}, {1.0, 0.0}},
        {{-0.5,  0.5, -0.5, 1.0}, {1.0, 1.0}},
        {{-0.5,  0.5, -0.5, 1.0}, {1.0, 1.0}},
        {{ 0.5,  0.5, -0.5, 1.0}, {0.0, 1.0}},
        {{ 0.5, -0.5, -0.5, 1.0}, {0.0, 0.0}},


        {{-0.5,  0.5,  0.5, 1.0}, {0.0, 0.0}},
        {{ 0.5,  0.5,  0.5, 1.0}, {1.0, 0.0}},
        {{ 0.5,  0.5, -0.5, 1.0}, {1.0, 1.0}},
        {{ 0.5,  0.5, -0.5, 1.0}, {1.0, 1.0}},
        {{-0.5,  0.5, -0.5, 1.0}, {0.0, 1.0}},
        {{-0.5,  0.5,  0.5, 1.0}, {0.0, 0.0}},


        {{-0.5, -0.5, -0.5, 1.0}, {0.0, 0.0}},
        {{ 0.5, -0.5, -0.5, 1.0}, {1.0, 0.0}},
        {{ 0.5, -0.5,  0.5, 1.0}, {1.0, 1.0}},
        {{ 0.5, -0.5,  0.5, 1.0}, {1.0, 1.0}},
        {{-0.5, -0.5,  0.5, 1.0}, {0.0, 1.0}},
        {{-0.5, -0.5, -0.5, 1.0}, {0.0, 0.0}},


        {{-0.5, -0.5, -0.5, 1.0}, {0.0, 0.0}},
        {{-0.5, -0.5,  0.5, 1.0}, {1.0, 0.0}},
        {{-0.5,  0.5,  0.5, 1.0}, {1.0, 1.0}},
        {{-0.5,  0.5,  0.5, 1.0}, {1.0, 1.0}},
        {{-0.5,  0.5, -0.5, 1.0}, {0.0, 1.0}},
        {{-0.5, -0.5, -0.5, 1.0}, {0.0, 0.0}},


        {{ 0.5, -0.5,  0.5, 1.0}, {0.0, 0.0}},
        {{ 0.5, -0.5, -0.5, 1.0}, {1.0, 0.0}},
        {{ 0.5,  0.5, -0.5, 1.0}, {1.0, 1.0}},
        {{ 0.5,  0.5, -0.5, 1.0}, {1.0, 1.0}},
        {{ 0.5,  0.5,  0.5, 1.0}, {0.0, 1.0}},
        {{ 0.5, -0.5,  0.5, 1.0}, {0.0, 0.0}},
    };

    cubeVertexBuffer = _pDevice->newBuffer(
        &cubeVertices, sizeof(cubeVertices), MTL::ResourceStorageModeShared
    );
    cubeVertexBuffer->setLabel(NS::String::string("Cube Vertex Buffer", NS::ASCIIStringEncoding));
    grassTexture = new Texture("assets/mc_grass.jpeg", _pDevice);
}

void Renderer::buildShaders()
{
    MTL::Function* vertexShader = metallibrary->newFunction(
        NS::String::string("vertexShader", NS::ASCIIStringEncoding)
    );
    if (!vertexShader) {
        __builtin_printf("ERROR: vertex function 'vertexShader' not found in library.\n");
        assert(false);
    }

    MTL::Function* fragmentShader = metallibrary->newFunction(
        NS::String::string("fragmentShader", NS::ASCIIStringEncoding)
    );
    if (!fragmentShader) {
        __builtin_printf("ERROR: fragment function 'fragmentShader' not found in library.\n");
        assert(false);
    }

    auto* compilerDesc = MTL4::CompilerDescriptor::alloc()->init();
    _pCompiler = _pDevice->newCompiler(compilerDesc, nullptr);
    compilerDesc->release();
    if (!_pCompiler) {
        __builtin_printf("ERROR: newCompiler() returned null.\n");
        assert(false);
    }

    auto* vertexFunctionDescriptor = MTL4::LibraryFunctionDescriptor::alloc()->init();
    vertexFunctionDescriptor->setLibrary(metallibrary);
    vertexFunctionDescriptor->setName(NS::String::string("vertexShader", NS::ASCIIStringEncoding));
    auto* fragmentFunctionDescriptor = MTL4::LibraryFunctionDescriptor::alloc()->init();
    fragmentFunctionDescriptor->setLibrary(metallibrary);
    fragmentFunctionDescriptor->setName(NS::String::string("fragmentShader", NS::ASCIIStringEncoding));

    MTL4::RenderPipelineDescriptor* pDesc = MTL4::RenderPipelineDescriptor::alloc()->init();
    pDesc->setLabel(NS::String::string("Cube Pipeline (Metal 4)", NS::ASCIIStringEncoding));
    pDesc->setVertexFunctionDescriptor(vertexFunctionDescriptor);
    pDesc->setFragmentFunctionDescriptor(fragmentFunctionDescriptor);
    pDesc->colorAttachments()->object(0)->setPixelFormat(
        MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB
    );
    // NOTE: unlike the classic MTL::RenderPipelineDescriptor, MTL4's
    // pipeline descriptor has no depthAttachmentPixelFormat field at all --
    // the depth format comes from the depth texture attached to the render
    // pass at draw time (see draw() below), not from the pipeline. Make
    // sure the MTKView backing this renderer is still configured with
    // view.depthStencilPixelFormat = MTLPixelFormatDepth32Float so its
    // currentMTL4RenderPassDescriptor produces a matching depth attachment.

    MTL::DepthStencilDescriptor* depthStencilDescriptor =
        MTL::DepthStencilDescriptor::alloc()->init();
    depthStencilDescriptor->setDepthCompareFunction(MTL::CompareFunctionLessEqual);
    depthStencilDescriptor->setDepthWriteEnabled(true);
    depthStencilState = _pDevice->newDepthStencilState(depthStencilDescriptor);
    depthStencilDescriptor->release();

    NS::Error* pPipelineError = nullptr;
    _pPSO = _pCompiler->newRenderPipelineState(pDesc, (MTL4::CompilerTaskOptions*)nullptr, &pPipelineError);
    if (!_pPSO) {
        if (pPipelineError) {
            __builtin_printf("Pipeline compile error: %s\n", pPipelineError->localizedDescription()->utf8String());
        } else {
            __builtin_printf("ERROR: newRenderPipelineState() returned null (no error object).\n");
        }
        assert(false);
    }

    UniformBuffer        = _pDevice->newBuffer(sizeof(Uniforms), MTL::ResourceStorageModeShared);
    transformationBuffer = _pDevice->newBuffer(sizeof(MVP),      MTL::ResourceStorageModeShared);

    pDesc->release();
    vertexFunctionDescriptor->release();
    fragmentFunctionDescriptor->release();
    fragmentShader->release();
    vertexShader->release();
}

void Renderer::ensureDepthTexture(double width, double height)
{
    if (depthTexture && width == _depthTexWidth && height == _depthTexHeight) {
        return;
    }

    MTL::Texture* oldDepthTexture = depthTexture;

    MTL::TextureDescriptor* depthDesc = MTL::TextureDescriptor::alloc()->init();
    depthDesc->setTextureType(MTL::TextureType2D);
    depthDesc->setPixelFormat(MTL::PixelFormatDepth32Float);
    depthDesc->setWidth((NS::UInteger)width);
    depthDesc->setHeight((NS::UInteger)height);
    depthDesc->setUsage(MTL::TextureUsageRenderTarget);
    depthDesc->setStorageMode(MTL::StorageModePrivate);

    depthTexture = _pDevice->newTexture(depthDesc);
    depthDesc->release();

    if (!depthTexture) {
        __builtin_printf("ERROR: Failed to create depth texture.\n");
        assert(false);
    }
    depthTexture->setLabel(NS::String::string("Depth Texture", NS::ASCIIStringEncoding));

    _depthTexWidth = width;
    _depthTexHeight = height;

    // Keep the residency set in sync: drop the old-sized depth texture
    // (if any) and add the new one, then recommit.
    if (oldDepthTexture) {
        _residencySet->removeAllocation(oldDepthTexture);
    }
    _residencySet->addAllocation(depthTexture);
    _residencySet->commit();

    if (oldDepthTexture) {
        oldDepthTexture->release();
    }
}

void Renderer::draw(MTK::View* pView)
{
    NS::AutoreleasePool* pPool = NS::AutoreleasePool::alloc()->init();

    const size_t frameIdx = _frameNum % kMaxFramesInFlight;
    if (_frameNum >= kMaxFramesInFlight) {
        _frameEvent->waitUntilSignaledValue(_frameNum - kMaxFramesInFlight, UINT64_MAX);
    }

    MTL4::CommandAllocator* cmdAlloc = _cmdAllocators[frameIdx];
    cmdAlloc->reset();

    Uniforms uniforms;
    uniforms.time = {0.1f, 0.3f};
    memcpy(UniformBuffer->contents(), &uniforms, sizeof(Uniforms));

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 1.0f, -1.0f));
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));

    static float accumulatedDegrees = 0.0f;
    const float rotationSpeedDegreesPerSecond = 45.0f;
    accumulatedDegrees += rotationSpeedDegreesPerSecond * Time::DeltaTime;
    if (accumulatedDegrees >= 360.0f)
        accumulatedDegrees -= 360.0f;

    float angleInRadians = accumulatedDegrees * (M_PI / 180.0f);
    model = glm::rotate(model, angleInRadians, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 viewMatrix = glm::lookAt(
        glm::vec3(0.0f, 0.0f,  5.0f),
        glm::vec3(0.0f, 0.0f,  0.0f),
        glm::vec3(0.0f, 1.0f,  0.0f)
    );

    auto drawableSize = pView->drawableSize();
    float aspectRatio = (float)drawableSize.width / (float)drawableSize.height;
    float fov  = glm::radians(60.0f);
    float nearZ = 0.1f;
    float farZ  = 100.0f;
    glm::mat4 perspectiveMatrix = glm::perspective(fov, aspectRatio, nearZ, farZ);
    glm::mat4 MVP_GLM = perspectiveMatrix * viewMatrix * model;

    MVP mvp1;
    mvp1.MVP = matrix_float4x4({
        simd::float4{ MVP_GLM[0][0], MVP_GLM[0][1], MVP_GLM[0][2], MVP_GLM[0][3] },
        simd::float4{ MVP_GLM[1][0], MVP_GLM[1][1], MVP_GLM[1][2], MVP_GLM[1][3] },
        simd::float4{ MVP_GLM[2][0], MVP_GLM[2][1], MVP_GLM[2][2], MVP_GLM[2][3] },
        simd::float4{ MVP_GLM[3][0], MVP_GLM[3][1], MVP_GLM[3][2], MVP_GLM[3][3] },
    });
    memcpy(transformationBuffer->contents(), &mvp1, sizeof(MVP));

    // MTK::View has no MTL4-specific render pass convenience in this
    // binding, so build the MTL4::RenderPassDescriptor by hand from the
    // current drawable, same as mtl_engine.cpp did with its own
    // manually-managed depth texture.
    CA::MetalDrawable* drawable = pView->currentDrawable();
    if (!drawable) {
        __builtin_printf("ERROR: currentDrawable is nil -- skipping this frame.\n");
        pPool->release();
        return;
    }

    ensureDepthTexture(drawableSize.width, drawableSize.height);

    MTL4::RenderPassDescriptor* pRpd = MTL4::RenderPassDescriptor::alloc()->init();

    MTL::RenderPassColorAttachmentDescriptor* cd = pRpd->colorAttachments()->object(0);
    cd->setTexture(drawable->texture());
    cd->setLoadAction(MTL::LoadActionClear);
    cd->setClearColor(MTL::ClearColor(41.0f / 255.0f, 42.0f / 255.0f, 48.0f / 255.0f, 1.0));
    cd->setStoreAction(MTL::StoreActionStore);

    MTL::RenderPassDepthAttachmentDescriptor* dd = pRpd->depthAttachment();
    dd->setTexture(depthTexture);
    dd->setLoadAction(MTL::LoadActionClear);
    dd->setClearDepth(1.0);
    dd->setStoreAction(MTL::StoreActionDontCare);

    _pCommandBuffer->beginCommandBuffer(cmdAlloc);

    MTL4::RenderCommandEncoder* pEnc = _pCommandBuffer->renderCommandEncoder(pRpd);
    if (!pEnc) {
        __builtin_printf("ERROR: renderCommandEncoder is nil\n");
        _pCommandBuffer->endCommandBuffer();
        pRpd->release();
        pPool->release();
        return;
    }

    pEnc->setRenderPipelineState(_pPSO);
    pEnc->setDepthStencilState(depthStencilState);
    pEnc->setFrontFacingWinding(MTL::WindingCounterClockwise);
    pEnc->setCullMode(MTL::CullModeBack);
    pEnc->setArgumentTable(_argTable, MTL::RenderStageVertex);
    pEnc->setArgumentTable(_argTable, MTL::RenderStageFragment);

    pEnc->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(36));

    pEnc->endEncoding();

    // Same fix from the mtl_engine conversation: the drawable/layer's own
    // residency set must be attached per-command-buffer via
    // useResidencySet, not just once on the queue at setup, or the
    // drawable's backing texture can silently fail to be resident.
    // CA::MetalDrawable exposes layer() directly, unlike MTK::View, so
    // get it from the drawable instead.
    CA::MetalLayer* metalLayer = drawable->layer();
    if (metalLayer) {
        _pCommandBuffer->useResidencySet(metalLayer->residencySet());
    }

    _pCommandBuffer->endCommandBuffer();

    _pCommandQueue4->wait(drawable);
    _pCommandQueue4->commit(&_pCommandBuffer, 1);
    _pCommandQueue4->signalDrawable(drawable);
    drawable->present();

    _pCommandQueue4->signalEvent(_frameEvent, _frameNum);
    _frameNum++;

    pRpd->release();
    pPool->release();
}