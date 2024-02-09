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
        opacities.emplace_back(1.f);
        opacities.emplace_back(1.f);
        opacities.emplace_back(0.f);
        opacities.emplace_back(0.f);
        opacities.emplace_back(0.f);
        opacities.emplace_back(1.f);
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
    namespace rkm = rkcommon::math;
    namespace ospray = ospray::cpp;
    Renderer::Renderer(const AniObjWidget &config)
    {
        // initialize OSPRay; OSPRay parses (and removes) its commandline parameters,
        // e.g. "--osp:debug"
        const char *argv[] = {"vistool_osp_offline"};
        InitializeOSPRay(1, argv);

        ospray::Renderer renderer("scivis");
        renderer.setParam("aoSamples", 0);
        renderer.setParam("volumeSamplingRate", 0.008f);
        renderer.setParam("backgroundColor", rkm::vec3f(0.3f, 0.3f, 0.3f));
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
            vertices.push_back(rkcommon::math::vec3f(v.x, v.y, v.z));
        }

        ospray::Volume volume("unstructured");

        // set data objects for volume object
        volume.setParam("vertex.position", ospray::CopiedData(vertices));
        volume.setParam("vertex.data", ospray::CopiedData(umeshHdlPtr->perVertex->values));
        volume.setParam("background", ospray::CopiedData(rkm::vec3f(1.0f, 0.0f, 1.0f)));

        // define cell offsets in indices array
        std::vector<size_t> cells = {};

        // define cell types
        std::vector<uint8_t> cellTypes = {
            //OSP_TETRAHEDRON
        };

        std::vector<uint64> indices;
        // loop over the elements and create a cummlative index list
        for (auto &t : umeshHdlPtr->tets)
        {
            cellTypes.push_back(OSP_TETRAHEDRON);
            cells.push_back(indices.size());
            indices.push_back(t[0]);
            indices.push_back(t[1]);
            indices.push_back(t[2]);
            indices.push_back(t[3]);
        }
        for (auto &p : umeshHdlPtr->pyrs)
        {
            cellTypes.push_back(OSP_PYRAMID);
            cells.push_back(indices.size());
            indices.push_back(p[0]);
            indices.push_back(p[1]);
            indices.push_back(p[2]);
            indices.push_back(p[3]);
            indices.push_back(p[4]);
        }
        for (auto &w : umeshHdlPtr->wedges)
        {
            cellTypes.push_back(OSP_WEDGE);
            cells.push_back(indices.size());
            indices.push_back(w[0]);
            indices.push_back(w[1]);
            indices.push_back(w[2]);
            indices.push_back(w[3]);
            indices.push_back(w[4]);
            indices.push_back(w[5]);
        }
        for (auto &h : umeshHdlPtr->hexes)
        {
            cellTypes.push_back(OSP_HEXAHEDRON);
            cells.push_back(indices.size());
            indices.push_back(h[0]);
            indices.push_back(h[1]);
            indices.push_back(h[2]);
            indices.push_back(h[3]);
            indices.push_back(h[4]);
            indices.push_back(h[5]);
            indices.push_back(h[6]);
            indices.push_back(h[7]);
        }
        printf("#cells: %d #cell types %d\n", cells.size(), cellTypes.size());

        volume.setParam("index", ospray::CopiedData(indices));
        volume.setParam("cell.index", ospray::CopiedData(cells));
        volume.setParam("cell.type", ospray::CopiedData(cellTypes));

        volume.commit();

        ospray::VolumetricModel model(volume);
        model.setParam("transferFunction", makeTransferFunction({umeshHdlPtr->perVertex->valueRange.lower,umeshHdlPtr->perVertex->valueRange.upper }));
        model.commit();

        //get model bounds and print using ospGetBounds

        ospray::Group group;

        group.setParam("volume", ospray::CopiedData(model));
        group.commit();
        ospray::Instance instance(group);
        instance.commit();

        std::vector<ospray::Instance> inst;
        inst.push_back(instance);

        ospray::Light light("distant");
        light.setParam("direction", rkm::vec3f(0.8f, -0.6f, 0.9f));
        light.setParam("color", rkm::vec3f(0.78f, 0.551f, 0.483f));
        light.setParam("intensity", 3.14f);
        light.setParam("angularDiameter", 100.f);
        light.commit();
        ospray::Light ambient("ambient");
        ambient.setParam("intensity", 0.35f);
        ambient.setParam("visible", true);
        ambient.commit();
        std::vector<ospray::Light> lights{light, ambient};

        ospray::World world;
        world.setParam("instance", ospray::CopiedData(inst));
        world.setParam("light", ospray::CopiedData(lights));
        world.commit();

        auto bounds = world.getBounds<rkm::box3f>();
        std::cout << "bounds: " << bounds.lower.x << " " << bounds.lower.y << " " << bounds.lower.z << ", " << bounds.upper.x << " " << bounds.upper.y << " " << bounds.upper.z << std::endl;
        //print the umesh bounds
        std::cout << "umesh bounds: " << umeshHdlPtr->bounds.lower << " " << umeshHdlPtr->bounds.upper << std::endl;

        //calculate center of the bounds and set the camera diretion to the center and position to the center - 5
        rkm::vec3f center = (bounds.lower + bounds.upper) / 2.f;
        rkm::vec3f cameraPos = center - rkm::vec3f(1.001f, 1.3f, 0.f) * 90.f;
        rkm::vec3f cameraDir = center - cameraPos;

        // create and setup camera
        ospray::Camera camera("perspective");
        camera.setParam("aspect", 1.0f);
        camera.setParam("position", cameraPos);
        camera.setParam("direction", cameraDir);
        camera.setParam("up", rkm::vec3f({0.f, -1.f, 0.f}));
        camera.commit();

         for (int i = 0; i < 10; ++i) {
            // Calculate new camera position for rotation
            float angle = i * (2.0f * M_PI / 10.0f); // 10 frames for a full circle
            rkm::vec3f cameraPos = center + rkm::vec3f(std::cos(angle) * 90.f, std::sin(angle) * 90.f, 0.f);
            rkm::vec3f cameraDir = center - cameraPos;
            
            // Setup camera
            ospray::Camera camera("perspective");
            camera.setParam("aspect", 1.0f);
            camera.setParam("position", cameraPos);
            camera.setParam("direction", cameraDir);
            camera.setParam("up", rkm::vec3f({0.001f, -1.f, -0.01f}));
            camera.commit();
            
            // Render frame
            ospray::FrameBuffer framebuffer(1024, 1024, OSP_FB_SRGBA, OSP_FB_COLOR | OSP_FB_ACCUM);
            framebuffer.clear();
            framebuffer.renderFrame(renderer, camera, world);
            
            // Access and save rendered image
            const uint32_t *fb = (uint32_t *)framebuffer.map(OSP_FB_COLOR);
            rkcommon::utility::writePPM("output_frame_" + std::to_string(i) + ".ppm", 1024, 1024, fb);
        }
        //update the camera position to move up
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