#include <pybind11/stl.h>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <iostream>

namespace py = pybind11;

void echo(int i) {
    std::cout << i <<std::endl;
}

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
#include "../../stb_image_write.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <chrono>
#include <limits>
#ifdef _WIN32
#define NOMINMAX
#include <conio.h>
#include <malloc.h>
#include <windows.h>
#else
#include <alloca.h>
#endif

#include "rkcommon/utility/SaveImage.h"
#include "../GLFWOSPWindow.h"

using json = nlohmann::json;
using namespace visuser;


enum DATATYPE{TRI_MESH, VOL, TOTAL_DATA_TYPES};
std::string dataTypeString[] = {"triangle_mesh", "volume"};


GLFWOSPWindow *GLFWOSPWindow::activeWindow = nullptr;

ospray::cpp::TransferFunction makeTransferFunction(const vec2f &valueRange, tfnw::TransferFunctionWidget& widget)
{
    ospray::cpp::TransferFunction transferFunction("piecewiseLinear");
    std::string tfColorMap{"jet"};
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
	widget.alpha_control_pts[0].x = 0.f;
	widget.alpha_control_pts[0].y = 0.f;
        widget.alpha_control_pts[1].x = 1.f;
	widget.alpha_control_pts[1].y = 1.f;
    } else if (tfOpacityMap == "linearInv") {
	opacities.emplace_back(1.f);
	opacities.emplace_back(0.f);
        widget.alpha_control_pts[0].x = 1.f;
	widget.alpha_control_pts[0].y = 1.f;
        widget.alpha_control_pts[1].x = 0.f;
	widget.alpha_control_pts[1].y = 0.f;
    } else if (tfOpacityMap == "opaque") {
	opacities.emplace_back(1.f);
	opacities.emplace_back(1.f);
        widget.alpha_control_pts[0].x = 0.f;
	widget.alpha_control_pts[0].y = 1.f;
        widget.alpha_control_pts[1].x = 1.f;
	widget.alpha_control_pts[1].y = 1.f;
    }

    transferFunction.setParam("color", ospray::cpp::CopiedData(colors));
    transferFunction.setParam("opacity", ospray::cpp::CopiedData(opacities));
    transferFunction.setParam("valueRange", valueRange);
    transferFunction.commit();
    //    widget.update_colormap();

    return transferFunction;
}

void init (void* fb, GLFWOSPWindow &glfwOspWindow){
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glEnable( GL_BLEND );
    glGenTextures(1, &glfwOspWindow.texture);
    glBindTexture(GL_TEXTURE_2D, glfwOspWindow.texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load and generate the texture

    glTexImage2D(GL_TEXTURE_2D,
		 0,
		 GL_RGBA32F,
		 glfwOspWindow.imgSize.x,
		 glfwOspWindow.imgSize.y,
		 0,
		 GL_RGBA,
		 GL_FLOAT,
		 fb);

}


static std::vector<std::string>
init_app(const std::vector<std::string>& args)
{
    int argc = args.size();
    const char **argv = new const char*[argc];
    
    for (int i = 0; i < argc; i++)
        argv[i] = args[i].c_str();
    
    OSPError res = ospInit(&argc, argv);
    
    if (res != OSP_NO_ERROR)
    {
        delete [] argv;
	std::cout <<"ospInit() failed";
    }
    
    std::vector<std::string> newargs;
    
    for (int i = 0; i < argc; i++)
        newargs.push_back(std::string(argv[i]));
    
    delete [] argv;
    
    return newargs;
}

int run_app(py::array_t<float> &input_array, int x, int y, int z, int count)
{
 
#ifdef _WIN32
    bool waitForKey = false;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
	// detect standalone console: cursor at (0,0)?
	waitForKey = csbi.dwCursorPosition.X == 0 && csbi.dwCursorPosition.Y == 0;
    }
#endif
	
    // use scoped lifetimes of wrappers to release everything before ospShutdown()
    {
    
	// initialize GLFW
	if (!glfwInit()) {
	    throw std::runtime_error("Failed to initialize GLFW!");
	}

	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

	GLFWOSPWindow glfwOspWindow;

	// create GLFW window
	glfwOspWindow.glfwWindow = glfwCreateWindow(glfwOspWindow.windowSize.x, glfwOspWindow.windowSize.y, "Viewer", nullptr, nullptr);

	if (!glfwOspWindow.glfwWindow) {
	    glfwTerminate();
	    throw std::runtime_error("Failed to create GLFW window!");
	}

	py::buffer_info buf_info = input_array.request();
	glfwOspWindow.all_data_ptr = static_cast<float *>(buf_info.ptr);
	glfwOspWindow.count = count;
	std::cout << "shape:" <<count <<" of "<< x <<" "<<y<<" "<<z <<std::endl;

	
	glfwOspWindow.volumeDimensions[0] = x; 
	glfwOspWindow.volumeDimensions[1] = y; 
	glfwOspWindow.volumeDimensions[2] = z; 
	float min=std::numeric_limits<float>::infinity(), max=0;
	std::vector<float> voxels(glfwOspWindow.volumeDimensions.long_product());
	
	// construct ospray variables
	ospray::cpp::Group group;
	{
	     for (long long i =0 ; i < glfwOspWindow.volumeDimensions.long_product(); i++){
	    	voxels[i] = glfwOspWindow.all_data_ptr[i];
	    	min = std::min(min, voxels[i]);
	    	max = std::max(max, voxels[i]);        
            }
	    //for (size_t z_i=0; z_i<z; z_i++){
	    //	for (size_t y_i=0; y_i<y; y_i++){
	    //	    long long offset = z_i * x * y + y_i * x;
	    //	    for (size_t x_i=0; x_i<x; x_i++){
	    //		size_t flipX = x - 1 - x_i;
	    //		voxels[offset + flipX] = glfwOspWindow.all_data_ptr[offset + x_i];
	    //		min = std::min(min, voxels[offset + flipX]);
	    //		max = std::max(max, voxels[offset + flipX]);
	    //	    }
	    //	}
	     //	for (int k=0; k<10; k++)
	     //	    if (z_i == float(z)/10*k)
	     //		std::cout <<z_i*x*y<<" "<< k<<"0% \n";    
	     //}

	    
	    std::cout <<"End load max: "<< max <<" min: "<<min<< "\n";
	    glfwOspWindow.voxel_data = &voxels;
	    glfwOspWindow.tf_range = vec2f(min, max);
        }
    		
	glfwOspWindow.tfn = makeTransferFunction(vec2f(min, max), glfwOspWindow. tfn_widget);
    	    
	// volume
	//ospray::cpp::Volume volume("structuredSpherical");
	if (1){
	    ospray::cpp::Volume volume("structuredRegular");
	    volume.setParam("gridOrigin", vec3f(0.f,0.f,0.f));
	    volume.setParam("gridSpacing", vec3f(10.f / reduce_max(glfwOspWindow.volumeDimensions)));
	    //volume.setParam("gridSpacing", vec3f(10.f, 180.f / volumeDimensions.y, 360.f/volumeDimensions.z));
	    volume.setParam("data", ospray::cpp::SharedData(glfwOspWindow.voxel_data->data(), glfwOspWindow.volumeDimensions));
	    volume.setParam("dimensions", glfwOspWindow.volumeDimensions);
	    volume.commit();
	    glfwOspWindow.volume = volume;
	
	    // put the mesh into a model
	    ospray::cpp::VolumetricModel model(volume);
	     
	    model.setParam("transferFunction", glfwOspWindow.tfn);
	    model.commit();
	    glfwOspWindow.model = model;
	}else
	    glfwOspWindow.initVolumeOceanZMap(glfwOspWindow.volumeDimensions, 10.f);
	
	group.setParam("volume", ospray::cpp::CopiedData(glfwOspWindow.model));
	group.commit();

	glfwOspWindow.initClippingPlanes();

	// put the group into an instance (give the group a world transform)
	//ospray::cpp::Instance instance(group);
	//instance.commit();
    	glfwOspWindow.instance = ospray::cpp::Instance(group);
    	glfwOspWindow.instance.commit();

	// put the instance in the world
	ospray::cpp::World world;
	//world.setParam("instance", ospray::cpp::CopiedData(instance));
	glfwOspWindow.world.setParam("instance", ospray::cpp::CopiedData(glfwOspWindow.instance));

	// create and setup light for Ambient Occlusion
	ospray::cpp::Light light("ambient");
	light.commit();
	
	//world.setParam("light", ospray::cpp::CopiedData(light));
	//world.commit();
	glfwOspWindow.world.setParam("light", ospray::cpp::CopiedData(light));
    	glfwOspWindow.world.commit();
    	
    	// set up arcball camera for ospray
    	glfwOspWindow.arcballCamera.reset(new ArcballCamera(glfwOspWindow.world.getBounds<box3f>(), glfwOspWindow.windowSize));
    	glfwOspWindow.arcballCamera->updateWindowSize(glfwOspWindow.windowSize);
    	std::cout << "boundbox: "<< glfwOspWindow.world.getBounds<box3f>() << "\n";
    
	
	// create renderer, choose Scientific Visualization renderer
	ospray::cpp::Renderer *renderer = &glfwOspWindow.renderer;;

	// complete setup of renderer
	renderer->setParam("aoSamples", 1);
	renderer->setParam("backgroundColor", 0.0f); // white, transparent
	renderer->commit();

	
	ospray::cpp::Camera* camera = &glfwOspWindow.camera;
	    
	camera->setParam("aspect", glfwOspWindow.imgSize.x / (float)glfwOspWindow.imgSize.y);
	camera->setParam("position", glfwOspWindow.arcballCamera->eyePos());
	camera->setParam("direction", glfwOspWindow.arcballCamera->lookDir());
	camera->setParam("up", glfwOspWindow.arcballCamera->upDir());
	camera->commit(); // commit each object to indicate modifications are done
	
	std::cout << "All osp objects committed\n";
	glfwOspWindow.preRenderInit();	
	
	auto t1 = std::chrono::high_resolution_clock::now();
	auto t2 = std::chrono::high_resolution_clock::now();

	std::cout << "Begin render loop\n";
	do{
	    glfwPollEvents();
    
	    t1 = std::chrono::high_resolution_clock::now();
      
	    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	    glEnable(GL_TEXTURE_2D);
  
      
	    glEnable(GL_FRAMEBUFFER_SRGB); // Turn on sRGB conversion for OSPRay frame
	    glfwOspWindow.display();
	    glDisable(GL_FRAMEBUFFER_SRGB); // Disable SRGB conversion for UI
	    glDisable(GL_TEXTURE_2D);
      
	    // Start the Dear ImGui frame
	    ImGui_ImplGlfwGL3_NewFrame();
	    glfwOspWindow.buildUI();
	    ImGui::Render();
	    ImGui_ImplGlfwGL3_Render();
      
	    // Swap buffers
	    glfwMakeContextCurrent(glfwOspWindow.glfwWindow);
	    glfwSwapBuffers(glfwOspWindow.glfwWindow);
      

	    t2 = std::chrono::high_resolution_clock::now();
	    auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
	    glfwSetWindowTitle(glfwOspWindow.glfwWindow, (std::string("Render FPS:")+std::to_string(int(1.f / time_span.count()))).c_str());

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(glfwOspWindow.glfwWindow, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
	       glfwWindowShouldClose(glfwOspWindow.glfwWindow) == 0 );
    
	ImGui_ImplGlfwGL3_Shutdown();
	glfwTerminate();
	
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



PYBIND11_MODULE(vistool_py, m) {
    // Optional docstring
    m.doc() = "the renderer's py library";
        
    m.def("init_app", &init_app, "init render app");
    m.def("run_app", &run_app, "run render app");
}
