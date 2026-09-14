#pragma once
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

struct GLFWwindow;

// Wraps the Cocoa/CAMetalLayer side of window setup. No Objective-C syntax
// here, so this header (and anything that includes it) stays plain C++.
// Implementation lives in metal_view_bridge.mm.
namespace MetalViewBridge {

    // Creates a CAMetalLayer, attaches it to the GLFW window's NSWindow
    // content view, and returns an opaque handle to it.
    void* CreateAndAttachLayer(GLFWwindow* glfwWindow,
                                MTL::Device* device,
                                MTL::PixelFormat pixelFormat,
                                int width, int height);

    void ResizeLayer(void* layerHandle, int width, int height);

    // metal-cpp wrapper for this frame's drawable, or nullptr if none ready.
    CA::MetalDrawable* NextDrawable(void* layerHandle);

    // metal-cpp wrapper around the layer itself (for residencySet(), etc.)
    CA::MetalLayer* AsMetalLayerCpp(void* layerHandle);

    MTL::PixelFormat GetPixelFormat(void* layerHandle);

} // namespace MetalViewBridge