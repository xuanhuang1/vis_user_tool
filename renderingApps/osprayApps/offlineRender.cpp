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
#include "GLFWOSPWindow.h"

using json = nlohmann_loader::json;
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
	std::string overwrite_inputf = "";

	// which kf file to render
	// -2 = render all frames, -1 = render all kfs, >0 = render selected kf
	int header_sel = -1; 
	for (int i = 1; i < argc; ++i) {
	    if (args[i] == "-h") {
		std::cout << "./mini_vistool <config.json> [options] \n"
			  << "  [options]: \n"
			  << "  -f img_output_name"
			  << "  -header-sel # (select a keyframe under the header file, i.e. config.json must be a header)"
			  << "\n";
		return 0;
	    } else {
	    	if (i == 1){
		    std::ifstream cfg_file(args[i].c_str());
		    if (!cfg_file) {
			std::cerr << "[error]: Failed to open config file " << args[i] << "\n";
			throw std::runtime_error("Failed to open input config file");
		    }
		    cfg_file >> config;
		}else{
		    if (args[i] == "-f"){
			overwrite_inputf = args[++i];
		    }else if (args[i] == "-header-sel"){
			header_sel = std::stoi(args[++i]);
		    }else if (args[i] == "-header")
			header_sel = -1;
		}
	    }
	}
	
	AniObjWidget widget;
	AniObjHandler h(argv[1]);
	if (header_sel < 0)
	    widget.init_from_json(h.widgets[0].config);
	else widget.init_from_json(h.widgets[header_sel].config);

	if (overwrite_inputf != ""){
	    widget.file_name = overwrite_inputf;
	}

	vec3i volumeDimensions(widget.dims[0], widget.dims[1], widget.dims[2]);
	float min=std::numeric_limits<float>::infinity(), max=0;
	std::vector<float> voxels(volumeDimensions.long_product());

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


	// construct one time objects
        // init volume mesh
	ospray::cpp::Group group;
	if (widget.type_name == "structured"){
	    glfwOspWindow.initVolume(volumeDimensions, glfwOspWindow.world_size_x);
	}else if (widget.type_name == "unstructured"){
	    glfwOspWindow.initVolumeOceanZMap(volumeDimensions, glfwOspWindow.world_size_x);
	}else if (widget.type_name == "structuredSpherical"){
	    std::string path_to_bgmap = "/home/xuanhuang/projects/vis_interface/vis_user_tool/renderingApps/mesh/land.png";
	    glfwOspWindow.initVolumeSphere(volumeDimensions, path_to_bgmap);
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

#ifdef _WIN32
    if (waitForKey) {
	printf("\n\tpress any key to exit");
	_getch();
    }
#endif

    

    return 0;
}

