#include <algorithm>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <fstream>
#include "loader.h"


//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"
//#include "stb_image_write.h"

using json = nlohmann::json;
using namespace visuser;

const std::string USAGE =
    "./mini_vistool <config.json> [options]\n"
    "Options:\n"
    //"  -prefix <name>       Provide a prefix to prepend to the image file names.\n"
    //"  -fif <N>             Restrict the number of frames being rendered in parallel.\n"
    //"  -no-output           Don't save output images (useful for benchmarking).\n"
    //"  -detailed-stats      Record and print statistics about CPU use, thread pinning, etc.\n"
    "  -h                   Print this help.";

int main(int argc, char **argv)
{
    if (argc < 2) std::cout << USAGE<< std::endl;

    std::vector<std::string> args(argv, argv + argc);
    
    json config;
    std::string prefix;
    for (int i = 1; i < argc; ++i) {
        if (args[i] == "-h") {
            std::cout << USAGE << "\n";
            return 0;
        } else {
            std::ifstream cfg_file(args[i].c_str());
            if (!cfg_file) {
                std::cerr << "[error]: Failed to open config file " << args[i] << "\n";
                throw std::runtime_error("Failed to open input config file");
            }
            cfg_file >> config;
        }
    }
    
    std::vector<Camera> cams = load_cameras(config["camera"].get<std::vector<json>>(), 10);

    for(auto &c : cams)
	c.print();
    return 0;
}
