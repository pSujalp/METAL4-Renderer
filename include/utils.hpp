#pragma once

#include <cstdio>
#include <cstdint>
#include <string>
#include <array>
#include <format>
#include <simd/simd.h>
#include <Foundation/Foundation.hpp>


using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using usize = size_t;

using f32 = float;
using f64 = double;


using vec2 = simd_float2;
using vec3 = simd_float3;
using vec4 = simd_float4;


using String = std::string;

template< typename T, size_t S >
using Array = std::array<T, S>;

#define fmt std::format

inline NS::String* ns_str( const char* c_str )
{
    return NS::String::string( c_str, NS::ASCIIStringEncoding );
}

inline NS::String* ns_str( const String str )
{
    return NS::String::string( str.c_str(), NS::ASCIIStringEncoding );
}


// defer //////////////////////////////////////////////////////////////////////
template<typename Lambda>
class Deferrable
{
public:
    Deferrable() = delete;
    Deferrable( Lambda  f ): action( f ) {}
    ~Deferrable() { action(); }
private:
    const Lambda action;
};

// Preprocessor magic so __COUNTER__ can be expanded correctly
#define CONCATENATE( l, r )         DO_CONCATENATE( l, r )
#define DO_CONCATENATE( l, r )      DO_CONCATENATE_2( l, r )
#define DO_CONCATENATE_2( l, r )    l##r

#define defer( f ) const auto CONCATENATE( _deferred, __COUNTER__ ) = Deferrable( [&](){ f; } );
