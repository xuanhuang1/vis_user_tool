#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "imgui/imgui.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace keyframe {

class KeyframeWidget {
    struct vec2f {
        float x, y;

        vec2f(float c = 0.f);
        vec2f(float x, float y);
        vec2f(const ImVec2 &v);

        float length() const;

        vec2f operator+(const vec2f &b) const;
        vec2f operator-(const vec2f &b) const;
        vec2f operator/(const vec2f &b) const;
        vec2f operator*(const vec2f &b) const;
        operator ImVec2() const;
    };

  
    size_t selected_point = -1;

    bool clicked_on_item = false;
  std::string guiText = "color map";

public:
    KeyframeWidget();
    std::vector<float> time_ctrl_points = {0.f, 1.f};
    std::vector<float> dataIndex_ctrl_points = {0.f, 1.f};
    std::vector<float> cam_ctrl_points = {0.f, 1.f};
    std::vector<float> tfn_ctrl_points = {0.f, 1.f};

    // Add the transfer function UI into the currently active window
    void draw_ui();

    // Returns true if the colormap was updated since the last
    // call to draw_ui
    //bool changed() const;

    void setGuiText(std::string in) { guiText = in;}
private:
    void draw_attribute_line(ImDrawList *draw_list, const vec2f view_scale, const vec2f view_offset, std::vector<float>& pts, uint32_t index, std::string txt);
    float display_offsets[4]  = {0.85f, 0.5f, 0.3f, 0.1f};
};
}
