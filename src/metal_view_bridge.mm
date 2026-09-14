#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>

#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "metal_view_bridge.h"

namespace MetalViewBridge {

void* CreateAndAttachLayer(GLFWwindow* glfwWindow, MTL::Device* device,
                            MTL::PixelFormat pixelFormat, int width, int height) {
    NSWindow* nsWindow = glfwGetCocoaWindow(glfwWindow);

    CAMetalLayer* layer = [CAMetalLayer layer];
    layer.device = (__bridge id<MTLDevice>)device;
    layer.pixelFormat = (MTLPixelFormat)pixelFormat;
    layer.drawableSize = CGSizeMake(width, height);

    nsWindow.contentView.layer = layer;
    nsWindow.contentView.wantsLayer = YES;

    return (__bridge_retained void*)layer; // balance with a Release() if you ever tear it down
}

void ResizeLayer(void* layerHandle, int width, int height) {
    CAMetalLayer* layer = (__bridge CAMetalLayer*)layerHandle;
    layer.drawableSize = CGSizeMake(width, height);
}

CA::MetalDrawable* NextDrawable(void* layerHandle) {
    CAMetalLayer* layer = (__bridge CAMetalLayer*)layerHandle;
    id<CAMetalDrawable> drawable = [layer nextDrawable];
    return (__bridge CA::MetalDrawable*)drawable;
}

CA::MetalLayer* AsMetalLayerCpp(void* layerHandle) {
    return (__bridge CA::MetalLayer*)layerHandle;
}

MTL::PixelFormat GetPixelFormat(void* layerHandle) {
    CAMetalLayer* layer = (__bridge CAMetalLayer*)layerHandle;
    return (MTL::PixelFormat)layer.pixelFormat;
}

}