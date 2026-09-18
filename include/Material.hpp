#pragma once

#ifdef __METAL_VERSION__
#include <metal_stdlib>
using namespace metal;

struct PBRMaterial {
    texture2d<float> base_color_texture [[texture(0)]];
    texture2d<float> normalmap_texture [[texture(1)]];
    texture2d<float> metallic_texture [[texture(2)]];
    texture2d<float> roughness_texture [[texture(3)]];
    texture2d<float> specular_texture [[texture(4)]];
};

enum class MESH_BUFFER_INDEX : uint8_t {
    MESH_VERTEX_DATA = 0,
    MVP_DATA = 1,
};

enum class MESH_MAT_INDEX : uint8_t {
    PBR_MAT = 0,
};

#else

#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>
#include <QuartzCore/CAMetalLayer.h>

#include "VertexData.hpp"
#include <vector>

struct PBRMaterial {
    MTL::Texture *base_color_texture;
    MTL::Texture *normalmap_texture;
    MTL::Texture *metallic_texture;
    MTL::Texture *roughness_texture;
    MTL::Texture *specular_texture;

    ~PBRMaterial() {
        if (base_color_texture) base_color_texture->release();
        if (normalmap_texture) normalmap_texture->release();
        if (metallic_texture) metallic_texture->release();
        if (roughness_texture) roughness_texture->release();
        if (specular_texture) specular_texture->release();
    }
};

enum class MESH_BUFFER_INDEX : uint8_t {
    MESH_VERTEX_DATA = 0,
    MVP_DATA = 1,
};

enum class MESH_MAT_INDEX : uint8_t {
    PBR_MAT = 0,
};
#endif