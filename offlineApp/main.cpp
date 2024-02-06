#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../stb_image_write.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#ifdef _WIN32
#define NOMINMAX
#include <conio.h>
#include <malloc.h>
#include <windows.h>
#else
#include <alloca.h>
#endif

#include <vector>

#include "renderer.h"

#include "../loader.h"

using namespace rkcommon::math;
using json = nlohmann::json;
using namespace visuser;

// #define DEBUG_LOG

int main(int argc, const char **argv)
{
    std::vector<std::string> args(argv, argv + argc);

    //
    // load json file to configure input
    //

    json config;
    std::string prefix;
    for (int i = 1; i < argc; ++i)
    {
        if (args[i] == "-h")
        {
            std::cout << "./vistool_osp_offline <config.json> [options TODO] \n";
            return 0;
        }
        else
        {
            std::ifstream cfg_file(args[i].c_str());
            if (!cfg_file)
            {
                std::cerr << "[error]: Failed to open config file " << args[i] << "\n";
                throw std::runtime_error("Failed to open input config file");
            }
            cfg_file >> config;
        }
    }

    // load json
    std::cout << "\n\nStart json loading ... \n";
    AniObjWidget widget(config);
    widget.load_info();
    widget.load_cameras();
    widget.load_tfs();
    std::cout << "\nEnd json loading ... \n\n";

    // log out json info
    // see AniObjWidget member list in ../load.h
    // includes
    // 1. basic input file info
    // 2. keyframes for for rendering an animation
#ifdef DEBUG_LOG
    {
        widget.print_info();
        for (auto &c : widget.cameras)
            c.print();
    }
#endif

    //
    // end of json load
    //

    // construct the unstructured mesh
    // grid with uniform x and y
    // but varying z from widget.zMapping
    // i.e. the physical depth for this data

    // init renderer

    // create ospray renderer
    Renderer renderer(widget);

    std::cout << "\nStart rendering... \n\n";

    // start rendering into images
    for (uint32_t i = widget.frameRange[0]; i <= widget.frameRange[1]; i++)
    {
        // get this frame
        widget.advanceFrame();

        // load camera and tf
        Camera cam;
        std::vector<float> c, o;
        glm::vec2 valueRange = widget.tfRange;
        widget.getFrameCam(cam);
        widget.getFrameTF(c, o);
#ifdef DEBUG_LOG
        cam.print();
        std::cout << valueRange[0] << " " << valueRange[1] << "\n";
#endif

        // render
        renderer.Render();

        // output
        std::string image_name = "output-f" + std::to_string(i) + ".jpg";
        std::cout << "rendering frame " << i << " into " << image_name << "\n";
    }

    return 0;
}
