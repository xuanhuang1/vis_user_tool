#include "rectMesh.h"
#include <fstream>

namespace visuser
{
    RectMesh::RectMesh(rkm::vec3ui dimensions, 
        std::vector<float> z_mapping, std::string rawFilePath,
        rkm::vec2f xy_scaling)
    {
        // create vertices
        for (int k = 0; k < dimensions.z; k++)
        {
            for (int j = 0; j < dimensions.y; j++)
            {
                for (int i = 0; i < dimensions.x; i++)
                {
                    vertices.push_back(rkm::vec3f(i * xy_scaling.x, j * xy_scaling.y, z_mapping[k]));
                }
            }
        }

        // create indices in vtk hexahedron format
        for (int k = 0; k < dimensions.z - 1; k++)
        {
            for (int j = 0; j < dimensions.y -1; j++)
            {
                for (int i = 0; i < dimensions.x - 1; i++)
                {
                    //bottom half I, II, III, IV
                    indices.push_back(i + j * dimensions.x + k * dimensions.x * dimensions.y);
                    indices.push_back(i + 1 + j * dimensions.x + k * dimensions.x * dimensions.y);
                    indices.push_back(i + 1 + (j + 1) * dimensions.x + k * dimensions.x * dimensions.y); 
                    indices.push_back(i + (j + 1) * dimensions.x + k * dimensions.x * dimensions.y);
                    //top half V, VI, VII, VIII
                    indices.push_back(i + j * dimensions.x + (k + 1) * dimensions.x * dimensions.y);
                    indices.push_back(i + 1 + j * dimensions.x + (k + 1) * dimensions.x * dimensions.y);
                    indices.push_back(i + 1 + (j + 1) * dimensions.x + (k + 1) * dimensions.x * dimensions.y);
                    indices.push_back(i + (j + 1) * dimensions.x + (k + 1) * dimensions.x * dimensions.y);
                }
            }
        }

        std::ifstream inputFile(rawFilePath, std::ios::binary);
        if (!inputFile.is_open()) {
            std::cerr << "Error: Failed to open file " << rawFilePath << std::endl;
        }

        // Read 32-bit float values from the file and push them to the vector
        float scalar;
        while (inputFile.read(reinterpret_cast<char*>(&scalar), sizeof(float))) {
            scalars.push_back(scalar);
        }
        // Close the file
        inputFile.close();
    }

}