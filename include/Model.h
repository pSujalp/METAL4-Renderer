#pragma once

#include "Mesh.h"

class Model {


    public :

    Model(std::string str,MTL::Device * metalDevice);
    void convert_mesh_part(ufbx_mesh *mesh, ufbx_mesh_part *part);

    std::vector<Mesh*> meshes;

    MTL::Device * metalDevice;

};