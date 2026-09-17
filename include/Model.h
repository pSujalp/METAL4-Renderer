#pragma once 
#include "Mesh.h"
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <vector>
#include "ufbx.h"
#include "Texture.hpp"
#include "stb_image.h"


class Model{
    public:
    std::vector<Mesh*> meshes;
    std::unordered_map<std::string,PBRMaterial> PBRmaterials_map;

    Model() = default;
    Model(const std::string & filePath,MTL::Device*metalDevice);

};