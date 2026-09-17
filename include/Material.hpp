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


    ~PBRMaterial(){

        if(base_color_texture) base_color_texture->release();

        if(normalmap_texture) normalmap_texture->release();

        if(metallic_texture) metallic_texture->release();

        if(roughness_texture) roughness_texture->release();

        if(specular_texture) specular_texture->release();
    }

};