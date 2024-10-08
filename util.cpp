#include "util.h"
#include <algorithm>
#include <array>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>


using json = nlohmann_loader::json;

std::string get_file_content(const std::string &fname)
{
    std::ifstream file{fname};
    if (!file.is_open()) {
        std::cout << "Failed to open file: " << fname << std::endl;
        return "";
    }
    return std::string{std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};
}

std::string get_file_extension(const std::string &fname)
{
    const size_t fnd = fname.find_last_of('.');
    if (fnd == std::string::npos) {
        return "";
    }
    return fname.substr(fnd + 1);
}

std::string get_file_basename(const std::string &path)
{
    size_t fname_offset = path.find_last_of('/');
    if (fname_offset == std::string::npos) {
        return path;
    }
    return path.substr(fname_offset + 1);
}

std::string get_file_basepath(const std::string &path)
{
    size_t end = path.find_last_of('/');
    if (end == std::string::npos) {
        return path;
    }
    return path.substr(0, end);
}

bool starts_with(const std::string &str, const std::string &prefix)
{
    return std::strncmp(str.c_str(), prefix.c_str(), prefix.size()) == 0;
}

std::vector<vec3> generate_fibonacci_sphere(const size_t n_points, const float radius)
{
    const float increment = M_PI * (3.f - std::sqrt(5.f));
    const float offset = 2.f / n_points;
    std::vector<vec3> points;
    points.reserve(n_points);
    for (size_t i = 0; i < n_points; ++i) {
        const float y = ((i * offset) - 1.f) + offset / 2.f;
        const float r = std::sqrt(1.f - y * y);
        const float phi = i * increment;
        const float x = r * std::cos(phi);
        const float z = r * std::sin(phi);
        points.emplace_back(x * radius, y * radius, z * radius);
    }
    return points;
}

