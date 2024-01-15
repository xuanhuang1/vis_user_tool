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

#include <vector>

#include "ospray/ospray_cpp.h"
#include "ospray/ospray_cpp/ext/rkcommon.h"
#include "rkcommon/utility/SaveImage.h"

#include "../loader.h"
#include "ArcballCamera.h"
#include "TransferFunctionWidget.h"

#define GLFW_INCLUDE_NONE
#include <GL/glew.h>
#include <GLFW/glfw3.h>
// imgui
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"


using namespace rkcommon::math;
using json = nlohmann::json;
using namespace visuser;


// image size
vec2i imgSize{800, 600};
vec2i windowSize{800,600};
unsigned int texture;
unsigned int guiTextures[128];
unsigned int guiTextureSize = 0;

GLFWwindow *glfwWindow = nullptr;

enum DATATYPE{TRI_MESH, VOL, TOTAL_DATA_TYPES};
std::string dataTypeString[] = {"triangle_mesh", "volume"};


class GLFWOSPWindow{
public:
  ospray::cpp::Camera camera{"perspective"};
  ospray::cpp::Renderer renderer{"scivis"};
  ospray::cpp::World world;
  ospray::cpp::Instance instance;
  ospray::cpp::VolumetricModel model;
  std::vector<float> * voxel_data; // pointer to voxels data
  
  static GLFWOSPWindow *activeWindow;
  ospray::cpp::FrameBuffer framebuffer;
  std::unique_ptr<ArcballCamera> arcballCamera;
  vec2f previousMouse{vec2f(-1)};
  
  ospray::cpp::TransferFunction tfn{"piecewiseLinear"};
  tfnw::TransferFunctionWidget tfn_widget;
  
  GLFWOSPWindow(){
    activeWindow = this;
    
    /// prepare framebuffer
    auto buffers = OSP_FB_COLOR | OSP_FB_DEPTH | OSP_FB_ACCUM | OSP_FB_ALBEDO
      | OSP_FB_NORMAL;
    framebuffer = ospray::cpp::FrameBuffer(imgSize.x, imgSize.y, OSP_FB_RGBA32F, buffers);
    
  }
  
  void display();
  void motion(double, double);
  void mouse(int, int, int, int);
  void reshape(int, int);
    
  void setFunc(){

  glfwSetCursorPosCallback(
      glfwWindow, [](GLFWwindow *, double x, double y) {
		    activeWindow->motion(x, y);
		  });
    glfwSetFramebufferSizeCallback(
      glfwWindow, [](GLFWwindow *, int newWidth, int newHeight) {
        activeWindow->reshape(newWidth, newHeight);
      });

    
  }

  void renderNewFrame(){
    framebuffer.clear();
    // render one frame
    framebuffer.renderFrame(renderer, camera, world);
  }

  void buildUI();
};

GLFWOSPWindow *GLFWOSPWindow::activeWindow = nullptr;


void GLFWOSPWindow::motion(double x, double y)
{
  const vec2f mouse(x, y);
  if (previousMouse != vec2f(-1)) {
    const bool leftDown =
      glfwGetMouseButton(glfwWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    const bool rightDown =
      glfwGetMouseButton(glfwWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    const bool middleDown =
      glfwGetMouseButton(glfwWindow, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
    const vec2f prev = previousMouse;

    bool cameraChanged = leftDown || rightDown || middleDown;

    // don't modify camera with mouse on ui window
    if (ImGui::GetIO().WantCaptureMouse){ 
    	return;
    }
    
    if (leftDown) {
      const vec2f mouseFrom(std::clamp(prev.x * 2.f / windowSize.x - 1.f, -1.f, 1.f),
			    std::clamp(prev.y * 2.f / windowSize.y - 1.f, -1.f, 1.f));
      const vec2f mouseTo(std::clamp(mouse.x * 2.f / windowSize.x - 1.f, -1.f, 1.f),
			  std::clamp(mouse.y * 2.f / windowSize.y - 1.f, -1.f, 1.f));
      arcballCamera->rotate(mouseFrom, mouseTo);
    } else if (rightDown) {
      arcballCamera->zoom(mouse.y - prev.y);
    } else if (middleDown) {
      arcballCamera->pan(vec2f(mouse.x - prev.x, prev.y - mouse.y));
    }

    if (cameraChanged) {
      //updateCamera();
      //addObjectToCommit(camera.handle());
      camera.setParam("aspect", windowSize.x / float(windowSize.y));
      camera.setParam("position", arcballCamera->eyePos());
      camera.setParam("direction", arcballCamera->lookDir());
      camera.setParam("up", arcballCamera->upDir());
      camera.commit();
    }
  }

  previousMouse = mouse;

}


void GLFWOSPWindow::reshape(int w, int h)
{
  
  windowSize.x = w;
  windowSize.y = h;
  
  // create new frame buffer
  auto buffers = OSP_FB_COLOR | OSP_FB_DEPTH | OSP_FB_ACCUM | OSP_FB_ALBEDO
      | OSP_FB_NORMAL;
  framebuffer =
    ospray::cpp::FrameBuffer(imgSize.x, imgSize.y, OSP_FB_RGBA32F, buffers);
  framebuffer.commit();
  
  glViewport(0, 0, windowSize.x, windowSize.y);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, windowSize.x, 0.0, windowSize.y, -1.0, 1.0);

  // update camera
  //arcballCamera->updateWindowSize(windowSize);

  camera.setParam("aspect", windowSize.x / float(windowSize.y));
  camera.commit();

}

void GLFWOSPWindow::display()
{ 
  
   glBindTexture(GL_TEXTURE_2D, texture);
   // render textured quad with OSPRay frame buffer contents
   
   renderNewFrame();
   auto fb = framebuffer.map(OSP_FB_COLOR);

   glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RGBA32F,
		imgSize.x,
		imgSize.y,
		0,
		GL_RGBA,
		GL_FLOAT,
		fb);
   framebuffer.unmap(fb);
   
   
   glBegin(GL_QUADS);

   glTexCoord2f(0.f, 0.f);
   glVertex2f(0.f, 0.f);

   glTexCoord2f(0.f, 1.f);
   glVertex2f(0.f, windowSize.y);

   glTexCoord2f(1.f, 1.f);
   glVertex2f(windowSize.x, windowSize.y);

   glTexCoord2f(1.f, 0.f);
   glVertex2f(windowSize.x, 0.f);

   glEnd();
}


void GLFWOSPWindow::buildUI(){
	static float f = 0;
	ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize;
	ImGui::Begin("Menu Window", nullptr, flags);

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        if (ImGui::SliderFloat("float", &f, 0.0f, 1.0f)){
  	}
  	
  	if (tfn_widget.changed()) {
	    std::vector<float> tmpOpacities;
	    auto alphaOpacities = tfn_widget.get_alpha_control_pts();
	    for (uint32_t i=0;i<alphaOpacities.size();i++){
	      tmpOpacities.push_back((alphaOpacities[i].y*50.f));
	    }
	    tfn_widget.setUnchanged();
    
	    tfn.setParam("opacity", ospray::cpp::CopiedData(tmpOpacities));
	    tfn.commit();
	    model.commit();
	  }

  
	  tfn_widget.draw_ui();
	  
	  
	  
        ImGui::End();
}

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

void init (void* fb){
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glEnable( GL_BLEND );
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load and generate the texture

    glTexImage2D(GL_TEXTURE_2D,
		 0,
		 GL_RGBA32F,
		 imgSize.x,
		 imgSize.y,
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
	// create GLFW window
	glfwWindow = glfwCreateWindow(windowSize.x, windowSize.y, "Viewer", nullptr, nullptr);

	if (!glfwWindow) {
	    glfwTerminate();
	    throw std::runtime_error("Failed to create GLFW window!");
	}


    
	GLFWOSPWindow glfwOspWindow;


	std::vector<std::string> args(argv, argv + argc);
    
	json config;
	std::string prefix;
	for (int i = 1; i < argc; ++i) {
	    if (args[i] == "-h") {
		std::cout << "./mini_vistool <config.json> x y z <file> [options]\n";
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
	std::vector<Camera> cams = load_cameras(config["camera"].get<std::vector<json>>(), 10);
	std::string dataType = config["data"]["type"];

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
	
	vec3i volumeDimensions(std::stoi(argv[2]), std::stoi(argv[3]), std::stoi(argv[4]));
	float min=std::numeric_limits<float>::infinity(), max=0;
	std::vector<float> voxels(volumeDimensions.long_product());
	
	// construct ospray variables
	ospray::cpp::Group group;
	{
	    std::fstream file;
      	    file.open(argv[5], std::fstream::in | std::fstream::binary);
    	    std::cout <<"dim "<<argv[2]<<" "<<argv[3]<<" "<<argv[4]<<"\n";
	    for (uint32_t i =0 ; i < volumeDimensions.long_product(); i++){
	    	float buff;
		file.read((char*)(&buff), sizeof(buff));
		voxels[i] = float(buff);
		if (float(buff) > max) max = float(buff);
		if (float(buff) < min) min = float(buff);
	    }
	    file.close();
	    glfwOspWindow.voxel_data = &voxels;
	    std::cout << max <<" "<<min<<"\n";
	    //glfwOspWindow.tfn = makeTransferFunction(vec2f(-0.1f, 0.1f))
      	    //glfwOspWindow.tfns.push_back(makeTransferFunctionForColor(vec2f(min, max), glfwOspWindow.colors[j]));
	 }
	 std::vector<float> tfn_colors;
    	    std::vector<float> tfn_opacities;
    	    std::vector<vec3f> tfn_colors_3f;
    	    glfwOspWindow.tfn_widget.get_colormapf(tfn_colors, tfn_opacities);
    	    for (uint32_t i=0; i<tfn_colors.size() / 3; i++)
    	    	tfn_colors_3f.emplace_back(tfn_colors[3*i], tfn_colors[3*i+1], tfn_colors[3*i+2]);
    	    tfn_opacities.push_back(0.f);
    	    tfn_opacities.push_back(1.f);
    		
    	    ospray::cpp::TransferFunction tfn("piecewiseLinear");
    	    tfn.setParam("color", ospray::cpp::CopiedData(tfn_colors_3f));
            tfn.setParam("opacity", ospray::cpp::CopiedData(tfn_opacities));
            tfn.setParam("valueRange", vec2f(-0.1f, 0.1f));
    	    tfn.commit();
    	    glfwOspWindow.tfn = tfn;
    	    
	    // volume
	    ospray::cpp::Volume volume("structuredRegular");
	    volume.setParam("gridOrigin", vec3f(0.f,0.f,0.f));
	    volume.setParam("gridSpacing", vec3f(10.f / reduce_max(volumeDimensions)));
	    volume.setParam("data", ospray::cpp::SharedData(glfwOspWindow.voxel_data->data(), volumeDimensions));
	    volume.setParam("dimensions", volumeDimensions);
	    volume.commit();
	    // put the mesh into a model
	    ospray::cpp::VolumetricModel model(volume);
	    
	    model.setParam("transferFunction", glfwOspWindow.tfn);
	    model.commit();
	    glfwOspWindow.model = model;
	    
	    group.setParam("volume", ospray::cpp::CopiedData(model));
	
	
	group.commit();

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
    	glfwOspWindow.arcballCamera.reset(new ArcballCamera(glfwOspWindow.world.getBounds<box3f>(), windowSize));
    	glfwOspWindow.arcballCamera->updateWindowSize(windowSize);
    	std::cout << glfwOspWindow.world.getBounds<box3f>() << "\n";
    
	
	// create renderer, choose Scientific Visualization renderer
	ospray::cpp::Renderer *renderer = &glfwOspWindow.renderer;;

	// complete setup of renderer
	renderer->setParam("aoSamples", 1);
	renderer->setParam("backgroundColor", 0.0f); // white, transparent
	renderer->commit();

	
	ospray::cpp::Camera* camera = &glfwOspWindow.camera;
	    
	camera->setParam("aspect", imgSize.x / (float)imgSize.y);
	camera->setParam("position", glfwOspWindow.arcballCamera->eyePos());
	camera->setParam("direction", glfwOspWindow.arcballCamera->lookDir());
	camera->setParam("up", glfwOspWindow.arcballCamera->upDir());
	camera->commit(); // commit each object to indicate modifications are done
	
	    
	    
    glfwOspWindow.renderNewFrame();
    
    
    glfwMakeContextCurrent(glfwWindow);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    ImGui_ImplGlfwGL3_Init(glfwWindow, true);
    ImGui::StyleColorsDark();
    
    auto fb = glfwOspWindow.framebuffer.map(OSP_FB_COLOR);
    init(fb);
    glfwOspWindow.framebuffer.unmap(fb);
    glfwOspWindow.setFunc();
    glfwOspWindow.reshape(windowSize.x, windowSize.y);
    glfwSetInputMode(glfwWindow, GLFW_STICKY_KEYS, GL_TRUE);
    
    auto t1 = std::chrono::high_resolution_clock::now();
    auto t2 = std::chrono::high_resolution_clock::now();

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
      glfwMakeContextCurrent(glfwWindow);
      glfwSwapBuffers(glfwWindow);
      

      t2 = std::chrono::high_resolution_clock::now();
      auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
      glfwSetWindowTitle(glfwWindow, (std::string("Render FPS:")+std::to_string(int(1.f / time_span.count()))).c_str());

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(glfwWindow, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
	   glfwWindowShouldClose(glfwWindow) == 0 );
    
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

