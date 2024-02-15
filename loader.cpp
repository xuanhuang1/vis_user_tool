#include "loader.h"
#include <chrono>
#include <cstdio>
#include <iostream>
#include "json.hpp"

//#include "stb_image.h"
//#include "util.h"
#include "ext/glm/gtx/string_cast.hpp"

using json = nlohmann::json;

visuser::Camera::Camera(): pos(glm::vec3(1,0,0)), dir(glm::vec3(-1,0,0)), up(glm::vec3(0,0,1)){
	frame = 0;
}

visuser::Camera::Camera(const glm::vec3 &pos, const glm::vec3 &dir, const glm::vec3 &up)
	: pos(pos), dir(dir), up(up){
	frame = 0;	
}

visuser::Camera::Camera(const glm::vec3 &pos, const glm::vec3 &dir, const glm::vec3 &up, uint32_t f)
	: pos(pos), dir(dir), up(up), frame(f){
}

    
void visuser::Camera::print(){
	std::cout << "Camera: \n pos: [" <<glm::to_string(this->pos) << "], "
		  << "\n dir: [" <<glm::to_string(this->dir) << "], "
		  << "\n up : [" <<glm::to_string(this->up) << "]\n";
	
}

visuser::Camera interpolate(visuser::Camera &a, visuser::Camera &b, glm::vec2 range, uint32_t f){
	float val = (f - range[0]) / float(range[1] - range[0]);
	glm::vec3 pos = mix(a.pos, b.pos, val);
	glm::vec3 dir = glm::normalize(mix(a.dir, b.dir, val));
	glm::vec3 up  = glm::normalize(mix(a.up, b.up, val));
	return visuser::Camera(pos, dir, up);
}
    
    
    
visuser::AniObjWidget::AniObjWidget(const nlohmann::json in_file){
	config = in_file;
}

void visuser::AniObjWidget::load_info(){
	file_name 	= config["data"]["name"];
	type_name 	= config["data"]["type"];
	dims 		= get_vec3i(config["data"]["dims"]);
	frameRange 	= get_vec2i(config["data"]["frameRange"]);
	currentF 	= frameRange[0];
	
	// load z list, length must == dims.z
	zMapping 	= config["data"]["zMapping"].get<std::vector<float>>();
	if (zMapping.size() != dims.z) std::cerr << "unmatched z mapping size!\n";
}

void visuser::AniObjWidget::print_info(){
	std::cout << "data info...\n\n"
		  << "input file: " << file_name << "\n"
		  << "type: " << type_name <<"\n"
		  << "dims: " << dims[0] <<" "<< dims[1] <<" "<<dims[2]<<"\n"
		  << "output frame range: " << frameRange[0] <<" "<<frameRange[1]<<"\n";
	std::cout << "z mapping: [";
	for (auto z : zMapping) std::cout << z <<" ";
	std::cout << "]\n\nEnd data log...\n\n";
}

void visuser::AniObjWidget::load_cameras(){
    	const std::vector<json> &camera_set = config["camera"].get<std::vector<json>>();
	for (size_t i = 0; i < camera_set.size(); ++i) {
	    const auto &c = camera_set[i];
	    cameras.push_back(Camera(get_vec3f(c["pos"]),
					 get_vec3f(c["dir"]),
					 get_vec3f(c["up"]),
					 c["frame"].get<uint32_t>()));
	}
	currentCam = cameras[0];
}

void visuser::AniObjWidget::load_tfs(){
    	const std::vector<json> &tf_set = config["transferFunc"].get<std::vector<json>>();
	colors = tf_set[0]["colors"].get<std::vector<float>>();
	opacities = tf_set[0]["opacities"].get<std::vector<float>>();
	tfRange = get_vec2i(tf_set[0]["range"]);
}


void visuser::AniObjWidget::advanceFrame(){
	currentCam = interpolate(cameras[0], cameras[1], frameRange, currentF);
	
	// do nothing for transfer function now
	
	currentF++;
}





