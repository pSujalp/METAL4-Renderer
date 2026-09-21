#pragma once

#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>
#include <QuartzCore/CAMetalLayer.h>

#include "Texture.hpp"
#include <cstring>
#include <string>
#include <vector>
#include "Material.hpp"
#include "VertexData.hpp"
#include <iostream>
#include "magic_enum/magic_enum.hpp"
#include "DeletionQueue.h"
#include "ShaderFunctionDescriptor.h"

// Non-owning view of a material's textures.
// Whoever creates the Texture objects is responsible for freeing them.
struct PBR_Mat
{
    Texture *base_color_texture = nullptr;
    Texture *normalmap_texture  = nullptr;
    Texture *metallic_texture   = nullptr;
    Texture *roughness_texture  = nullptr;
    Texture *specular_texture   = nullptr;

    PBR_Mat() = default;
    ~PBR_Mat() = default;
};

class Mesh
{
public:
    Mesh() = default;

    Mesh(std::vector<VertexData> &meshv, const std::string &material_name,
         const std::vector<uint32_t> &indices, MTL::Device *metalDevice, DeletionQueue &dq);

    void UpdateShaders(const MTL::Library *lib, DeletionQueue &dq,
                       MTL4::Compiler *metal4Complier, const MTL::PixelFormat &pf);

    

    void UpdateResidency(MTL::ResidencySet *residency_set);

    void Draw(MTL4::RenderCommandEncoder *encoder, const MESHMVP &mvp, const PBR_Mat *pbr_mat);

public:
    MTL::Device *metalDevice = nullptr;

    MTL::Buffer *index_Data = nullptr;
    unsigned long indexCount = 0;

    MTL::Buffer *Mesh_Data = nullptr;
    MTL::Buffer *MVP_Data = nullptr;

    std::string material_name;

    Texture *base_color_texture = nullptr;
    Texture *normalmap_texture  = nullptr;
    Texture *metallic_texture   = nullptr;
    Texture *roughness_texture  = nullptr;
    Texture *specular_texture   = nullptr;

    MTL::RenderPipelineState *MeshPSO = nullptr;
    MTL::DepthStencilState *MeshDepthStencilState = nullptr;
    MTL4::ArgumentTable *Mesharg_table = nullptr;
};