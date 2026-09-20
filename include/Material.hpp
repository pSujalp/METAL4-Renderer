#pragma once

#include <simd/simd.h>
using namespace simd;


enum class MESH_BUFFER_INDEX : uint8_t {
    MESH_VERTEX_DATA = 0,
    MVP_DATA = 1,
};



struct MESHMVP{
    matrix_float4x4 MVP;
};
