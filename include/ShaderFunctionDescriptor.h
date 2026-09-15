#pragma once

#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>
#include <QuartzCore/CAMetalLayer.h>
#include <string>
#include <optional> 

class ShaderFunctionDescriptor
{
public:
    ShaderFunctionDescriptor(const MTL::Library* shaderLibrary,
                            const std::optional<std::string> &vertexName ,
                            const std::optional<std::string> &fragmentName)
    {
        vertexShaderFunctionDescriptor = MTL4::LibraryFunctionDescriptor::alloc()->init();
        fragmentShaderFunctionDescriptor = MTL4::LibraryFunctionDescriptor::alloc()->init();

        if (vertexName.has_value())
        {
            vertexShaderFunctionDescriptor->setLibrary(shaderLibrary);
            vertexShaderFunctionDescriptor->setName(NS::String::string(vertexName.value().c_str(), NS::ASCIIStringEncoding));
        }

        if (fragmentName.has_value())
        {
            fragmentShaderFunctionDescriptor->setLibrary(shaderLibrary);
            fragmentShaderFunctionDescriptor->setName(NS::String::string(fragmentName.value().c_str(), NS::ASCIIStringEncoding));
        }
    }

    MTL4::LibraryFunctionDescriptor* vertexShaderFunctionDescriptor = nullptr;
    MTL4::LibraryFunctionDescriptor* fragmentShaderFunctionDescriptor = nullptr;

    void cleanup()
    {
        if (vertexShaderFunctionDescriptor)
            vertexShaderFunctionDescriptor->release();
        if (fragmentShaderFunctionDescriptor)
            fragmentShaderFunctionDescriptor->release();
    }

    ~ShaderFunctionDescriptor()
    {
        cleanup();
    }
};