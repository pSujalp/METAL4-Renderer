#pragma once 

#include "VertexData.hpp"
#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>
#include <QuartzCore/CAMetalLayer.h>

#include <vector>

struct PBRMaterial 
{
    MTL::Texture * base_color_texture;
    MTL::Texture * normalmap_texture;
    MTL::Texture * metallic_texture;
    MTL::Texture * roughness_texture;
    MTL::Texture * specular_texture;


};