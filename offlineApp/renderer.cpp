#include "renderer.h"

namespace visuser
{

    Renderer::Renderer(int argc, const char **argv, const AniObjWidget &config)
    {
        // initialize OSPRay; OSPRay parses (and removes) its commandline parameters,
        // e.g. "--osp:debug"
        int argc_ = argc;
        InitializeOSPRay(argc, argv);

        // load umesh
        auto start = std::chrono::high_resolution_clock::now();
        umeshHdlPtr = umesh::io::loadBinaryUMesh(config.file_name);
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
        std::cout << "Time taken to load umesh: " << duration.count() << " milliseconds" << std::endl;
        std::cout << "found " << umeshHdlPtr->tets.size() << " tetrahedra" << std::endl;
        std::cout << "found " << umeshHdlPtr->pyrs.size() << " pyramids" << std::endl;
        std::cout << "found " << umeshHdlPtr->wedges.size() << " wedges" << std::endl;
        std::cout << "found " << umeshHdlPtr->hexes.size() << " hexahedra" << std::endl;
        std::cout << "found " << umeshHdlPtr->vertices.size() << " vertices" << std::endl;
    }

    Renderer::~Renderer()
    {
        ospShutdown();
    }

    void Renderer::InitializeOSPRay(
        int argc, const char **argv, bool errorsFatal)
    {
        // initialize OSPRay; OSPRay parses (and removes) its commandline parameters,
        // e.g. "--osp:debug"
        OSPError initError = ospInit(&argc, argv);

        if (initError != OSP_NO_ERROR)
            throw std::runtime_error("OSPRay not initialized correctly!");

        OSPDevice device = ospGetCurrentDevice();
        if (!device)
            throw std::runtime_error("OSPRay device could not be fetched!");

        // set an error callback to catch any OSPRay errors and exit the application
        if (errorsFatal)
        {
            ospDeviceSetErrorCallback(
                device,
                [](void *, OSPError error, const char *errorDetails)
                {
                    std::cerr << "OSPRay error: " << errorDetails << std::endl;
                    exit(error);
                },
                nullptr);
        }
        else
        {
            ospDeviceSetErrorCallback(
                device,
                [](void *, OSPError, const char *errorDetails)
                {
                    std::cerr << "OSPRay error: " << errorDetails << std::endl;
                },
                nullptr);
        }

        ospDeviceSetStatusCallback(
            device, [](void *, const char *msg)
            { std::cout << msg; },
            nullptr);

        bool warnAsErrors = true;
        auto logLevel = OSP_LOG_WARNING;

        ospDeviceSetParam(device, "warnAsError", OSP_BOOL, &warnAsErrors);
        ospDeviceSetParam(device, "logLevel", OSP_UINT, &logLevel);

        ospDeviceCommit(device);
        ospDeviceRelease(device);
    }

} // namespace visuser