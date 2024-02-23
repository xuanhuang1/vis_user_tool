#include "GLFWOSPWindow.h"

//
// Lower level rendering contents
//


void GLFWOSPWindow::display()
{
    if (kf_widget.play) playAnimationFrame(); 
  
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

void GLFWOSPWindow::preRenderInit(){
    renderNewFrame();
		
    glfwMakeContextCurrent(glfwWindow);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    ImGui_ImplGlfwGL3_Init(glfwWindow, true);
    ImGui::StyleColorsDark();
    
    auto fb = framebuffer.map(OSP_FB_COLOR);
	
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
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

    framebuffer.unmap(fb);
    setFunc();
    reshape(windowSize.x, windowSize.y);
    glfwSetInputMode(glfwWindow, GLFW_STICKY_KEYS, GL_TRUE);
}



//
//               User input
//

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

//
//               GUI
//

void GLFWOSPWindow::buildUI(){
    static float f = 1.f;
    static bool changeF = false;
    static int curTime = 0;
    static int data_time = 0;
    ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize;
    bool camera_need_commit = false;
    bool tf_need_commit = false;

    ImGui::Begin("Menu Window", nullptr, flags);

    ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        
    if (ImGui::TreeNode("Transfer Function")){
	if (ImGui::SliderFloat("float", &f, 1.0f, 10.0f)){changeF = true;}
    
  	if (tfn_widget.changed() || changeF) {
	    std::vector<float> tmpOpacities, tmpColors; 
	    tfn_widget.get_osp_colormapf(tmpColors, tmpOpacities);
	    for (uint32_t i=0;i<tmpOpacities.size();i++){
	    	tmpOpacities[i] = tmpOpacities[i]*f;
	    }
	    
	    tfn_widget.setUnchanged();
	    changeF = false;

	    // update opacities only
	    tfn.setParam("opacity", ospray::cpp::CopiedData(tmpOpacities));
	    tfn.commit();
	    model.commit();
	}

  
	tfn_widget.draw_ui();
	ImGui::TreePop();
    }
    
    if (ImGui::TreeNode("Data time")){
  	if (ImGui::SliderInt("time", &data_time, 0, count-1)) {
	    long long offset = data_time * volumeDimensions.long_product();
	    //for (long long i =0 ; i < volumeDimensions.long_product(); i++){
            //    (*voxel_data)[i] = all_data_ptr[i+offset];
            //}
	    
	}
	ImGui::TreePop();
    }
    
    if (ImGui::TreeNode("Animation keyframe")){
  	//if (ImGui::SliderInt("time", &curTime, 0, kf_widget.timeFrameMax)) {
	//
	//}
	
	//ImGui::SameLine();
        //if (ImGui::Button("export")) {}
	
	kf_widget.draw_ui();
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,255,0,255));
	ImGui::Text("active kf:");
	ImGui::PopStyleColor();
	ImGui::SameLine();
	if (ImGui::Button("setF")) {}
	ImGui::SameLine();
	if (ImGui::Button("loadF")) {
	  
	    ArcballCamera cam;
	    std::vector<float> tmpOpacities, tmpColors;
	    int data_index;
	    kf_widget.loadKeyFrame(cam, tmpColors, tmpOpacities, data_index);
	    
	    // update camera
	    arcballCamera->set(cam);
	    camera_need_commit = true;

	    //update tf
	    //tfn_widget.set_osp_colormapf(tmpColors, tmpOpacities);
	    //tfn_widget.changed();
        }
	ImGui::SameLine();
        if (ImGui::Button("play")) {kf_widget.play = true;}

	if (kf_widget.kfs.size() == 0) kf_widget.record_frame = 0;
	
	// record keyframe if requested
	if (kf_widget.record_frame != -1){
	    std::vector<float> tmpOpacities, tmpColors; 
	    tfn_widget.get_osp_colormapf(tmpColors, tmpOpacities);
	    kf_widget.recordKeyFrame(*arcballCamera, tmpColors, tmpOpacities, data_time);

	}

	if (camera_need_commit){
	    camera.setParam("aspect", windowSize.x / float(windowSize.y));
	    camera.setParam("position", arcballCamera->eyePos());
	    camera.setParam("direction", arcballCamera->lookDir());
	    camera.setParam("up", arcballCamera->upDir());
	    camera.commit();
	}
	
	ImGui::TreePop();
    }
	  
	  
	  
    ImGui::End();
}

void GLFWOSPWindow::playAnimationFrame(){
    if (kf_widget.playFrame >= kf_widget.kfs.back().timeFrame){
	kf_widget.playFrame = -1;
	kf_widget.play = false;
    }
    
    // load current frame info
    float cam[9];
    std::vector<float> tmpOpacities, tmpColors;
    int data_index;
    kf_widget.getFrameFromKF(cam, tmpColors, tmpOpacities, data_index, kf_widget.playFrame);
    vec3f pos(cam[0], cam[1], cam[2]);
    vec3f dir(cam[3], cam[4], cam[5]);
    vec3f up(cam[6], cam[7], cam[8]);

    
    // update camera
    camera.setParam("aspect", windowSize.x / float(windowSize.y));
    camera.setParam("position", pos);
    camera.setParam("direction", dir);
    camera.setParam("up", up);
    camera.commit();
	    
    // update opacities only
    tfn.setParam("opacity", ospray::cpp::CopiedData(tmpOpacities));
    tfn.commit();
    model.commit();

    kf_widget.playFrame++;
}
