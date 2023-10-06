#include "loader.h"
#include <chrono>
#include <cstdio>
#include <iostream>
#include "json.hpp"

//#include "stb_image.h"
//#include "util.h"
#include "ext/glm/gtx/string_cast.hpp"

using json = nlohmann::json;

visuser::Camera::Camera(const glm::vec3 &pos, const glm::vec3 &dir, const glm::vec3 &up)
	: pos(pos), dir(dir), up(up)
    {
    }

    
void visuser::Camera::print(){
	std::cout << "Camera: \n pos: [" <<glm::to_string(this->pos) << "], "
		  << "\n dir: [" <<glm::to_string(this->dir) << "], "
		  << "\n up : [" <<glm::to_string(this->up) << "]\n";
	
    }

std::vector<visuser::Camera> visuser::load_cameras(const std::vector<json> &camera_set, const float radius)
    {
	std::vector<Camera> cameras;
	for (size_t i = 0; i < camera_set.size(); ++i) {
	    const auto &c = camera_set[i];
	    if (c.find("orbit") != c.end()) {
		const float orbit_radius = radius;
		auto orbit_points = generate_fibonacci_sphere(c["orbit"].get<int>(), orbit_radius);
		for (const auto &p : orbit_points) {
		    cameras.push_back(Camera(p, normalize(-p), vec3(0, 1, 0)));
		}
	    } else {
		cameras.push_back(Camera(get_vec3f(c["pos"]),
					 get_vec3f(c["dir"]),
					 get_vec3f(c["up"])));
	    }
	}
	return cameras;
    }



