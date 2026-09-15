#include "PrimitiveVerticesData.h"


PrimitiveVerticesData::PrimitiveVerticesData(){
    CubeVertices.reserve(36);

    CubeVertices.emplace_back(VertexData{{-0.5, -0.5, 0.5, 1.0}, {0.0, 0.0}});
    CubeVertices.emplace_back(VertexData{{0.5, -0.5, 0.5, 1.0}, {1.0, 0.0}});
    CubeVertices.emplace_back(VertexData{{0.5, 0.5, 0.5, 1.0}, {1.0, 1.0}});
    CubeVertices.emplace_back(VertexData{{0.5, 0.5, 0.5, 1.0}, {1.0, 1.0}});
    CubeVertices.emplace_back(VertexData{{-0.5, 0.5, 0.5, 1.0}, {0.0, 1.0}});
    CubeVertices.emplace_back(VertexData{{-0.5, -0.5, 0.5, 1.0}, {0.0, 0.0}});

    CubeVertices.emplace_back(VertexData{{0.5, -0.5, -0.5, 1.0}, {0.0, 0.0}});
    CubeVertices.emplace_back(VertexData{{-0.5, -0.5, -0.5, 1.0}, {1.0, 0.0}});
    CubeVertices.emplace_back(VertexData{{-0.5, 0.5, -0.5, 1.0}, {1.0, 1.0}});
    CubeVertices.emplace_back(VertexData{{-0.5, 0.5, -0.5, 1.0}, {1.0, 1.0}});
    CubeVertices.emplace_back(VertexData{{0.5, 0.5, -0.5, 1.0}, {0.0, 1.0}});
    CubeVertices.emplace_back(VertexData{{0.5, -0.5, -0.5, 1.0}, {0.0, 0.0}});

    CubeVertices.emplace_back(VertexData{{-0.5, 0.5, 0.5, 1.0}, {0.0, 0.0}});
    CubeVertices.emplace_back(VertexData{{0.5, 0.5, 0.5, 1.0}, {1.0, 0.0}});
    CubeVertices.emplace_back(VertexData{{0.5, 0.5, -0.5, 1.0}, {1.0, 1.0}});
    CubeVertices.emplace_back(VertexData{{0.5, 0.5, -0.5, 1.0}, {1.0, 1.0}});
    CubeVertices.emplace_back(VertexData{{-0.5, 0.5, -0.5, 1.0}, {0.0, 1.0}});
    CubeVertices.emplace_back(VertexData{{-0.5, 0.5, 0.5, 1.0}, {0.0, 0.0}});

    CubeVertices.emplace_back(VertexData{{-0.5, -0.5, -0.5, 1.0}, {0.0, 0.0}});
    CubeVertices.emplace_back(VertexData{{0.5, -0.5, -0.5, 1.0}, {1.0, 0.0}});
    CubeVertices.emplace_back(VertexData{{0.5, -0.5, 0.5, 1.0}, {1.0, 1.0}});
    CubeVertices.emplace_back(VertexData{{0.5, -0.5, 0.5, 1.0}, {1.0, 1.0}});
    CubeVertices.emplace_back(VertexData{{-0.5, -0.5, 0.5, 1.0}, {0.0, 1.0}});
    CubeVertices.emplace_back(VertexData{{-0.5, -0.5, -0.5, 1.0}, {0.0, 0.0}});

    CubeVertices.emplace_back(VertexData{{-0.5, -0.5, -0.5, 1.0}, {0.0, 0.0}});
    CubeVertices.emplace_back(VertexData{{-0.5, -0.5, 0.5, 1.0}, {1.0, 0.0}});
    CubeVertices.emplace_back(VertexData{{-0.5, 0.5, 0.5, 1.0}, {1.0, 1.0}});
    CubeVertices.emplace_back(VertexData{{-0.5, 0.5, 0.5, 1.0}, {1.0, 1.0}});
    CubeVertices.emplace_back(VertexData{{-0.5, 0.5, -0.5, 1.0}, {0.0, 1.0}});
    CubeVertices.emplace_back(VertexData{{-0.5, -0.5, -0.5, 1.0}, {0.0, 0.0}});

    CubeVertices.emplace_back(VertexData{{0.5, -0.5, 0.5, 1.0}, {0.0, 0.0}});
    CubeVertices.emplace_back(VertexData{{0.5, -0.5, -0.5, 1.0}, {1.0, 0.0}});
    CubeVertices.emplace_back(VertexData{{0.5, 0.5, -0.5, 1.0}, {1.0, 1.0}});
    CubeVertices.emplace_back(VertexData{{0.5, 0.5, -0.5, 1.0}, {1.0, 1.0}});
    CubeVertices.emplace_back(VertexData{{0.5, 0.5, 0.5, 1.0}, {0.0, 1.0}});
    CubeVertices.emplace_back(VertexData{{0.5, -0.5, 0.5, 1.0}, {0.0, 0.0}});



    SkyboxVertices.reserve(36);

    SkyboxVertices.emplace_back(SkyboxVertexData{{-1.0f, 1.0f, -1.0f}});
    SkyboxVertices.emplace_back(SkyboxVertexData{{-1.0f, -1.0f, -1.0f}});
    SkyboxVertices.emplace_back(SkyboxVertexData{{1.0f, -1.0f, -1.0f}});
    SkyboxVertices.emplace_back(SkyboxVertexData{{1.0f, -1.0f, -1.0f}});
    SkyboxVertices.emplace_back(SkyboxVertexData{{1.0f, 1.0f, -1.0f}});
    SkyboxVertices.emplace_back(SkyboxVertexData{{-1.0f, 1.0f, -1.0f}});

    SkyboxVertices.emplace_back(SkyboxVertexData{{-1.0f, -1.0f, 1.0f}});
    SkyboxVertices.emplace_back(SkyboxVertexData{{-1.0f, -1.0f, -1.0f}});
    SkyboxVertices.emplace_back(SkyboxVertexData{{-1.0f, 1.0f, -1.0f}});
    SkyboxVertices.emplace_back(SkyboxVertexData{{-1.0f, 1.0f, -1.0f}});
    SkyboxVertices.emplace_back(SkyboxVertexData{{-1.0f, 1.0f, 1.0f}});
    SkyboxVertices.emplace_back(SkyboxVertexData{{-1.0f, -1.0f, 1.0f}});

    SkyboxVertices.emplace_back(SkyboxVertexData{{1.0f, -1.0f, -1.0f}});
    SkyboxVertices.emplace_back(SkyboxVertexData{{1.0f, -1.0f, 1.0f}});
    SkyboxVertices.emplace_back(SkyboxVertexData{{1.0f, 1.0f, 1.0f}});
    SkyboxVertices.emplace_back(SkyboxVertexData{{1.0f, 1.0f, 1.0f}});
    SkyboxVertices.emplace_back(SkyboxVertexData{{1.0f, 1.0f, -1.0f}});
    SkyboxVertices.emplace_back(SkyboxVertexData{{1.0f, -1.0f, -1.0f}});

    SkyboxVertices.emplace_back(SkyboxVertexData{{-1.0f, -1.0f, 1.0f}});
    SkyboxVertices.emplace_back(SkyboxVertexData{{-1.0f, 1.0f, 1.0f}});
    SkyboxVertices.emplace_back(SkyboxVertexData{{1.0f, 1.0f, 1.0f}});
    SkyboxVertices.emplace_back(SkyboxVertexData{{1.0f, 1.0f, 1.0f}});
    SkyboxVertices.emplace_back(SkyboxVertexData{{1.0f, -1.0f, 1.0f}});
    SkyboxVertices.emplace_back(SkyboxVertexData{{-1.0f, -1.0f, 1.0f}});

    SkyboxVertices.emplace_back(SkyboxVertexData{{-1.0f, 1.0f, -1.0f}});
    SkyboxVertices.emplace_back(SkyboxVertexData{{1.0f, 1.0f, -1.0f}});
    SkyboxVertices.emplace_back(SkyboxVertexData{{1.0f, 1.0f, 1.0f}});
    SkyboxVertices.emplace_back(SkyboxVertexData{{1.0f, 1.0f, 1.0f}});
    SkyboxVertices.emplace_back(SkyboxVertexData{{-1.0f, 1.0f, 1.0f}});
    SkyboxVertices.emplace_back(SkyboxVertexData{{-1.0f, 1.0f, -1.0f}});

    SkyboxVertices.emplace_back(SkyboxVertexData{{-1.0f, -1.0f, -1.0f}});
    SkyboxVertices.emplace_back(SkyboxVertexData{{-1.0f, -1.0f, 1.0f}});
    SkyboxVertices.emplace_back(SkyboxVertexData{{1.0f, -1.0f, -1.0f}});
    SkyboxVertices.emplace_back(SkyboxVertexData{{1.0f, -1.0f, -1.0f}});
    SkyboxVertices.emplace_back(SkyboxVertexData{{-1.0f, -1.0f, 1.0f}});
    SkyboxVertices.emplace_back(SkyboxVertexData{{1.0f, -1.0f, 1.0f}});
  }

