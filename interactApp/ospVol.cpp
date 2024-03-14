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
#include "GLFWOSPWindow.h"

using json = nlohmann::json;
using namespace visuser;

enum DATATYPE{TRI_MESH, VOL, TOTAL_DATA_TYPES};
std::string dataTypeString[] = {"triangle_mesh", "volume"};

GLFWOSPWindow *GLFWOSPWindow::activeWindow = nullptr;

float clamp(float x)
{
    if (x < 0.f) {
	return 0.f;
    }
    if (x > 1.f) {
	return 1.f;
    }
    return x;
}



ospray::cpp::TransferFunction makeTransferFunction(const vec2f &valueRange, tfnw::TransferFunctionWidget& widget)
{
    ospray::cpp::TransferFunction transferFunction("piecewiseLinear");
    std::string tfColorMap{"jet"};
    std::string tfOpacityMap{"opaque"};
  
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

ospray::cpp::TransferFunction loadTransferFunction(AniObjWidget &widget, tfnw::TransferFunctionWidget& tfWidget)
{
    ospray::cpp::TransferFunction transferFunction("piecewiseLinear");
    
    std::vector<float> colors;
    std::vector<vec3f> colors3f;
    std::vector<float> opacities;
    const vec2f valueRange(widget.tfRange[0], widget.tfRange[1]);
    widget.getFrameTF(colors, opacities);

    for (uint32_t i=0; i<colors.size()/3; i++){
    	colors3f.emplace_back(colors[i*3], colors[i*3+1], colors[i*3+2]);
    }
    
    tfWidget.set_osp_colormapf(colors, opacities);
    std::cout << "load tf col sz="<< tfWidget.osp_colors.size()<<" "
	      <<tfWidget.alpha_control_pts.size() <<" \n";
    
    transferFunction.setParam("color", ospray::cpp::CopiedData(colors3f));
    transferFunction.setParam("opacity", ospray::cpp::CopiedData(opacities));
    transferFunction.setParam("valueRange", valueRange);
    transferFunction.commit();

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

int main(int argc, const char **argv)
{


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


	std::vector<std::string> args(argv, argv + argc);
    
	json config;
	std::string prefix;
	for (int i = 1; i < argc; ++i) {
	    if (args[i] == "-h") {
		std::cout << "./mini_vistool <config.json> [options]\n";
		return 0;
	    } else {
	    	if (i == 1){
		    std::ifstream cfg_file(args[i].c_str());
		    if (!cfg_file) {
			std::cerr << "[error]: Failed to open config file " << args[i] << "\n";
			throw std::runtime_error("Failed to open input config file");
		    }
		    cfg_file >> config;
		}
	    }
	}
	
    
	// load json
	//std::vector<Camera> cams = load_cameras(config["camera"].get<std::vector<json>>(), 10);
	//std::string dataType = config["data"]["type"];
	std::cout << "\n\nStart json loading ... \n";
    	AniObjWidget widget(config);
    	widget.load_info();
    	widget.load_cameras();
    	widget.load_tfs();
    	std::cout << "\nEnd json loading ... \n\n";
    
	
	vec3i volumeDimensions(widget.dims[0], widget.dims[1], widget.dims[2]);
	float min=std::numeric_limits<float>::infinity(), max=0;
	std::vector<float> voxels(volumeDimensions.long_product());
	
	// construct ospray variables
	ospray::cpp::Group group;
	{
	    std::fstream file;
      	    file.open(widget.file_name, std::fstream::in | std::fstream::binary);
    	    std::cout <<"dim "<<widget.dims[0]<<" "<<widget.dims[1]<<" "<<widget.dims[2]<<"\nLoad "<< voxels.size()<< " :";
	    for (size_t z=0; z<volumeDimensions[2]; z++){
		long long offset = z * volumeDimensions[0] * volumeDimensions[1];
		for (size_t y=0; y<volumeDimensions[1]; y++){
		    for (size_t x =0 ; x < volumeDimensions[0]; x++){
			float buff;
			file.read((char*)(&buff), sizeof(buff));
			voxels[offset + y*volumeDimensions[0] + x] = float(buff);
			if (float(buff) > max) max = float(buff);
			if (float(buff) < min) min = float(buff);
		    }
		}
		for (int k=0; k<10; k++)
		    if (z == (volumeDimensions[2]/10)*k)
			std::cout <<z*volumeDimensions[0] * volumeDimensions[1]<<" "<< k<<"0% \n";    
	    }
	    std::cout <<"End load \n";
	    file.close();
	    glfwOspWindow.voxel_data = &voxels;
	    glfwOspWindow.tf_range[0] = min;
	    glfwOspWindow.tf_range[1] = max;
	    std::cout <<"range: "<< max <<" "<<min<<"\n";
	}
    		
	//glfwOspWindow.tfn = makeTransferFunction(vec2f(-1.f, 1.f), glfwOspWindow. tfn_widget);
    	glfwOspWindow.tfn = loadTransferFunction(widget, glfwOspWindow.tfn_widget);

	// construct volume 
	//glfwOspWindow.initVolume(volumeDimensions, widget);
	glfwOspWindow.initVolumeSphere(volumeDimensions, widget);
	//glfwOspWindow.initVolumeOceanZMap(volumeDimensions, glfwOspWindow.world_size);
	group.setParam("volume", ospray::cpp::CopiedData(glfwOspWindow.model));
	group.setParam("geometry", ospray::cpp::CopiedData(glfwOspWindow.gmodel));
	group.commit();

	glfwOspWindow.initClippingPlanes();

	// put the group into an instance (give the group a world transform)
   	glfwOspWindow.instance = ospray::cpp::Instance(group);
    	glfwOspWindow.instance.commit();

	// put the instance in the world
	ospray::cpp::World world;
	glfwOspWindow.world.setParam("instance", ospray::cpp::CopiedData(glfwOspWindow.instance));

	// create and setup light for Ambient Occlusion
	ospray::cpp::Light light("ambient");
	light.commit();
	
	glfwOspWindow.world.setParam("light", ospray::cpp::CopiedData(light));
    	glfwOspWindow.world.commit();
    	
    	// set up arcball camera for ospray
    	glfwOspWindow.arcballCamera.reset(new ArcballCamera(glfwOspWindow.world.getBounds<box3f>(), glfwOspWindow.windowSize));
    	glfwOspWindow.arcballCamera->updateWindowSize(glfwOspWindow.windowSize);
    	std::cout << glfwOspWindow.arcballCamera->eyePos() <<"\n";
    	std::cout << glfwOspWindow.arcballCamera->lookDir() <<"\n";
    	std::cout << glfwOspWindow.arcballCamera->upDir() <<"\n";
    	
    	auto c = widget.cameras[0];
    	vec3f pos(c.pos[0], c.pos[1], c.pos[2]);
    	vec3f dir(c.dir[0], c.dir[1], c.dir[2]);
    	vec3f up(c.up[0], c.up[1], c.up[2]);
	
	// create renderer, choose Scientific Visualization renderer
	ospray::cpp::Renderer *renderer = &glfwOspWindow.renderer;;

	// complete setup of renderer
	renderer->setParam("aoSamples", 1);
	renderer->setParam("backgroundColor", 0.0f); // white, transparent
	renderer->commit();

	
	ospray::cpp::Camera* camera = &glfwOspWindow.camera;
	    
	camera->setParam("aspect", glfwOspWindow.imgSize.x / (float)glfwOspWindow.imgSize.y);
	//camera->setParam("position", glfwOspWindow.arcballCamera->eyePos());
	//camera->setParam("direction", glfwOspWindow.arcballCamera->lookDir());
	//camera->setParam("up", glfwOspWindow.arcballCamera->upDir());
	camera->setParam("position", pos);
	camera->setParam("direction", dir);
	camera->setParam("up", up);
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

