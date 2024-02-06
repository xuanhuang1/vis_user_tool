#include <iostream>
#include "ospray/ospray_cpp.h"
#include "ospray/ospray_cpp/ext/rkcommon.h"

#include "umesh/io/UMesh.h"
#include "rkcommon/utility/SaveImage.h"

#include "../loader.h"

namespace visuser
{

    // ospray based volume renderer
    class Renderer
    {
    public:
        Renderer(const AniObjWidget &config);
        ~Renderer();

        void Render();
        void TakeSnapShot(const std::string &filename);

    private:
        inline void InitializeOSPRay(int argc, const char **argv, bool errorsFatal = true);
        std::shared_ptr<umesh::UMesh> umeshHdlPtr;
    };

} // namespace visuser
