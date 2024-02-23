#include <iostream>
#include "ospray/ospray_cpp.h"
#include "ospray/ospray_cpp/ext/rkcommon.h"

#ifdef OFFLINE_WITH_UMESH
#include "umesh/io/UMesh.h"
#endif

#include "rkcommon/utility/SaveImage.h"

#include "../mesh/rectMesh.h"

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
        void SaveFramePPM(const std::string &filename);
        void SaveFramePNG(const std::string &filename);
#ifdef OFFLINE_WITH_UMESH
        void InitializeVolumeModel(
            const std::shared_ptr<umesh::UMesh> umeshPtr);
#endif
        void InitializeVolumeModel( 
            const std::shared_ptr<RectMesh> rectMeshPtr);
        void SetCamera(const Camera &cam);

        void SetTFColors(const std::vector<rkcommon::math::vec3f> colors);
        void SetTFColors(const std::vector<float> colors);
        void SetTFOpacities(const std::vector<float> opacities);
        void SetTFRange(const rkm::range1f &range);
    private:
        inline void InitializeOSPRay(int argc, const char **argv, bool errorsFatal = true);
        
        ospray::Renderer renderer;
        ospray::FrameBuffer framebuffer;
        rkm::vec2i imgSize = rkm::vec2i(1024, 1024);

        ospray::Volume volume;
        ospray::VolumetricModel model;
        ospray::Camera camera;
        ospray::World world;
        ospray::TransferFunction transferFunction;

        OSPDevice device;
    };

} // namespace visuser
