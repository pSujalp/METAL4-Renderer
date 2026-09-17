
#pragma once
#include <simd/simd.h>
using namespace simd;

#include "VertexData.hpp"

struct Vertex {
    float3 position;
    float3 normal;
    float3 tangent;
    float3 bitangent;
    float2 textureCoordinate;
    int diffuseTextureIndex;
    int specularTextureIndex;
    int normalMapIndex;
    int emissiveMapIndex;
};






