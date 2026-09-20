#pragma once 
#include "Mesh.h"
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <vector>
#include "ufbx.h"
#include "Texture.hpp"
#include "stb_image.h"
#include <cstring>
#include "Material.hpp"

#include "DeletionQueue.h"

class Model{
    public:
    std::vector<Mesh*> meshes;
   

    Model() = default;
    Model(const std::string & filePath,MTL::Device*metalDevice, DeletionQueue &dq );
    void UpdateShaders(const MTL::Library *lib, DeletionQueue &dq, MTL4::Compiler *metal4Complier, const MTL::PixelFormat &pf);
    void UpdateResidency(MTL::ResidencySet *residency_set);
    void Draw(MTL4::RenderCommandEncoder *encoder, MESHMVP & mvp);

};