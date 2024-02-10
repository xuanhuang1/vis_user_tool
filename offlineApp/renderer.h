#include <iostream>
#include "ospray/ospray_cpp.h"
#include "ospray/ospray_cpp/ext/rkcommon.h"

#include "umesh/io/UMesh.h"
#include "rkcommon/utility/SaveImage.h"

#include "../loader.h"

namespace visuser
{
    namespace rkm = rkcommon::math;
    namespace ospray = ospray::cpp;
    // ospray based volume renderer
    class Renderer
    {
    public:
        Renderer(const AniObjWidget &config);
        ~Renderer();

        void Render();
        void SaveFrame(const std::string &filename);
        void InitializeVolumeModel(
            const std::shared_ptr<umesh::UMesh> umeshPtr);

    private:
        inline void InitializeOSPRay(int argc, const char **argv, bool errorsFatal = true);
        
        ospray::Renderer renderer;
        ospray::FrameBuffer framebuffer;
        rkm::vec2i imgSize = rkm::vec2i(1024, 1024);

        ospray::Volume volume;
        ospray::VolumetricModel model;
        ospray::Camera camera;
        ospray::World world;

        OSPDevice device;
    };

} // namespace visuser
