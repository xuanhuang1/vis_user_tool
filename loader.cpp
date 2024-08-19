#include "loader.h"
#include <chrono>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "json.hpp"

//#include "stb_image.h"
//#include "util.h"
#include "ext/glm/gtx/string_cast.hpp"

using json = nlohmann_loader::json;

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

visuser::Camera visuser::interpolate(visuser::Camera &a, visuser::Camera &b, glm::vec2 range, uint32_t f){
	float val = (f - range[0]) / float(range[1] - range[0]);
	glm::vec3 pos = mix(a.pos, b.pos, val);
	glm::vec3 dir = glm::normalize(mix(a.dir, b.dir, val));
	glm::vec3 up  = glm::normalize(mix(a.up, b.up, val));
	return visuser::Camera(pos, dir, up);
}
    
void visuser::jsonFromFile(const char* name, nlohmann_loader::json &j){
    std::ifstream cfg_file(name);
    if (!cfg_file) {
        std::cerr << "[error]: Failed to open config file " << name << "\n";
        throw std::runtime_error("Failed to open input config file");
    }
    cfg_file >> j;
    std::cout << "Reading " << name <<"\n";
}
    
    
visuser::AniObjWidget::AniObjWidget(const nlohmann_loader::json in_file){
    config = in_file;
}


void visuser::AniObjWidget::init(){
    load_info(); 
    load_cameras(); 
    load_tfs();
}


void visuser::AniObjWidget::init_from_json(const nlohmann_loader::json in_file){
    config = in_file;
    load_info();
    load_cameras();
    load_tfs();
}

void visuser::AniObjWidget::load_info(){
    file_name 	= config["data"]["name"];
    type_name 	= config["data"]["type"];
    world_bbox  = get_vec3f(config["data"]["world_bbox"]);
    dims 	= get_vec3i(config["data"]["dims"]);
    frameRange 	= get_vec2i(config["data"]["frameRange"]);
    currentF 	= frameRange[0];
    
    if (config["data"].contains("backgroundMap")) {
    	bgmap_name = config["data"]["backgroundMap"];
    }else bgmap_name = "";
	
    std::cout << "end load info\n";
}

void visuser::AniObjWidget::print_info(){
    std::cout << "data info...\n\n"
	      << "input file: " << file_name << "\n"
	      << "type: " << type_name <<"\n"
	      << "dims: " << dims[0] <<" "<< dims[1] <<" "<<dims[2]<<"\n"
	      << "output frame range: " << frameRange[0] <<" "<<frameRange[1]<<"\n";
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
    std::cout << "end load cames\n";
}

void visuser::AniObjWidget::load_tfs(){
    const std::vector<json> &tf_set = config["transferFunc"].get<std::vector<json>>();
    colors = tf_set[0]["colors"].get<std::vector<float>>();
    opacities = tf_set[0]["opacities"].get<std::vector<float>>();
    tfRange = get_vec2f(tf_set[0]["range"]);
    std::cout << "end load tfs\n";
}

void visuser::AniObjWidget::overwrite_data_info(std::string f_name, glm::vec3 d){
    file_name = f_name;
    dims = d;
}

void visuser::AniObjWidget::advanceFrame(){
    currentCam = interpolate(cameras[0], cameras[1], frameRange, currentF);
	
    // do nothing for transfer function now
	
    currentF++;
}


// object handler

visuser::AniObjHandler::AniObjHandler(const char* filename){
    init(filename);
}

void visuser::AniObjHandler::init(const char* filename){
    jsonFromFile(filename, header_config);
    is_header = header_config["isheader"];
    
    if (!is_header){
    	// not a header, read again as plain kf file
    	widgets.resize(1);
    	widgets[0].init_from_json(header_config);
    }else{
    	// is a header, read all kf files
    	auto filenames = header_config["file_list"].get<std::vector<json>>();
    	auto datanames = header_config["data_list"].get<std::vector<json>>();
	std::filesystem::path p = std::filesystem::absolute(filename).parent_path();
	std::string p_str = p.generic_string() + "/";
	std::cout << "path: "<< p_str <<"\n";
    	widgets.resize(filenames.size());
    	for (size_t i=0; i<filenames.size(); i++){
	    nlohmann_loader::json config;
	    std::string kf_name = filenames[i]["keyframe"];
	    jsonFromFile((p_str+kf_name).c_str(), config);
	    widgets[i].init_from_json(config);
	    if (!filenames[i]["data_i"].is_null()){
		uint32_t data_i = filenames[i]["data_i"];
		if (datanames.size() > data_i){ 
		    widgets[i].overwrite_data_info(datanames[data_i]["name"], 
						   get_vec3i(datanames[data_i]["dims"]));
		    std::cout << "overwriting " << kf_name 
			      << " to \n  data: " << widgets[i].file_name
			      << " \n  dims: " << glm::to_string(widgets[i].dims)
			      << "\n";
		}	
	    }
    	}
    }
}


void visuser::writeSampleJsonFile(std::string meta_file_name){
    std::vector<uint32_t> data_i_list_kf = {0, 0, 1};
    std::map<uint32_t, uint32_t > data_i_list;
    nlohmann_loader::ordered_json j;
    std::string base_file_name = meta_file_name+"_kf";
    
    j["isheader"] = true;
    
    for (auto i: data_i_list_kf){
	if (data_i_list.find(i) == data_i_list.end()){
	    uint32_t idx = data_i_list.size();
	    j["data_list"][idx]["name"] = "<file "+std::to_string(idx)+">";
	    j["data_list"][idx]["dims"] = {(idx+1)*100, (idx+1)*100, (idx+1)*100};
	    data_i_list[i] = idx;
	}
    }
	
    // export all key frames to json file
    // write a header of file names 
    for (size_t i=0; i<data_i_list_kf.size();i++){
	std::string file_name = base_file_name + std::to_string(i) + ".json";
	j["file_list"][i]["keyframe"] = file_name;
	j["file_list"][i]["data_i"] = data_i_list[data_i_list_kf[i]];

	// write json for each keyframe interval
	nlohmann_loader::ordered_json tmp_j;
	tmp_j["isheader"] = false;
	tmp_j["data"]["type"] = "structured";
	tmp_j["data"]["name"] = "";
	tmp_j["data"]["dims"] = {100, 100, 100};
	tmp_j["data"]["world_bbox"] = {10, 10, 10};
	tmp_j["data"]["frameRange"] = {i*5, (i+1)*5};

	// cameras
	for (size_t j=0; j<2; j++)
	    {
		nlohmann_loader::ordered_json tmp_cam;
		tmp_cam["frame"] = (i+j)*5;
		for (size_t c=0; c<3; c++){
		    tmp_cam["pos"].push_back(c);
		    tmp_cam["dir"].push_back(c);
		    tmp_cam["up"].push_back(c);
		}
		tmp_j["camera"].push_back(tmp_cam);
	    }

	// tf
	tmp_j["transferFunc"][0]["frame"] = i*5;
	tmp_j["transferFunc"][0]["range"] = {-1, 1};
        tmp_j["transferFunc"][0]["colors"].push_back(0);
	tmp_j["transferFunc"][0]["colors"].push_back(0);
	tmp_j["transferFunc"][0]["colors"].push_back(255);
        tmp_j["transferFunc"][0]["opacities"].push_back(0);
	tmp_j["transferFunc"][0]["opacities"].push_back(1);
	    
	std::ofstream o(file_name);
	o << std::setw(4)<< tmp_j <<std::endl;
	o.close();
    }
    std::ofstream o_meta(meta_file_name+".json");
    o_meta << std::setw(4) << j <<std::endl;
    o_meta.close();
}





