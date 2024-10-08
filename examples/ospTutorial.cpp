// Copyright 2009 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

/* This is a small example tutorial how to use OSPRay in an application.
 *
 * On Linux build it in the build_directory with
 *   g++ ../apps/ospTutorial/ospTutorial.cpp -I ../ospray/include \
 *       -I ../../rkcommon -L . -lospray -Wl,-rpath,. -o ospTutorial
 * On Windows build it in the build_directory\$Configuration with
 *   cl ..\..\apps\ospTutorial\ospTutorial.cpp /EHsc -I ..\..\ospray\include ^
 *      -I ..\.. -I ..\..\..\rkcommon ospray.lib
 * Above commands assume that rkcommon is present in a directory right "next
 * to" the OSPRay directory. If this is not the case, then adjust the include
 * path (alter "-I <path/to/rkcommon>" appropriately).
 */

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

#include "ospray/ospray_cpp.h"
#include "ospray/ospray_cpp/ext/rkcommon.h"
#include "rkcommon/utility/SaveImage.h"

#include "../loader.h"

using namespace rkcommon::math;
using json = nlohmann_loader::json;
using namespace visuser;

enum DATATYPE{TRI_MESH, VOL, TOTAL_DATA_TYPES};
std::string dataTypeString[] = {"triangle_mesh", "volume"};


ospray::cpp::TransferFunction makeTransferFunction(const vec2f &valueRange)
{
  ospray::cpp::TransferFunction transferFunction("piecewiseLinear");
  std::string tfColorMap{"rgb"};
  std::string tfOpacityMap{"linear"};
  
  std::vector<vec3f> colors;
  std::vector<float> opacities;

  if (tfColorMap == "jet") {
    colors.emplace_back(0, 0, 0.562493);
    colors.emplace_back(0, 0, 1);
    colors.emplace_back(0, 1, 1);
    colors.emplace_back(0.500008, 1, 0.500008);
    colors.emplace_back(1, 1, 0);
    colors.emplace_back(1, 0, 0);
    colors.emplace_back(0.500008, 0, 0);
  } else if (tfColorMap == "rgb") {
    colors.emplace_back(0, 0, 1);
    colors.emplace_back(0, 1, 0);
    colors.emplace_back(1, 0, 0);
  } else {
    colors.emplace_back(0.f, 0.f, 0.f);
    colors.emplace_back(1.f, 1.f, 1.f);
  }

  if (tfOpacityMap == "linear") {
    opacities.emplace_back(0.f);
    opacities.emplace_back(1.f);
  } else if (tfOpacityMap == "linearInv") {
    opacities.emplace_back(1.f);
    opacities.emplace_back(0.f);
  } else if (tfOpacityMap == "opaque") {
    opacities.emplace_back(1.f);
  }

  transferFunction.setParam("color", ospray::cpp::CopiedData(colors));
  transferFunction.setParam("opacity", ospray::cpp::CopiedData(opacities));
  transferFunction.setParam("valueRange", valueRange);
  transferFunction.commit();

  return transferFunction;
}



int main(int argc, const char **argv)
{
    std::vector<std::string> args(argv, argv + argc);
    
    json config;
    std::string prefix;
    for (int i = 1; i < argc; ++i) {
        if (args[i] == "-h") {
            std::cout << "./mini_vistool <config.json> [options]\n";
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

    
    // load json
    //std::vector<Camera> cams = load_cameras(config["camera"].get<std::vector<json>>(), 10);
    //std::string dataType = config["data"]["type"];
    AniObjWidget widget(config);
    widget.load_info();
    widget.load_cameras();
    std::vector<Camera> cams = widget.cameras;
    std::string dataType = dataTypeString[0];

    // log out json info
    {
	bool foundDataType = false;
	for( uint32_t i=0; i<TOTAL_DATA_TYPES; i++){
	    if (dataType == dataTypeString[i]){
		std::cout << "data type: " << dataType <<"\n";
		foundDataType = true;
	    }
	}
	if (!foundDataType) std::cerr << "Unsupported data type " << dataType <<"\n";
      
	for(auto &c : cams)
	    c.print();
    }


    // construct ospray variables
  
    // image size
    vec2i imgSize;
    imgSize.x = 640; // width
    imgSize.y = 480; // height

    // camera
    vec3f cam_pos{0.f, 0.f, 0.f};
    vec3f cam_up{0.f, 1.f, 0.f};
    vec3f cam_view{0.1f, 0.f, 1.f};

    // triangle mesh data
    std::vector<vec3f> vertex = {vec3f(-1.0f, -1.0f, 3.0f),
	vec3f(-1.0f, 1.0f, 3.0f),
	vec3f(1.0f, -1.0f, 3.0f),
	vec3f(1.0f, 1.0f, 3.0f)};

    std::vector<vec4f> color = {vec4f(0.9f, 0.5f, 0.5f, 1.0f),
	vec4f(0.8f, 0.8f, 0.8f, 1.0f),
	vec4f(0.8f, 0.8f, 0.8f, 1.0f),
	vec4f(0.5f, 0.9f, 0.5f, 1.0f)};

    std::vector<vec3ui> index = {vec3ui(0, 1, 2), vec3ui(1, 2, 3)};

#ifdef _WIN32
    bool waitForKey = false;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
	// detect standalone console: cursor at (0,0)?
	waitForKey = csbi.dwCursorPosition.X == 0 && csbi.dwCursorPosition.Y == 0;
    }
#endif

    // initialize OSPRay; OSPRay parses (and removes) its commandline parameters,
    // e.g. "--osp:debug"
    OSPError init_error = ospInit(&argc, argv);
    if (init_error != OSP_NO_ERROR)
	return init_error;

    // use scoped lifetimes of wrappers to release everything before ospShutdown()
    {
	// create and setup camera
	ospray::cpp::Camera camera("perspective");
	camera.setParam("aspect", imgSize.x / (float)imgSize.y);
	camera.setParam("position", cam_pos);
	camera.setParam("direction", cam_view);
	camera.setParam("up", cam_up);
	camera.commit(); // commit each object to indicate modifications are done

	ospray::cpp::Group group;
	if (dataType == dataTypeString[TRI_MESH]){
	    // create and setup model and mesh
	    ospray::cpp::Geometry mesh("mesh");
	    mesh.setParam("vertex.position", ospray::cpp::CopiedData(vertex));
	    mesh.setParam("vertex.color", ospray::cpp::CopiedData(color));
	    mesh.setParam("index", ospray::cpp::CopiedData(index));
	    mesh.commit();

	    // put the mesh into a model
	    ospray::cpp::GeometricModel model(mesh);
	    model.commit();

	    // put the model into a group (collection of models)
	    group.setParam("geometry", ospray::cpp::CopiedData(model));
	}else if (dataType == dataTypeString[VOL]){
	    vec3i volumeDimensions(100, 100, 100);
	    std::vector<float> voxels(volumeDimensions.long_product());
	    for (uint32_t i =0 ; i < voxels.size(); i++)
		voxels[i] = float(i) / volumeDimensions.long_product();
	    
	    // volume
	    ospray::cpp::Volume volume("structuredRegular");
	    volume.setParam("gridOrigin", vec3f(-0.5f,-0.5f,1.5f));
	    volume.setParam("gridSpacing", vec3f(1.f / reduce_max(volumeDimensions)));
	    volume.setParam("data", ospray::cpp::CopiedData(voxels.data(), volumeDimensions));
	    volume.setParam("dimensions", volumeDimensions);
	    volume.commit();
	    // put the mesh into a model
	    ospray::cpp::VolumetricModel model(volume);
	    model.setParam("transferFunction", makeTransferFunction(vec2f(0.f, 1.f)));
	    model.commit();
	    
	    group.setParam("volume", ospray::cpp::CopiedData(model));
	}
	group.commit();

	// put the group into an instance (give the group a world transform)
	ospray::cpp::Instance instance(group);
	instance.commit();

	// put the instance in the world
	ospray::cpp::World world;
	world.setParam("instance", ospray::cpp::CopiedData(instance));

	// create and setup light for Ambient Occlusion
	ospray::cpp::Light light("ambient");
	light.commit();

	world.setParam("light", ospray::cpp::CopiedData(light));
	world.commit();
	
	// create renderer, choose Scientific Visualization renderer
	ospray::cpp::Renderer renderer("scivis");

	// complete setup of renderer
	renderer.setParam("aoSamples", 1);
	renderer.setParam("backgroundColor", 1.0f); // white, transparent
	renderer.commit();

	// create and setup framebuffer
	ospray::cpp::FrameBuffer framebuffer(
					     imgSize.x, imgSize.y, OSP_FB_SRGBA, OSP_FB_COLOR | OSP_FB_ACCUM);
	framebuffer.clear();
	stbi_flip_vertically_on_write(1);

	// render frames
	for (size_t i=0; i<cams.size(); i++)
	    {
		for (int k=0; k<3; k++){
		    cam_pos[k] = cams[i].pos[k];
		    cam_up[k] = cams[i].up[k];
		    cam_view[k] = cams[i].dir[k];
		}
		camera.setParam("position", cam_pos);
		camera.setParam("direction", cam_view);
		camera.setParam("up", cam_up);
		camera.commit();
	    
		framebuffer.renderFrame(renderer, camera, world);

		// access framebuffer and write its content as PPM file
		uint32_t *fb = (uint32_t *)framebuffer.map(OSP_FB_COLOR);
		std::string imgName = "outframe_osp_"+std::to_string(i)+".jpg";
		//rkcommon::utility::writePPM(imgName, imgSize.x, imgSize.y, fb);
		stbi_write_jpg(imgName.c_str(), imgSize.x, imgSize.y, 4, fb, 90);
		framebuffer.unmap(fb);
		framebuffer.clear();
		std::cout << "rendering frame to " << imgName << std::endl;

		// render 10 more frames, which are accumulated to result in a better
		// converged image
		//for (int frames = 0; frames < 10; frames++)
		//    framebuffer.renderFrame(renderer, camera, world);

		//fb = (uint32_t *)framebuffer.map(OSP_FB_COLOR);
		//rkcommon::utility::writePPM(
		//			    "accumulatedFrameCpp.ppm", imgSize.x, imgSize.y, fb);
		//framebuffer.unmap(fb);
		//std::cout << "rendering 10 accumulated frames to accumulatedFrameCpp.ppm"
		//	  << std::endl;

		ospray::cpp::PickResult res =
		    framebuffer.pick(renderer, camera, world, 0.5f, 0.5f);

		if (res.hasHit) {
		    std::cout << "picked geometry [instance: " << res.instance.handle()
			      << ", model: " << res.model.handle()
			      << ", primitive: " << res.primID << "]" << std::endl;
		}
	    }
    }
    ospShutdown();

#ifdef _WIN32
    if (waitForKey) {
	printf("\n\tpress any key to exit");
	_getch();
    }
#endif

    return 0;
}
