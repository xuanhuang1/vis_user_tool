#include "renderer.h"

// temporary
std::string tfColorMap{"jet"};
std::string tfOpacityMap{"linear"};
ospray::cpp::TransferFunction makeTransferFunction(
    const rkcommon::math::range1f &valueRange)
{
    ospray::cpp::TransferFunction transferFunction("piecewiseLinear");

    std::vector<rkcommon::math::vec3f> colors;
    std::vector<float> opacities;

    if (tfColorMap == "jet")
    {
        colors.emplace_back(0, 0, 0.562493);
        colors.emplace_back(0, 0, 1);
        colors.emplace_back(0, 1, 1);
        colors.emplace_back(0.500008, 1, 0.500008);
        colors.emplace_back(1, 1, 0);
        colors.emplace_back(1, 0, 0);
        colors.emplace_back(0.500008, 0, 0);
    }
    else if (tfColorMap == "rgb")
    {
        colors.emplace_back(0, 0, 1);
        colors.emplace_back(0, 1, 0);
        colors.emplace_back(1, 0, 0);
    }
    else
    {
        colors.emplace_back(0.f, 0.f, 0.f);
        colors.emplace_back(1.f, 1.f, 1.f);
    }

    if (tfOpacityMap == "linear")
    {
        opacities.emplace_back(0.f);
        opacities.emplace_back(1.f);
    }
    else if (tfOpacityMap == "linearInv")
    {
        opacities.emplace_back(1.f);
        opacities.emplace_back(0.f);
    }
    else if (tfOpacityMap == "opaque")
    {
        opacities.emplace_back(1.f);
    }

    transferFunction.setParam("color", ospray::cpp::CopiedData(colors));
    transferFunction.setParam("opacity", ospray::cpp::CopiedData(opacities));
    transferFunction.setParam("value", valueRange);
    transferFunction.commit();

    return transferFunction;
}

namespace visuser
{

    Renderer::Renderer(const AniObjWidget &config)
    {
        // initialize OSPRay; OSPRay parses (and removes) its commandline parameters,
        // e.g. "--osp:debug"
        const char *argv[] = {"vistool_osp_offline"};
        InitializeOSPRay(1, argv);

        ospray::cpp::Renderer renderer("scivis");
        renderer.setParam("aoSamples", 0);
        renderer.commit();

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

        std::vector<rkcommon::math::vec3f> vertices;
        // iterate over vertices and copy them into a vector
        for (auto &v : umeshHdlPtr->vertices)
        {
            vertices.push_back(rkcommon::math::vec3f(v[0], v[1], v[2]));
        }

        ospray::cpp::Volume volume("unstructured");

        // set data objects for volume object
        volume.setParam("vertex.position", ospray::cpp::CopiedData(vertices));

        volume.setParam("vertex.data", ospray::cpp::CopiedData(umeshHdlPtr->perVertex->values));

        // define cell offsets in indices array
        std::vector<unsigned long> cells = {
            0,                                                                                 // tets
            umeshHdlPtr->tets.size(),                                                          // pyrs
            umeshHdlPtr->tets.size() + umeshHdlPtr->pyrs.size(),                               // wedges
            umeshHdlPtr->tets.size() + umeshHdlPtr->pyrs.size() + umeshHdlPtr->wedges.size()}; // hexes

        // define cell types
        std::vector<uint8_t> cellTypes = {
            OSP_TETRAHEDRON,
            OSP_PYRAMID,
            OSP_WEDGE,
            OSP_HEXAHEDRON};

        std::vector<size_t> indices;
        // loop over the elements and create a cummlative index list
        for (auto &t : umeshHdlPtr->tets)
        {
            indices.push_back(t[0]);
            indices.push_back(t[1]);
            indices.push_back(t[2]);
            indices.push_back(t[3]);
        }
        for (auto &p : umeshHdlPtr->pyrs)
        {
            indices.push_back(p[0]);
            indices.push_back(p[1]);
            indices.push_back(p[2]);
            indices.push_back(p[3]);
            indices.push_back(p[4]);
        }
        for (auto &w : umeshHdlPtr->wedges)
        {
            indices.push_back(w[0]);
            indices.push_back(w[1]);
            indices.push_back(w[2]);
            indices.push_back(w[3]);
            indices.push_back(w[4]);
            indices.push_back(w[5]);
        }
        for (auto &h : umeshHdlPtr->hexes)
        {
            indices.push_back(h[0]);
            indices.push_back(h[1]);
            indices.push_back(h[2]);
            indices.push_back(h[3]);
            indices.push_back(h[4]);
            indices.push_back(h[5]);
            indices.push_back(h[6]);
            indices.push_back(h[7]);
        }

        volume.setParam("index", ospray::cpp::CopiedData(indices));
        volume.setParam("cell.index", ospray::cpp::CopiedData(cells));
        volume.setParam("cell.type", ospray::cpp::CopiedData(cellTypes));

        volume.commit();

        ospray::cpp::VolumetricModel model(volume);
        model.setParam("transferFunction", makeTransferFunction({0.f, 1.f}));
        model.commit();

        ospray::cpp::Group group;

        group.setParam("volume", ospray::cpp::CopiedData(model));
        group.commit();
        ospray::cpp::Instance instance(group);
        instance.commit();

        // create world add group
        ospray::cpp::World world;
        world.setParam("instance", ospray::cpp::CopiedData(instance));

        // calculate the bounding box of the umesh volume and set a camera to look at it
        rkcommon::math::vec3f center = {umeshHdlPtr->bounds.center().x, umeshHdlPtr->bounds.center().y, umeshHdlPtr->bounds.center().z};
        // get max extent of the bounding box
        float maxExtent = std::max(umeshHdlPtr->bounds.size().x, std::max(umeshHdlPtr->bounds.size().y, umeshHdlPtr->bounds.size().z));
        // set the camera position to the center of the bounding box and look at it

        arcballCamera.reset(
            new ArcballCamera(world.getBounds<box3f>(), windowSize));
        lastXfm = arcballCamera->transform();

        // init camera
        camera.setParam("position", vec3f(0.0f, 0.0f, 1.0f));
        updateCamera();
        camera.commit();

        ospray::cpp::FrameBuffer framebuffer(1024, 1024, OSP_FB_SRGBA, OSP_FB_COLOR | OSP_FB_ACCUM);
        framebuffer.clear();

        // Render
        framebuffer.renderFrame(renderer, camera, world);

        // Access rendered image
        const uint32_t *fb = (uint32_t *)framebuffer.map(OSP_FB_COLOR);
        rkcommon::utility::writePPM("output.ppm", 1024, 1024, fb);
    }

    void Renderer::Render()
    {
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