#pragma once
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

struct GLFWwindow;

namespace MetalViewBridge
{

    void *CreateAndAttachLayer(GLFWwindow *glfwWindow,
                               MTL::Device *device,
                               MTL::PixelFormat pixelFormat,
                               int width, int height);

    void ResizeLayer(void *layerHandle, int width, int height);

    CA::MetalDrawable *NextDrawable(void *layerHandle);

    CA::MetalLayer *AsMetalLayerCpp(void *layerHandle);

    MTL::PixelFormat GetPixelFormat(void *layerHandle);

}