#include "ospray/ospray_cpp/ext/rkcommon.h"

namespace visuser
{
    namespace rkm = rkcommon::math;
    class RectMesh
    {
    public:
        RectMesh(rkm::vec3ui dimensions, 
            std::vector<float> z_mapping, std::string rawFilePath,
            rkm::vec2f xy_scaling = {1.0f, 1.0f});

        std::vector<float> scalars;
        std::vector<rkm::vec3f> vertices;
        std::vector<size_t> indices;
    private:
    };
};