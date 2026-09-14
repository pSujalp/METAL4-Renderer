#include "Renderer.hpp"
#include "VertexData.hpp"
#include "Shader.h"
#include <unistd.h>
#include <cassert>
#include <cstring>


Renderer::Renderer(MTL::Device* pDevice)
: _pDevice(pDevice->retain())
{
    __builtin_printf("Step 1: creating command queue\n");
    _pCommandQueue = _pDevice->newCommandQueue();

    __builtin_printf("Step 2: createDefaultLibrary\n");
    createDefaultLibrary(pDevice);

    __builtin_printf("Step 3: buildShaders\n");
    buildShaders();

    __builtin_printf("Step 4: CreateCube\n");
    CreateCube();

    __builtin_printf("Step 5: constructor done\n");
}

Renderer::~Renderer()
{
    if (cubeVertexBuffer) cubeVertexBuffer->release();
    delete grassTexture;
    if (_pPSO) _pPSO->release();
    if (depthStencilState) depthStencilState->release();
    if (UniformBuffer) UniformBuffer->release();
    if (transformationBuffer) transformationBuffer->release();
    if (metallibrary) metallibrary->release();
    if (_pCommandQueue) _pCommandQueue->release();
    if (_pDevice) _pDevice->release();
}

void Renderer::createDefaultLibrary(MTL::Device* pDevice) {
    using NS::StringEncoding::UTF8StringEncoding;

    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    __builtin_printf("CWD: %s\n", cwd);

    Shader sh;
    const char* shadersrc = sh.GetShader("shaders/square.metal");
    if (!shadersrc) {
        __builtin_printf("ERROR: GetShader returned null — file not found\n");
        assert(false);
    }
    __builtin_printf("Shader source loaded, first 100 chars:\n%.100s\n", shadersrc);

    NS::Error* pError = nullptr;
    metallibrary = pDevice->newLibrary(
        NS::String::string(shadersrc, UTF8StringEncoding), nullptr, &pError
    );

    if (!metallibrary) {
        __builtin_printf("Compile FAILED: %s\n",
            pError->localizedDescription()->utf8String());
        assert(false);
    }
    __builtin_printf("Library compiled OK\n");

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
    NS::Error* pError = nullptr;

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

    MTL::RenderPipelineDescriptor* pDesc = MTL::RenderPipelineDescriptor::alloc()->init();
    pDesc->setVertexFunction(vertexShader);
    pDesc->setFragmentFunction(fragmentShader);
    pDesc->colorAttachments()->object(0)->setPixelFormat(
        MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB
    );

    // FIX: this was commented out. Unlike Metal 4's pipeline descriptor,
    // the classic MTL::RenderPipelineDescriptor DOES require the depth
    // attachment pixel format to be set, and it must match whatever
    // pixel format the MTKView driving this renderer uses for
    // `depthStencilPixelFormat` (set wherever the view itself is
    // configured). Without this, depth testing against the view's
    // depth attachment is undefined/likely broken even though
    // setDepthStencilState is called correctly at draw time.
    pDesc->setDepthAttachmentPixelFormat(MTL::PixelFormatDepth32Float);

    MTL::DepthStencilDescriptor* depthStencilDescriptor =
        MTL::DepthStencilDescriptor::alloc()->init();
    depthStencilDescriptor->setDepthCompareFunction(MTL::CompareFunctionLessEqual);
    depthStencilDescriptor->setDepthWriteEnabled(true);
    depthStencilState = _pDevice->newDepthStencilState(depthStencilDescriptor);
    depthStencilDescriptor->release();

    _pPSO = _pDevice->newRenderPipelineState(pDesc, &pError);
    if (!_pPSO) {
        __builtin_printf("%s", pError->localizedDescription()->utf8String());
        assert(false);
    }

    UniformBuffer       = _pDevice->newBuffer(sizeof(Uniforms), MTL::ResourceStorageModeShared);
    transformationBuffer = _pDevice->newBuffer(sizeof(MVP),     MTL::ResourceStorageModeShared);

    fragmentShader->release();
    vertexShader->release();
    pDesc->release();
}

void Renderer::draw(MTK::View* pView)
{
    NS::AutoreleasePool* pPool = NS::AutoreleasePool::alloc()->init();


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
    MTL::CommandBuffer* pCmd = _pCommandQueue->commandBuffer();
    if (!pCmd) {
        __builtin_printf("ERROR: commandBuffer is nil\n");
        pPool->release();
        return;
    }

    MTL::RenderPassDescriptor* pRpd = pView->currentRenderPassDescriptor();
    if (!pRpd) {
        __builtin_printf("ERROR: currentRenderPassDescriptor is nil\n");
        pCmd->commit();
        pPool->release();
        return;
    }

    MTL::RenderCommandEncoder* pEnc = pCmd->renderCommandEncoder(pRpd);
    if (!pEnc) {
        __builtin_printf("ERROR: renderCommandEncoder is nil\n");
        pCmd->commit();
        pPool->release();
        return;
    }

    pEnc->setRenderPipelineState(_pPSO);
    pEnc->setDepthStencilState(depthStencilState);
    // Explicit winding/cull mode (cube data below is CCW-front). Metal's
    // encoder defaults are CullModeNone / WindingClockwise; setting these
    // explicitly avoids relying on defaults and matches Apple's own
    // reference renderers.
    pEnc->setFrontFacingWinding(MTL::WindingCounterClockwise);
    pEnc->setCullMode(MTL::CullModeBack);

    pEnc->setVertexBuffer(cubeVertexBuffer,      0, 0);
    pEnc->setVertexBuffer(UniformBuffer,         0, 1);
    pEnc->setVertexBuffer(transformationBuffer,  0, 2);

    pEnc->setFragmentTexture(grassTexture->texture, 0);

    pEnc->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(36));

    pEnc->endEncoding();
    pCmd->presentDrawable(pView->currentDrawable());
    pCmd->commit();

    pPool->release();
}