#pragma once

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <memory>
#include <vector>

#include "json.hpp"
#include "util.h"
#include "ext/glm/glm.hpp"

//using json = nlohmann::json;

namespace visuser{
    // data
    struct Volume{
    };

    struct Mesh{
    };	

    // initial setting
    struct Camera {
    	glm::vec3 pos;
    	glm::vec3 dir;
    	glm::vec3 up;

    	Camera(const glm::vec3 &pos, const glm::vec3 &dir, const glm::vec3 &up);

	void print();
    };

    std::vector<Camera> load_cameras(const std::vector<nlohmann::json> &camera_set,
				     const float radius);

    // actions
    struct CameraAction{};
    struct VolumeIsosurface{};
    struct TfEdit{};
}

//std::vector<cpp::TransferFunction> load_colormaps(const std::vector<std::string> &files,
//                                                  const vec2f &value_range);

//std::shared_ptr<std::vector<uint8_t>> load_raw_volume(const std::string &file,
//                                                      const std::string &dtype,
//                                                      const vec3i &vol_dims,
//                                                      const vec3i &brick_dims,
//                                                      const vec3i &brick_offset);
