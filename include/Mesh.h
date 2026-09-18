#pragma once

#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>
#include <QuartzCore/CAMetalLayer.h>
#include "VertexData.hpp"
#include <simd/simd.h>

#include "ufbx.h"

using namespace simd;
#include <vector>




class Mesh{

    public:
    Mesh() = default;
    Mesh(std::string filepath);
    ~Mesh();

    std::vector<uint32_t> indices;
    std::vector<VertexData> vertices;

    MTL::Buffer * vertexBufferdata;
    MTL::Buffer * indexBufferData;




};