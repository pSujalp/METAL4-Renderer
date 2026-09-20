#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>


class Sphere{

        public:


        uint32_t indexCount =0;
        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;

        const unsigned int X_SEGMENTS = 64 ;    
        const unsigned int Y_SEGMENTS = 64 ;
        const float PI = 3.14159265359f;

        Sphere()
        {
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
            for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                positions.push_back(glm::vec3(xPos, yPos, zPos));
                uv.push_back(glm::vec2(xSegment, ySegment));
                normals.push_back(glm::vec3(xPos, yPos, zPos));
            }
        }

        bool oddRow = false;
        for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
        {
            if (!oddRow) 
            {
                for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
                {
                    indices.push_back(y       * (X_SEGMENTS + 1) + x);
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                }
            }
            else
            {
                for (int x = X_SEGMENTS; x >= 0; --x)
                {
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    indices.push_back(y       * (X_SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;
        }
        indexCount = static_cast<unsigned int>(indices.size());



}


     Sphere(int radius , int latitudes, int longitudes){
       if(longitudes < 3)
        longitudes = 3;
    if(latitudes < 2)
        latitudes = 2;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uv;
    std::vector<unsigned int> indices;

    float nx, ny, nz, lengthInv = 1.0f / radius;    
    
    struct Vertex
    {
        float x, y, z, s, t; 
    };

    float deltaLatitude = M_PI / latitudes;
    float deltaLongitude = 2 * M_PI / longitudes;
    float latitudeAngle;
    float longitudeAngle;

    
    for (int i = 0; i <= latitudes; ++i)
    {
        latitudeAngle = M_PI / 2 - i * deltaLatitude;
        float xy = radius * cosf(latitudeAngle);   
        float z = radius * sinf(latitudeAngle);    

       
        for (int j = 0; j <= longitudes; ++j)
        {
            longitudeAngle = j * deltaLongitude;

            Vertex vertex;
            vertex.x = xy * cosf(longitudeAngle);      
            vertex.y = xy * sinf(longitudeAngle);      
            vertex.z = z;                              
            vertex.s = (float)j/longitudes;            
            vertex.t = (float)i/latitudes;             
            vertices.push_back(glm::vec3(vertex.x, vertex.y, vertex.z));
            uv.push_back(glm::vec2(vertex.s, vertex.t));

            
            nx = vertex.x * lengthInv;
            ny = vertex.y * lengthInv;
            nz = vertex.z * lengthInv;
            normals.push_back(glm::vec3(nx, ny, nz));
        }
    }


    unsigned int k1, k2;
    for(int i = 0; i < latitudes; ++i)
    {
        k1 = i * (longitudes + 1);
        k2 = k1 + longitudes + 1;
        
        for(int j = 0; j < longitudes; ++j, ++k1, ++k2)
        {
            if (i != 0)
            {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (latitudes - 1))
            {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    this->indexCount = indices.size();
    this->positions = vertices;
    this->normals = normals;
    this->uv = uv;
    this->indices = indices;
    
}
    




};