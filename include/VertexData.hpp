

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
    AAPL_Vertex_DATA
};

enum class TEX_INDEX : uint8_t
{
    COLTEXTURE_ID,
    AAPL_TEX_ID
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


struct AAPLVertex {
    float2 position;
    float2 textureCoordinate;
   
};