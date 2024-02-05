#pragma once

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <memory>
#include <vector>

#include "json.hpp"
#include "util.h"
#include "ext/glm/glm.hpp"

namespace visuser{

    struct Camera {
    	glm::vec3 pos;
    	glm::vec3 dir;
    	glm::vec3 up;
    	uint32_t frame; 

	Camera();
    	Camera(const glm::vec3 &pos, const glm::vec3 &dir, const glm::vec3 &up);
    	Camera(const glm::vec3 &pos, const glm::vec3 &dir, const glm::vec3 &up, uint32_t f);
    	
	void print();
    };
    
    
    struct AniObjWidget{
    	// input data
    	nlohmann::json config;
    	std::string file_name;		// path to input raw file
    	std::string type_name;		// unstructured grid for now
    	glm::vec3 dims;			// x, y, z
    	std::vector<float> zMapping;	// physical depth mapping, size()==dim.z
    	
    	// animation widget settings
    	glm::vec2 frameRange;				// the range of animating this object
    	std::vector<Camera> cameras; 			// list of camera keyframes
    	glm::vec2 tfRange;				// TF range 
    	
    	// init
    	AniObjWidget(const nlohmann::json in_file);
    	void load_info();
    	void print_info();
    	void load_cameras();
    	void load_tfs();
    	
    	// animation
    	void getFrameCam(Camera &cam){cam = currentCam;}
    	void getFrameTF(std::vector<float> &c, std::vector<float> &o){c = colors; o = opacities;}
    	void advanceFrame();
    	
    	private:
    	uint32_t currentF = 0;
    	Camera currentCam;
    	std::vector<float> colors;
    	std::vector<float> opacities;
    	
    };
}


