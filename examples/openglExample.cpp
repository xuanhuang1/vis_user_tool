//========================================================================
// OpenGL triangle example
// Copyright (c) Camilla Löwy <elmindreda@glfw.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================
//! [code]

#define GLAD_GL_IMPLEMENTATION
#include "glad/gl.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../stb_image_write.h"


#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include "../ext/glm/gtc/matrix_transform.hpp"


#include "../loader.h"
#include <fstream>

using namespace visuser;


typedef struct Vertex
{
    glm::vec3 pos;
    glm::vec3 col;
} Vertex;

static const Vertex vertices[] =
{
    { { -1.0f, -1.0f, 3.0f }, { 0.9f, 0.5f, 0.5f}},
    { {  -1.0f, 1.0f, 3.0f }, { 0.8f, 0.8f, 0.8f}},
    { {  1.0f, -1.0f, 3.0f }, { 0.8f, 0.8f, 0.8f}},
    { {  -1.0f, 1.0f, 3.0f }, { 0.8f, 0.8f, 0.8f}},
    { {  1.0f, -1.0f, 3.0f }, { 0.8f, 0.8f, 0.8f}},
    { {  1.0f, 1.0f, 3.0f  }, { 0.5f, 0.9f, 0.5f}}
};

static const char* vertex_shader_text =
"#version 330\n"
"uniform mat4 MVP;\n"
"in vec3 vCol;\n"
"in vec3 vPos;\n"
"out vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 1.0);\n"
"    color = vCol;\n"
"}\n";

static const char* fragment_shader_text =
"#version 330\n"
"in vec3 color;\n"
"out vec4 fragment;\n"
"void main()\n"
"{\n"
"    fragment = vec4(color, 1.0);\n"
"}\n";

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(int argc, const char **argv)
{
    std::vector<std::string> args(argv, argv + argc);
    
    json config;
    std::string prefix;
    for (int i = 1; i < argc; ++i) {
        if (args[i] == "-h") {
            std::cout << "./mini_vistool <config.json> [options]\n";
            return 0;
        } else {
            std::ifstream cfg_file(args[i].c_str());
            if (!cfg_file) {
                std::cerr << "[error]: Failed to open config file " << args[i] << "\n";
                throw std::runtime_error("Failed to open input config file");
            }
            cfg_file >> config;
        }
    }
    
  std::vector<Camera> cams = load_cameras(config["camera"].get<std::vector<json>>(), 10);

  for(auto &c : cams)
	c.print();
	
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(640, 480, "OpenGL Triangle", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);

    // NOTE: OpenGL error checks have been omitted for brevity

    GLuint vertex_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);

    const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);

    const GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    const GLint mvp_location = glGetUniformLocation(program, "MVP");
    const GLint vpos_location = glGetAttribLocation(program, "vPos");
    const GLint vcol_location = glGetAttribLocation(program, "vCol");

    GLuint vertex_array;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);
    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void*) offsetof(Vertex, pos));
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void*) offsetof(Vertex, col));
                          
                          
    vec3 eye = {0, 0, 0};
    vec3 dir = {0.1, 0, 1};
    vec3 up = {0, 1, 0};
    
    stbi_flip_vertically_on_write(1);


    uint32_t renderCount = 0;

    while (/* !glfwWindowShouldClose(window) &&*/ (renderCount < cams.size()))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        const float ratio = width / (float) height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

	for (int k=0; k<3; k++){
	    eye[k] = cams[renderCount].pos[k];
	    up[k]  = cams[renderCount].up[k];
	    dir[k] = cams[renderCount].dir[k];
	}
        glm::mat4 m, p, mvp;
        p = glm::perspective(glm::radians(60.f), ratio, 0.1f, 200.f);
        m = glm::lookAt(eye, eye+dir, up);
        mvp = p * m;

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) &mvp);
        glBindVertexArray(vertex_array);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glfwSwapBuffers(window);
        glfwPollEvents();

        std::string imgName = "outframe_gl_"+std::to_string(renderCount)+".jpg";
	std::vector<char> buffer(width*height*3);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
	stbi_write_jpg(imgName.c_str(), width, height, 3, buffer.data(), 90);

	renderCount++;
	
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

//! [code]
