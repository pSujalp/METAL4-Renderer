#pragma once

#include <simd/simd.h>
using namespace simd;

enum class MESH_BUFFER_INDEX : uint8_t
{
    MESH_VERTEX_DATA = 0,
    MVP_DATA = 1,
};

enum class MESH_TEXTURE_INDEX : uint8_t
{
    base_color_texture,
    normalmap_texture,
    metallic_texture,
    roughness_texture,
    specular_texture
};

struct MESHMVP
{
    matrix_float4x4 MVP;
};
