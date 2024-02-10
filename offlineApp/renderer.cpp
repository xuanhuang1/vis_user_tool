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
    Renderer::Renderer(const AniObjWidget &config)
    {
        // initialize OSPRay
        const char *argv[] = {"vistool_osp_offline"};
        InitializeOSPRay(1, argv);

        renderer = ospray::Renderer("scivis");
        renderer.setParam("aoSamples", 5);
        renderer.setParam("volumeSamplingRate", 0.008f);
        renderer.setParam("backgroundColor", rkm::vec3f(0.3f, 0.3f, 0.3f));
        renderer.commit();

        // get model bounds and print using ospGetBounds
        auto umeshPtr = umesh::io::loadBinaryUMesh(config.file_name);
        InitializeVolumeModel(umeshPtr);

        ospray::Group group;

        group.setParam("volume", ospray::CopiedData(model));
        group.commit();
        ospray::Instance instance(group);
        instance.commit();

        std::vector<ospray::Instance> inst;
        inst.push_back(instance);

        // ospray::Light light("distant");
        // light.setParam("direction", rkm::vec3f(0.8f, -0.6f, 0.9f));
        // light.setParam("color", rkm::vec3f(0.78f, 0.551f, 0.483f));
        // light.setParam("intensity", 3.14f);
        // light.setParam("angularDiameter", 100.f);
        // light.commit();
        // ospray::Light ambient("ambient");
        // ambient.setParam("intensity", 0.35f);
        // ambient.setParam("visible", true);
        // ambient.commit();
        // std::vector<ospray::Light> lights{light, ambient};

        world = ospray::World();
        world.setParam("instance", ospray::CopiedData(inst));
        // world.setParam("light", ospray::CopiedData(lights));
        world.commit();

        auto bounds = world.getBounds<rkm::box3f>();
        std::cout << "bounds: " << bounds.lower.x << " " << bounds.lower.y
                  << " " << bounds.lower.z << ", " << bounds.upper.x << " " << bounds.upper.y
                  << " " << bounds.upper.z << std::endl;

        camera = ospray::Camera("perspective");
        camera.setParam("aspect", float(imgSize.x) / float(imgSize.y));
        if (config.cameras.size() > 0)
        {
            auto &cam = config.cameras[0];
            camera.setParam("position", rkm::vec3f(cam.pos.x, cam.pos.y, cam.pos.z));
            camera.setParam("direction", rkm::vec3f(cam.dir.x, cam.dir.y, cam.dir.z));
            camera.setParam("up", rkm::vec3f(cam.up.x, cam.up.y, cam.up.z));
        }
        else
        {
            // calculate center of the bounds and set the camera diretion to the center and position to the center - 5
            rkm::vec3f center = (bounds.lower + bounds.upper) / 2.f;
            float radius = length(bounds.upper - bounds.lower) * 1.5f;
            rkm::vec3f cameraPos = center - rkm::vec3f(1.f, 1.f, 1.f) * radius;
            rkm::vec3f cameraDir = center - cameraPos;
            camera.setParam("position", cameraPos);
            camera.setParam("direction", cameraDir);
            camera.setParam("up", rkm::vec3f(0.f, 1.f, 0.f));
        }

        camera.commit();

        // Render frame
        framebuffer = ospray::FrameBuffer(imgSize.x, imgSize.y,
                                          OSP_FB_SRGBA, OSP_FB_COLOR | OSP_FB_ACCUM);
        framebuffer.clear();
    }

    void Renderer::Render()
    {
        framebuffer.renderFrame(renderer, camera, world);
    }

    void Renderer::SaveFrame(const std::string &filename)
    {
        // Access and save rendered image
        const uint32_t *fb = (uint32_t *)framebuffer.map(OSP_FB_COLOR);
        rkcommon::utility::writePPM(filename + ".ppm", imgSize.x, imgSize.y, fb);
    }

    void Renderer::InitializeVolumeModel(const std::shared_ptr<umesh::UMesh> umeshPtr)
    {
        volume = ospray::Volume("unstructured");
        std::cout << "found " << umeshPtr->tets.size() << " tetrahedra" << std::endl;
        std::cout << "found " << umeshPtr->pyrs.size() << " pyramids" << std::endl;
        std::cout << "found " << umeshPtr->wedges.size() << " wedges" << std::endl;
        std::cout << "found " << umeshPtr->hexes.size() << " hexahedra" << std::endl;
        std::cout << "found " << umeshPtr->vertices.size() << " vertices" << std::endl;

        std::vector<rkcommon::math::vec3f> vertices;
        // iterate over vertices and copy them into a vector
        for (auto &v : umeshPtr->vertices)
        {
            vertices.push_back(rkcommon::math::vec3f(v.x, v.y, v.z));
        }

        // set data objects for volume object
        volume.setParam("vertex.position", ospray::CopiedData(vertices));
        volume.setParam("vertex.data", ospray::CopiedData(umeshPtr->perVertex->values));
        volume.setParam("background", ospray::CopiedData(rkm::vec3f(1.0f, 0.0f, 1.0f)));

        // define cell offsets in indices array
        std::vector<size_t> cells;

        // define cell types
        std::vector<uint8_t> cellTypes;

        std::vector<uint64> indices;
        // loop over the elements and create a cummlative index list
        for (auto &t : umeshPtr->tets)
        {
            cellTypes.push_back(OSP_TETRAHEDRON);
            cells.push_back(indices.size());
            indices.push_back(t[0]);
            indices.push_back(t[1]);
            indices.push_back(t[2]);
            indices.push_back(t[3]);
        }
        for (auto &p : umeshPtr->pyrs)
        {
            cellTypes.push_back(OSP_PYRAMID);
            cells.push_back(indices.size());
            indices.push_back(p[0]);
            indices.push_back(p[1]);
            indices.push_back(p[2]);
            indices.push_back(p[3]);
            indices.push_back(p[4]);
        }
        for (auto &w : umeshPtr->wedges)
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
        for (auto &h : umeshPtr->hexes)
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

        model = ospray::VolumetricModel(volume);
        model.setParam("transferFunction",
                       makeTransferFunction({umeshPtr->perVertex->valueRange.lower,
                                             umeshPtr->perVertex->valueRange.upper}));
        model.commit();
    }

    Renderer::~Renderer()
    {
        ospDeviceRelease(device);
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

        device = ospGetCurrentDevice();
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
    }

} // namespace visuser