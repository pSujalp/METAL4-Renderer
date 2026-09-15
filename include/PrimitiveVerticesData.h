#include "VertexData.hpp"
#include <vector>

class PrimitiveVerticesData
{
public:
  
  PrimitiveVerticesData();
  std::vector<VertexData> CubeVertices ;
  std::vector<SkyboxVertexData> SkyboxVertices;
};