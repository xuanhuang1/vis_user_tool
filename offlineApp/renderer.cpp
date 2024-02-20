#include "renderer.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../stb_image_write.h"

namespace visuser
{
    Renderer::Renderer(const AniObjWidget &config)
    {
        // initialize OSPRay
        const char *argv[] = {"vistool_osp_offline"};
        InitializeOSPRay(1, argv);

        //renderer
        renderer = ospray::Renderer("scivis");
        //renderer.setParam("aoSamples", 00);
        //renderer.setParam("aoDistance", 1e20f);
        //renderer.setParam("shadows", true);
        renderer.setParam("volumeSamplingRate", 10.f);
        renderer.setParam("backgroundColor", rkm::vec4f(0.3f, 0.3f, 0.3f, 1.0f));
        renderer.commit();

        //framebuffer
        framebuffer = ospray::FrameBuffer(imgSize.x, imgSize.y,
                                          OSP_FB_SRGBA, OSP_FB_COLOR | OSP_FB_ACCUM);

        //auto umeshPtr = umesh::io::loadBinaryUMesh(config.file_name);

        rkcommon::math::vec3ui dimensions(config.dims.x, config.dims.y, config.dims.z);
        std::shared_ptr<RectMesh> mesh = std::make_shared<RectMesh>(dimensions, config.zMapping, config.file_name);

        //transfer function
        transferFunction = ospray::TransferFunction("piecewiseLinear");
        if(config.tfRange.x == 0.f && config.tfRange.y == 0.f)
        {
            printf("using default tf\n");
            std::vector<rkcommon::math::vec3f> colors =
            {
                rkm::vec3f(1.f, 0.f, 0.f),
                rkm::vec3f(0.f, 1.f, 0.f),
                rkm::vec3f(0.f, 0.f, 1.f)
            };
            std::vector<float> opacities = 
            {
                0.0f, 1.0f
            };
            transferFunction.setParam("color", ospray::CopiedData(colors));
            transferFunction.setParam("opacity", ospray::CopiedData(opacities));
            SetTFRange(rkm::range1f(0.0f,1.0f));
            printf("tf range: %f %f\n", 0.0f,1.0f);
        }
        else
        {
            printf("using custom tf\n");
            //printf("tf range: %f %f\n", umeshPtr->getValueRange().lower, umeshPtr->getValueRange().upper);
            std::vector<float> colors, opacities;
            config.getFrameTF(colors, opacities);
            //print size of opacities and colors
            transferFunction.setParam("opacity", ospray::CopiedData(opacities));
            transferFunction.setParam("value", 
                                rkm::range1f(config.tfRange[0], config.tfRange[1]));
            SetTFColors(colors);
        }
        //volume
        //InitializeVolumeModel(umeshPtr);
        InitializeVolumeModel(mesh);


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
        // ambient.setParam("intensity", 15.f);
        // ambient.setParam("visible", false);
        // ambient.commit();
        // std::vector<ospray::Light> lights{light, ambient};

        world = ospray::World();
        world.setParam("instance", ospray::CopiedData(inst));
        //world.setParam("light", ospray::CopiedData(lights));
        world.commit();

        auto bounds = world.getBounds<rkm::box3f>();
        std::cout << "bounds: " << bounds.lower.x << " " << bounds.lower.y
                  << " " << bounds.lower.z << ", " << bounds.upper.x << " " << bounds.upper.y
                  << " " << bounds.upper.z << std::endl;

        //camera                                
        camera = ospray::Camera("perspective");
        camera.setParam("aspect", float(imgSize.x) / float(imgSize.y));
        if (config.cameras.size() > 0)
        {
            auto &cam = config.cameras[0];
            SetCamera(cam);
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
    }

    void Renderer::Render()
    {
        framebuffer.renderFrame(renderer, camera, world);
    }

    void Renderer::SaveFramePPM(const std::string &filename)
    {
        // Access and save rendered image
        const uint32_t *fb = (uint32_t *)framebuffer.map(OSP_FB_COLOR);
        rkcommon::utility::writePPM(filename + ".ppm", imgSize.x, imgSize.y, fb);
    }

    void Renderer::SaveFramePNG(const std::string &filename)
    {
        // Access and save rendered image
        const uint32_t *fb = (uint32_t *)framebuffer.map(OSP_FB_COLOR);
        stbi_write_png((filename + ".png").c_str(), imgSize.x, imgSize.y, 4, fb ,4 * imgSize.x);

    }
#ifdef OFFLINE_WITH_UMESH
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
        model.setParam("transferFunction", transferFunction);
        model.commit();
    }
#endif

    void Renderer::InitializeVolumeModel(const std::shared_ptr<RectMesh> rectMeshPtr)
    {
        volume = ospray::Volume("unstructured");
        std::cout << "found " << rectMeshPtr->indices.size() << " indices" << std::endl;
        std::cout << "found " << rectMeshPtr->vertices.size() << " vertices" << std::endl;
        std::cout << "found " << rectMeshPtr->scalars.size() << " scalars" << std::endl;

        // set data objects for volume object
        volume.setParam("vertex.position", ospray::CopiedData(rectMeshPtr->vertices));
        volume.setParam("vertex.data", ospray::CopiedData(rectMeshPtr->scalars));
        volume.setParam("background", ospray::CopiedData(rkm::vec3f(1.0f, 0.0f, 1.0f)));

        // define cell offsets in indices array
        std::vector<size_t> cells;

        // define cell types
        std::vector<uint8_t> cellTypes;

        std::vector<uint64> indices;
        for (size_t i = 0; i < rectMeshPtr->indices.size(); i += 8)
        {
            cellTypes.push_back(OSP_HEXAHEDRON);
            cells.push_back(indices.size());
            indices.push_back(rectMeshPtr->indices[i + 0]);
            indices.push_back(rectMeshPtr->indices[i + 1]);
            indices.push_back(rectMeshPtr->indices[i + 2]);
            indices.push_back(rectMeshPtr->indices[i + 3]);
            indices.push_back(rectMeshPtr->indices[i + 4]);
            indices.push_back(rectMeshPtr->indices[i + 5]);
            indices.push_back(rectMeshPtr->indices[i + 6]);
            indices.push_back(rectMeshPtr->indices[i + 7]);
        }
        printf("#cells: %d #cell types %d\n", cells.size(), cellTypes.size());

        volume.setParam("index", ospray::CopiedData(indices));
        volume.setParam("cell.index", ospray::CopiedData(cells));
        volume.setParam("cell.type", ospray::CopiedData(cellTypes));

        volume.commit();

        model = ospray::VolumetricModel(volume);
        model.setParam("transferFunction", transferFunction);
        model.commit();
    }

    void Renderer::SetCamera(const Camera &cam)
    {
        framebuffer.clear();

        camera.setParam("position", rkm::vec3f(cam.pos.x, cam.pos.y, cam.pos.z));
        camera.setParam("direction", rkm::vec3f(cam.dir.x, cam.dir.y, cam.dir.z));
        camera.setParam("up", rkm::vec3f(cam.up.x, cam.up.y, cam.up.z));
        camera.commit();
    }

    void Renderer::SetTFColors(const std::vector<rkcommon::math::vec3f> colors)
    {
        framebuffer.clear();
        transferFunction.setParam("color", ospray::CopiedData(colors));
        transferFunction.commit();
    }

    void Renderer::SetTFColors(const std::vector<float> colors)
    {
        framebuffer.clear();
        //loop over the colors and convert them to vec3f
        std::vector<rkm::vec3f> colorVec;
        for (size_t i = 0; i < colors.size(); i += 3)
        {
            colorVec.push_back(rkm::vec3f(colors[i], colors[i + 1], colors[i + 2]));
        }
        SetTFColors(colorVec);
    }

    void Renderer::SetTFOpacities(const std::vector<float> opacities)
    {
        framebuffer.clear();
        transferFunction.setParam("opacity", ospray::CopiedData(opacities));
        transferFunction.commit();
    }

    void Renderer::SetTFRange(const rkm::range1f &range)
    {
        framebuffer.clear();
        transferFunction.setParam("value", range);
        transferFunction.commit();
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