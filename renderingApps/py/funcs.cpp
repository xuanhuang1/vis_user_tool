#include <pybind11/stl.h>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include "../../ext/pybind11_json/pybind11_json.hpp"
#include <iostream>

namespace py = pybind11;

void echo(int i) {
    std::cout << i <<std::endl;
}

template<class T>
std::vector<T>makeVectorFromPyArray( py::array_t<T>py_array )
{
    return std::vector<T>(py_array.data(), py_array.data() + py_array.size());
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
    std::vector<float> colors_f;
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

    for (auto c : colors) {
	colors_f.push_back(c.x);
	colors_f.push_back(c.y);
	colors_f.push_back(c.z);
    }
    
    widget.set_osp_colormapf(colors_f, opacities);
    std::cout << "init tf col sz="<< widget.osp_colors.size()<<" "
	      <<widget.alpha_control_pts.size() <<" \n";
    
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

int run_app(py::array_t<float> &input_array, py::list &input_names, int x, int y, int z, int count, int mode)
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

	// process py inputs
	py::buffer_info buf_info = input_array.request();
        auto fnames = input_names.cast<std::vector<std::string>>();
	glfwOspWindow.all_data_ptr = static_cast<float *>(buf_info.ptr);
	glfwOspWindow.count = count;
	glfwOspWindow.setFileNames(fnames);
	glfwOspWindow.volumeDimensions[0] = x; 
	glfwOspWindow.volumeDimensions[1] = y; 
	glfwOspWindow.volumeDimensions[2] = z;
	std::cout << "shape:" <<count <<" of "<< x <<" "<<y<<" "<<z <<std::endl;
	std::cout << "file names: ";
	for (auto f : glfwOspWindow.file_names)
	    std::cout << f <<" ";
	std::cout <<"\n";

	// init vol containers
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
	    
	    std::cout <<"End load max: "<< max <<" min: "<<min<< "\n";
	    glfwOspWindow.voxel_data = &voxels;
	    glfwOspWindow.tf_range = vec2f(min, max);
        }
    		
	glfwOspWindow.tfn = makeTransferFunction(vec2f(min, max), glfwOspWindow. tfn_widget);
    	    
	// volume
	//ospray::cpp::Volume volume("structuredSpherical");
	if (mode == 0){
	    glfwOspWindow.initVolume(glfwOspWindow.volumeDimensions, glfwOspWindow.world_size_x);
	}else if (mode == 1)
	    glfwOspWindow.initVolumeOceanZMap(glfwOspWindow.volumeDimensions, glfwOspWindow.world_size_x);
	else if (mode == 2){
	    glfwOspWindow.initVolumeSphere(glfwOspWindow.volumeDimensions);
	    group.setParam("geometry", ospray::cpp::CopiedData(glfwOspWindow.gmodel));	
	}
	
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
	light.setParam("visible", false);
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
	//renderer->setParam("lightSamples", -1);
	//renderer->setParam("maxPathLength", 20);
	//renderer->setParam("roulettePathLength", 1);
	//renderer->setParam("minContribution", 0.001f);
	//renderer->setParam("maxContribution", 2000.f);
	
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

	
	glfwOspWindow.printSessionSummary();
    
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



void loadWidget(GLFWOSPWindow &glfwOspWindow, AniObjWidget &widget, std::vector<float> &voxels, float &max, float &min)
{
    vec3i volumeDimensions(widget.dims[0], widget.dims[1], widget.dims[2]);
    min=std::numeric_limits<float>::infinity();
    max=0;
	
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
    glfwOspWindow.tfn = loadTransferFunction(widget, glfwOspWindow.tfn_widget);
    glfwOspWindow.volumeDimensions = volumeDimensions;
	
    // load camera
    auto c = widget.cameras[0];
    vec3f pos(c.pos[0], c.pos[1], c.pos[2]);
    vec3f dir(c.dir[0], c.dir[1], c.dir[2]);
    vec3f up(c.up[0], c.up[1], c.up[2]);
    ospray::cpp::Camera* camera = &glfwOspWindow.camera;
    camera->setParam("aspect", glfwOspWindow.imgSize.x / (float)glfwOspWindow.imgSize.y);
    camera->setParam("position", pos);
    camera->setParam("direction", dir);
    camera->setParam("up", up);
    camera->commit(); // commit each object to indicate modifications are done	
}

void renderLoop(GLFWOSPWindow &glfwOspWindow){
    std::cout << "Begin render loop\n";
    do{
	glfwPollEvents();
    
	auto t1 = std::chrono::high_resolution_clock::now();
      
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
      

	auto t2 = std::chrono::high_resolution_clock::now();
	auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
	glfwSetWindowTitle(glfwOspWindow.glfwWindow, (std::string("Render FPS:")+std::to_string(int(1.f / time_span.count()))).c_str());

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(glfwOspWindow.glfwWindow, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
	   glfwWindowShouldClose(glfwOspWindow.glfwWindow) == 0 );
    
}


int run_offline(std::string jsonStr, std::string overwrite_inputf, int header_sel) 
{
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

	// load json
	std::cout << "\n\nStart json loading ... \n";
      
	AniObjHandler h(jsonStr.c_str());

	if (overwrite_inputf != ""){
	    h.widgets[0].file_name = overwrite_inputf;
	}

    	std::cout << "\nEnd json loading ... \n\n";
	
	vec3i volumeDimensions(h.widgets[0].dims[0], h.widgets[0].dims[1], h.widgets[0].dims[2]);
	float min=std::numeric_limits<float>::infinity(), max=0;
	std::vector<float> voxels(volumeDimensions.long_product());

	std::fstream file;
	file.open(h.widgets[0].file_name, std::fstream::in | std::fstream::binary);
	std::cout <<"dim "<<h.widgets[0].dims[0]<<" "<<h.widgets[0].dims[1]<<" "<<h.widgets[0].dims[2]<<"\nLoad "<< voxels.size()<< " :";
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
	std::cout <<"init range: "<< max <<" "<<min<<"\n";
	glfwOspWindow.tfn = loadTransferFunction(h.widgets[0], glfwOspWindow.tfn_widget);
	glfwOspWindow.volumeDimensions = volumeDimensions;
	
	// load camera
	auto c = h.widgets[0].cameras[0];
	vec3f pos(c.pos[0], c.pos[1], c.pos[2]);
	vec3f dir(c.dir[0], c.dir[1], c.dir[2]);
	vec3f up(c.up[0], c.up[1], c.up[2]);
	ospray::cpp::Camera* camera = &glfwOspWindow.camera;
	camera->setParam("aspect", glfwOspWindow.imgSize.x / (float)glfwOspWindow.imgSize.y);
	camera->setParam("position", pos);
	camera->setParam("direction", dir);
	camera->setParam("up", up);
	camera->commit(); // commit each object to indicate modifications are done	


	// construct one time objects
        // init volume mesh
	ospray::cpp::Group group;
	if (h.widgets[0].type_name == "structured"){
	    glfwOspWindow.initVolume(volumeDimensions, glfwOspWindow.world_size_x);
	}else if (h.widgets[0].type_name == "unstructured"){
	    glfwOspWindow.initVolumeOceanZMap(volumeDimensions, glfwOspWindow.world_size_x);
	}else if (h.widgets[0].type_name == "structuredSpherical"){
	    glfwOspWindow.initVolumeSphere(volumeDimensions);
	    group.setParam("geometry", ospray::cpp::CopiedData(glfwOspWindow.gmodel));
	}
	group.setParam("volume", ospray::cpp::CopiedData(glfwOspWindow.model));
	group.commit();

	glfwOspWindow.initClippingPlanes();
	std::cout << "volume loaded\n";

	// ospray objects init
   	glfwOspWindow.instance = ospray::cpp::Instance(group);
    	glfwOspWindow.instance.commit();
	ospray::cpp::World world;
	glfwOspWindow.world.setParam("instance", ospray::cpp::CopiedData(glfwOspWindow.instance));
	ospray::cpp::Light light("ambient");
	light.commit();
	glfwOspWindow.world.setParam("light", ospray::cpp::CopiedData(light));
    	glfwOspWindow.world.commit();
	ospray::cpp::Renderer *renderer = &glfwOspWindow.renderer;;
	renderer->setParam("aoSamples", 1);
	renderer->setParam("backgroundColor", 0.0f); // black, transparent
	renderer->commit();
	    
	std::cout << "All osp objects committed\n";

	glfwOspWindow.preRenderInit();

	if (header_sel >= 0){ // render selected keyframe
	    glfwPollEvents();
	    auto t1 = std::chrono::high_resolution_clock::now();

	    // render
	    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	    glEnable(GL_TEXTURE_2D);      
	    glEnable(GL_FRAMEBUFFER_SRGB); // Turn on sRGB conversion for OSPRay frame
	    glfwOspWindow.display();
	    glfwOspWindow.renderNewFrame();
	    glDisable(GL_FRAMEBUFFER_SRGB); // Disable SRGB conversion for UI
	    glDisable(GL_TEXTURE_2D);
	    // Swap buffers
	    glfwMakeContextCurrent(glfwOspWindow.glfwWindow);
	    glfwSwapBuffers(glfwOspWindow.glfwWindow);

	    // save file
	    std::string base_filename = h.widgets[header_sel].file_name.substr(h.widgets[header_sel].file_name.find_last_of("/\\") + 1);
	    std::string outname = base_filename.substr(0, base_filename.find_last_of("."));
	    outname = "img_"+outname+"_kf"+std::to_string(header_sel)+".png";
	    glfwOspWindow.saveFrame(outname);

	    auto t2 = std::chrono::high_resolution_clock::now();
	    auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
	    std::cout <<"write: "<< outname <<" "<< time_span.count() <<"sec \n\n";
	}else{ // render all key frames

	    // reload widget for each key frame
	    for (int kf_idx=0; kf_idx<h.widgets.size(); kf_idx++){
		min=std::numeric_limits<float>::infinity();
		max=0;
	
		std::fstream file;
		file.open(h.widgets[kf_idx].file_name, std::fstream::in | std::fstream::binary);
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
		glfwOspWindow.tfn = loadTransferFunction(h.widgets[kf_idx], glfwOspWindow.tfn_widget);
		glfwOspWindow.volumeDimensions = volumeDimensions;
	
		// load camera
		auto c = h.widgets[kf_idx].cameras[0];
		vec3f pos(c.pos[0], c.pos[1], c.pos[2]);
		vec3f dir(c.dir[0], c.dir[1], c.dir[2]);
		vec3f up(c.up[0], c.up[1], c.up[2]);
		ospray::cpp::Camera* camera = &glfwOspWindow.camera;
		camera->setParam("aspect", glfwOspWindow.imgSize.x / (float)glfwOspWindow.imgSize.y);
		camera->setParam("position", pos);
		camera->setParam("direction", dir);
		camera->setParam("up", up);
		camera->commit(); // commit each object to indicate modifications are done	

		if (header_sel == -1){// render key frames
		    glfwPollEvents();
		    auto t1 = std::chrono::high_resolution_clock::now();

		    // render
		    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		    glEnable(GL_TEXTURE_2D);      
		    glEnable(GL_FRAMEBUFFER_SRGB); // Turn on sRGB conversion for OSPRay frame
		    glfwOspWindow.display();
		    glfwOspWindow.renderNewFrame();
		    glDisable(GL_FRAMEBUFFER_SRGB); // Disable SRGB conversion for UI
		    glDisable(GL_TEXTURE_2D);
		    // Swap buffers
		    glfwMakeContextCurrent(glfwOspWindow.glfwWindow);
		    glfwSwapBuffers(glfwOspWindow.glfwWindow);

		    // save file
		    std::string base_filename = h.widgets[kf_idx].file_name.substr(h.widgets[kf_idx].file_name.find_last_of("/\\") + 1);
		    std::string outname = base_filename.substr(0, base_filename.find_last_of("."));
		    outname = "img_"+outname+"_kf"+std::to_string(kf_idx)+".png";
		    glfwOspWindow.saveFrame(outname);

		    auto t2 = std::chrono::high_resolution_clock::now();
		    auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
		    std::cout <<"write: "<< outname <<" "<< time_span.count() <<"sec \n\n";

		}else if (header_sel == -2){//renderAllFrames
		    
		    std::cout <<"\nrender frame "
			      << h.widgets[kf_idx].frameRange[0] <<" - "
			      << h.widgets[kf_idx].frameRange[1] <<" sec \n";
    
		    for (int f = h.widgets[kf_idx].frameRange[0]; f <= h.widgets[kf_idx].frameRange[1]; f++){
			auto t1 = std::chrono::high_resolution_clock::now();

			// render
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_TEXTURE_2D);      
			glEnable(GL_FRAMEBUFFER_SRGB); // Turn on sRGB conversion for OSPRay frame
			glfwOspWindow.display();
			glfwOspWindow.renderNewFrame();	    
			glDisable(GL_FRAMEBUFFER_SRGB); // Disable SRGB conversion for UI
			glDisable(GL_TEXTURE_2D);
			// Swap buffers
			glfwMakeContextCurrent(glfwOspWindow.glfwWindow);
			glfwSwapBuffers(glfwOspWindow.glfwWindow);

			std::string base_filename = h.widgets[kf_idx].file_name.substr(h.widgets[kf_idx].file_name.find_last_of("/\\") + 1);
			std::string outname = base_filename.substr(0, base_filename.find_last_of("."));
			outname = "img_"+outname+"_f"+std::to_string(f)+".png";
			glfwOspWindow.saveFrame(outname);

			auto t2 = std::chrono::high_resolution_clock::now();
			auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
			std::cout <<"write: f"<< f  <<" "<< time_span.count() <<"sec \n";

			if (f < h.widgets[kf_idx].frameRange[1]){
			    // advance frame 
			    h.widgets[kf_idx].advanceFrame();
			    auto c = Camera();
			    h.widgets[kf_idx].getFrameCam(c);
			    vec3f pos(c.pos[0], c.pos[1], c.pos[2]);
			    vec3f dir(c.dir[0], c.dir[1], c.dir[2]);
			    vec3f up(c.up[0], c.up[1], c.up[2]);
			    glfwOspWindow.camera.setParam("position", pos);
			    glfwOspWindow.camera.setParam("direction", dir);
			    glfwOspWindow.camera.setParam("up", up);
			    glfwOspWindow.camera.commit();
			}
		    }
    
		    std::cout <<"\n";

		}
	    }
	}
	
	ImGui_ImplGlfwGL3_Shutdown();
	glfwTerminate();
	
    }
    ospShutdown();

    return 0;
}



nlohmann::json generateScript(){
    nlohmann::json j = {{"value", 1}};

    std::cout << "This function returns an nlohmann::json instance: "  << j << std::endl;

    return j;
}

nlohmann::json fixedCamHelper(std::string meta_file_name,
			      std::vector<std::string> &filenames,
			      int kf_interval,
			      std::vector<int> dims,
			      std::string meshType,
			      int world_bbx_len,
			      std::vector<float> &cam,
			      std::vector<float> &tf_range)

{
    nlohmann::ordered_json j;
    std::vector<uint32_t> data_i_list_kf;
    std::string base_file_name = meta_file_name+"_kf";
    std::filesystem::path p = std::filesystem::current_path();
    std::string p_str = p.generic_string() + "/";

    j["isheader"] = true;
    for (int i=0; i<filenames.size();i++){
	data_i_list_kf.push_back(i);
	j["data_list"][i]["name"] = p_str+filenames[i];
	j["data_list"][i]["dims"] = {dims[0], dims[1], dims[2]};
    }
       
    // export all key frames to json file
    // write a header of file names 
    for (size_t i=0; i<data_i_list_kf.size();i++){
	std::string file_name = base_file_name + std::to_string(i) + ".json";
	j["file_list"][i]["keyframe"] = file_name;
	j["file_list"][i]["data_i"] = i;

	// write json for each keyframe interval
	nlohmann::ordered_json tmp_j;
	tmp_j["isheader"] = false;
	tmp_j["data"]["type"] = meshType;
	tmp_j["data"]["name"] = p_str+filenames[i];
	tmp_j["data"]["dims"] = {dims[0], dims[1], dims[2]};
	tmp_j["data"]["world_bbox"] = {world_bbx_len, world_bbx_len, world_bbx_len};
	tmp_j["data"]["frameRange"] = {i*kf_interval, (i+1)*kf_interval};

	// cameras
	for (size_t j=0; j<2; j++)
	    {
		nlohmann::ordered_json tmp_cam;
		tmp_cam["frame"] = (i+j)*kf_interval;
		for (size_t c=0; c<3; c++){
		    tmp_cam["pos"].push_back(cam[c]);
		    tmp_cam["dir"].push_back(cam[3+c]);
		    tmp_cam["up"].push_back(cam[6+c]);
		}
		tmp_j["camera"].push_back(tmp_cam);
	    }

	// tf
	tmp_j["transferFunc"][0]["frame"] = i*kf_interval;
	tmp_j["transferFunc"][0]["range"] = {tf_range[0], tf_range[1]};

	// fixed tf for now
        tmp_j["transferFunc"][0]["colors"] = {0, 0, 0.562493,  0, 0, 1,  0, 1, 1,  0.500008, 1, 0.500008,  1, 1, 0,  1, 0, 0,  0.500008, 0, 0};
        tmp_j["transferFunc"][0]["opacities"].push_back(0);
	tmp_j["transferFunc"][0]["opacities"].push_back(20);
	    
	std::ofstream o(file_name);
	o << std::setw(4)<< tmp_j <<std::endl;
	o.close();
    }
    std::ofstream o_meta(meta_file_name+".json");
    o_meta << std::setw(4) << j <<std::endl;
    o_meta.close();

    return j;
}


nlohmann::json generateScriptFixedCam(std::string meta_file_name,
				      py::list &input_names,
				      int kf_interval,
				      py::array_t<int> dims_in,
				      std::string meshType,
				      int world_bbx_len,
				      py::array_t<float> cam_in,
				      py::array_t<float> tf_range_in
				      )
{
    std::vector<std::string> filenames = input_names.cast<std::vector<std::string>>();
    std::vector<int> dims = makeVectorFromPyArray(dims_in);
    std::vector<float> cam = makeVectorFromPyArray(cam_in);
    std::vector<float> tf_range = makeVectorFromPyArray(tf_range_in);

    if (1){
	std::cout << "output file name "   << meta_file_name << "\n";
	std::cout << "filenames: "; for (auto s : filenames) std::cout << s <<" "; std::cout << "\n";
	std::cout << "dims: ";      for (auto d : dims)      std::cout << d <<" "; std::cout << "\n";
	std::cout << "cam: ";       for (auto c : cam)       std::cout << c <<" "; std::cout << "\n";
	std::cout << "tf_range: ";  for (auto v : tf_range)  std::cout << v <<" "; std::cout << "\n";
	std::cout << "key frame interval = " << kf_interval << "\n";
	std::cout << "mesh type = "          << meshType << "\n";
	std::cout << "world bbox size = "    << world_bbx_len << "\n";
    }

    return fixedCamHelper(meta_file_name, filenames, kf_interval, dims, meshType, world_bbx_len, cam, tf_range);
}


nlohmann::json readScript(std::string path){
    nlohmann::json j;
    std::ifstream f(path);
    f >> j;

    return j;
}


PYBIND11_MODULE(vistool_py, m) {
    // Optional docstring
    m.doc() = "the renderer's py library";
        
    m.def("init_app", &init_app, "init render app");
    m.def("run_app", &run_app, "run render app");
    m.def("run_offline_app", &run_offline, "run render app");

    m.def("generateScript", &generateScript, "generate preset script");
    m.def("generateScriptFixedCam", &generateScriptFixedCam, "generate preset script");
    m.def("readScript", &readScript, "read key frame script from file");
}
