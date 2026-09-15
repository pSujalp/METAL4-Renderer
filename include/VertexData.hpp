

#pragma once
#include <simd/simd.h>

using namespace simd;

struct VertexData
{
    float4 position;
    float2 textureCoordinate;
};

enum class BUFFER_INDEX : uint8_t
{
    VERTEX_DATA,
    Transformation_DATA,
    SKYBOX_BUFFER_INDEX ,
    MVP_BUFFER_INDEX 
};

enum class TEX_INDEX : uint8_t
{
    COLTEXTURE_ID,
    SKYTEX_TEXTURE_INDEX 
};

struct MVP
{
    matrix_float4x4 MVP;
};

struct Uniforms
{
    float2 time;
    int intAsBool;
};

