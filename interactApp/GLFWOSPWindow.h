#include "ospray/ospray_cpp.h"
#include "ospray/ospray_cpp/ext/rkcommon.h"
#include <vector>


#define GLFW_INCLUDE_NONE
#include <GL/glew.h>
#include <GLFW/glfw3.h>
// imgui
#include "./imgui/imgui.h"
#include "./imgui/imgui_impl_glfw_gl3.h"

#include "../loader.h"
#include "ArcballCamera.h"
#include "TransferFunctionWidget.h"
#include "clipping_plane.h"
#include "app_params.h"

#include "KeyframeWidget.h"
#include "../mesh/rectMesh.h"

using namespace rkcommon::math;

class GLFWOSPWindow{
public:
    // image size
    vec2i imgSize{800, 600};
    vec2i windowSize{800,600};
    unsigned int texture;
    unsigned int guiTextures[128];
    unsigned int guiTextureSize = 0;
    GLFWwindow *glfwWindow = nullptr;

    int world_size = 10;
    
    ospray::cpp::Camera camera{"perspective"};
    ospray::cpp::Renderer renderer{"scivis"};
    ospray::cpp::World world;
    ospray::cpp::Instance instance;
    ospray::cpp::VolumetricModel model;
    ospray::cpp::Volume volume;
    std::vector<float> * voxel_data; // pointer to voxels data
    vec3i volumeDimensions;
    float* all_data_ptr; // pointer to all data
    int count = 1;
  
    static GLFWOSPWindow *activeWindow;
    ospray::cpp::FrameBuffer framebuffer;
    std::unique_ptr<ArcballCamera> arcballCamera;
    vec2f previousMouse{vec2f(-1)};

    vec2f tf_range;
    ospray::cpp::TransferFunction tfn{"piecewiseLinear"};
    tfnw::TransferFunctionWidget tfn_widget;
    keyframe::KeyframeWidget kf_widget;
    visuser::RectMesh rectMesh;
    std::vector<float> clippingBox = {-1,0,-1,0};
    std::array<ClippingPlane, 4> clipping_planes;
    AppParam<std::array<ClippingPlaneParams, 4>> clipping_params;
  
    
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

    void initVolume(vec3i dim, visuser::AniObjWidget &widget);
    void initVolumeOceanZMap(vec3i volumeDimensions, float bb_x);
    void initClippingPlanes();
    
    void preRenderInit();    
    void renderNewFrame(){
	framebuffer.clear();
	// render one frame
	framebuffer.renderFrame(renderer, camera, world);
    }

    void buildUI();
    void playAnimationFrame();
    void printSessionSummary();

};
