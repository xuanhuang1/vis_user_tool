// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <fstream>

#include <string.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

//#include "../imgui/imgui.h"
#include "../imgui/examples/imgui_impl_glfw.h"
#include "../imgui/examples/imgui_impl_opengl3.h"


// include MC 
#include "isosurface/MarchingCube.h"



// change to 1 for windows vs proj
#define USE_WIN_VSPROJ 0




const char* vertex_shader =
"#version 330\n"
"uniform mat4 MVP;"
"uniform mat4 V;"
"in vec3 vp;"
  "in vec3 norm;"
  "out vec3 normal;"
  "out vec3 pos;"
"void main() {"
"  gl_Position = MVP * vec4(vp, 1);"
  "normal = norm;"
  "pos =  vec4(vp, 1).xyz;"
"}";

const char* fragment_shader =
  "#version 330\n"
  "uniform mat4 V;"
  "uniform vec3 EyePos;"

  "out vec4 frag_colour;"
  "in vec3 normal;"
  "in vec3 pos;"
  "void main() {"
  "  vec3 n = normalize(( V * vec4(normal,0)).xyz);"
  "  vec3 l = normalize(( V*vec4(0, 0, -0.5, 1)).xyz - (V*vec4(pos,1)).xyz);"
  "  vec3 eye = normalize( - (V*vec4(pos,1)).xyz);"
  "  vec3 R = reflect(-l ,n);"
  "  float cosAlpha = clamp(dot(eye, R), 0, 1);"
  "  float cosTheta = clamp(dot(n, l), 0, 1);"
  "  vec3 surfColor = vec3(1,0,0);"
  "  frag_colour = vec4(surfColor*(cosTheta + pow(cosAlpha,10)), 1.0);"
  "}";

glm::vec3 cameraPos;
glm::vec3 cameraTarget;
glm::vec3 up;
float theta = 0.0;
float phi = 0.0;
float r = -2.0;
double previousMouseX = 0.0;
double previousMouseY = 0.0;
bool isMouseHeld = false;

//For GUI controls
bool showSkin = false;
bool showSkeleton = false;
bool showOrgans = false;
bool showNerves = false;
bool showEyes = false;

bool isVolumeViewPicked = true;
static bool showMenu = true;
static bool saveFirstFrame = true;

int windowX = 0;
int windowY = 0;
int windowWidth = 0;
int windowHeight = 0;



// Our vertices. Three consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
// A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
static  float box_points[] = {
    -0.5f,-0.5f,-0.5f, // triangle 1 : begin
    -0.5f,-0.5f, 0.5f,
    -0.5f, 0.5f, 0.5f, // triangle 1 : end
    0.5f, 0.5f,-0.5f, // triangle 2 : begin
    -0.5f,-0.5f,-0.5f,
    -0.5f, 0.5f,-0.5f, // triangle 2 : end
    0.5f,-0.5f, 0.5f,
    -0.5f,-0.5f,-0.5f,
    0.5f,-0.5f,-0.5f,
    0.5f, 0.5f,-0.5f,
    0.5f,-0.5f,-0.5f,
    -0.5f,-0.5f,-0.5f,
    -0.5f,-0.5f,-0.5f,
    -0.5f, 0.5f, 0.5f,
    -0.5f, 0.5f,-0.5f,
    0.5f,-0.5f, 0.5f,
    -0.5f,-0.5f, 0.5f,
    -0.5f,-0.5f,-0.5f,
    -0.5f, 0.5f, 0.5f,
    -0.5f,-0.5f, 0.5f,
    0.5f,-0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,
    0.5f,-0.5f,-0.5f,
    0.5f, 0.5f,-0.5f,
    0.5f,-0.5f,-0.5f,
    0.5f, 0.5f, 0.5f,
    0.5f,-0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,
    0.5f, 0.5f,-0.5f,
    -0.5f, 0.5f,-0.5f,
    0.5f, 0.5f, 0.5f,
    -0.5f, 0.5f,-0.5f,
    -0.5f, 0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,
    -0.5f, 0.5f, 0.5f,
    0.5f,-0.5f, 0.5f
};

enum RENDER_TYPE{
	SURFACE, VOLUME
};

float width = 800, height = 600;
glm::mat4 view, MVP, model, projection;


RENDER_TYPE render_mode = SURFACE; // CHANGE HERE FOR A DIFFERENT MODE


GLuint getShader(const char* v, const char* f){
	// load shader program
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &v, NULL);
	glCompileShader(vs);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &f, NULL);
	glCompileShader(fs);

	GLuint shader_program_surface = glCreateProgram();
	glAttachShader(shader_program_surface, fs);
	glAttachShader(shader_program_surface, vs);
	glLinkProgram(shader_program_surface);
	return shader_program_surface;
}



GLuint getShaderFromFile(const char* path_v, const char* path_f){
	// load shader program
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	
	std::string content;
	std::ifstream fileStream(path_v, std::ios::in);

	if(!fileStream.is_open()) {
	  std::cerr << "Could not read file "
		    << path_v << ". File does not exist."
		    << std::endl;
	}

	std::string line = "";
	while(!fileStream.eof()) {
	  std::getline(fileStream, line);
	  content.append(line+"\n");
	}
	fileStream.close();
	
	const GLchar* v_code = content.c_str();
	
	glShaderSource(vs, 1, &v_code, NULL);
	glCompileShader(vs);


	std::string contentF = "";
	std::ifstream fileStreamF(path_f, std::ios::in);
	if(!fileStreamF.is_open()) {
	  std::cerr << "Could not read file "
		    << path_f << ". File does not exist."
		    << std::endl;
	}

	std::string lineF = "";
	while(!fileStreamF.eof()) {
	  std::getline(fileStreamF, lineF);
	  contentF.append(lineF+"\n");
	}
	fileStreamF.close();
	
	const GLchar* f_code = contentF.c_str();
	
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &f_code, NULL);
	glCompileShader(fs);

	GLuint shader_program_surface = glCreateProgram();
	glAttachShader(shader_program_surface, fs);
	glAttachShader(shader_program_surface, vs);
	glLinkProgram(shader_program_surface);
	return shader_program_surface;
}


static void reorientCamera(float thetaChange, float phiChange, float rChange)
{

	theta += thetaChange;
	
	
	phi += phiChange;
	if (phi > 3.1415)
		phi -= 3.1415*2;
	if (phi < -3.1415)
		phi += 3.1415*2;
	
	r += rChange / 10.0;
	cameraPos = glm::vec3(r * sin(theta) * sin(phi), r * cos(theta) * sin(phi), r * cos(phi));
	up = glm::vec3(sin(theta) * cos(-phi), cos(theta) * cos(-phi), sin(-phi));
	
}

static void rotateLeft(double xVariance)
{
    reorientCamera(-xVariance, 0.0f, 0.0f);
}

static void rotateRight(double xVariance)
{
    reorientCamera(xVariance, 0.0f, 0.0f);
}

static void rotateUp(double yVariance)
{
    reorientCamera(0.0f, yVariance, 0.0f);
}

static void rotateDown(double yVariance)
{
    reorientCamera(0.0f, -yVariance, 0.0f);
}

static void zoomIn(double zVariance)
{
    reorientCamera(0.0f, 0.0f, zVariance);
}

static void zoomOut(double zVariance)
{
    reorientCamera(0.0f, 0.0f, -zVariance);
}


static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_S && action == GLFW_PRESS)
        render_mode = SURFACE;
    if (key == GLFW_KEY_V && action == GLFW_PRESS)
        render_mode = VOLUME;
    if (key == GLFW_KEY_M && action == GLFW_PRESS)
       showMenu = !showMenu;

    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
	rotateLeft(0.5f);
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
	rotateRight(0.5f);
    if (key == GLFW_KEY_UP && action == GLFW_PRESS)
	rotateUp(0.5f);
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
	rotateDown(0.5f);
    if (key == GLFW_KEY_I && action == GLFW_PRESS)
	zoomIn(0.1f);
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
	zoomOut(0.1f);
}

static void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	double xVariance = xpos - previousMouseX;
	double yVariance = ypos - previousMouseY;
	previousMouseX = xpos;
	previousMouseY = ypos;
	if (isMouseHeld )
	{
		reorientCamera(-xVariance / 10.0, -yVariance / 15.0, 0.0f);
	}
	else
	{
		reorientCamera(0.0f, 0.0f, 0.0f);
	}
}

void  mouse_click_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !(previousMouseX > windowX - 10 && previousMouseX < windowX + windowWidth + 10 && previousMouseY > windowY -10 && previousMouseY < windowY + windowHeight + 10))
	{
		isMouseHeld = true;
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		isMouseHeld = false;
	}
}

void  scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	//r = yoffset;
	//reorientCamera(0.0f, 0.0f, r);
}


void generate_colormap(std::vector<float> &tfnc_rgba, std::vector<glm::vec4> &col_pts, int isovalue, int range, int colormap_length){
  
  tfnc_rgba.clear();
  tfnc_rgba.resize(0);
  
  for (int i=0; i< colormap_length; i++){
    float colormap_stepsize = float(colormap_length)/(col_pts.size()-1);
    uint32_t col_index_last = uint32_t(i/colormap_stepsize) ;
    uint32_t col_index_next = col_index_last + 1;
    //if (col_index_next == col_pts.size()) col_index_next = col_index_last;
	  
    for (int j=0; j<3; j++)
      tfnc_rgba.push_back(col_pts[col_index_last][j] +
			  (col_pts[col_index_next][j] - col_pts[col_index_last][j])
			  *float(i%uint32_t(colormap_stepsize))/float(colormap_stepsize));
      
    if (abs(i -isovalue) > range)
      tfnc_rgba.push_back(0);
    else
      tfnc_rgba.push_back(col_pts[col_index_last][3] +
			  (col_pts[col_index_next][3] - col_pts[col_index_last][3])
			  *float(i%uint32_t(colormap_stepsize))/float(colormap_stepsize));
  }
}



int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	// Open a window and create its OpenGL context
	window = glfwCreateWindow( width, height, "", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);


	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Set Render Mode Callback
	glfwSetKeyCallback(window, key_callback);
	// Set camera control callback
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_click_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Create and link shader 
	GLuint shader_program_surface = getShader(vertex_shader, fragment_shader);


#if USE_WIN_VSPROJ
	GLuint shader_program_rayTrace = getShaderFromFile("../../../AIExample/src/rayTrace/rayt.vert",
							   "../../../AIExample/src/rayTrace/rayt.frag");
#else
	GLuint shader_program_rayTrace = getShaderFromFile("../../AIExample/src/rayTrace/rayt.vert",
							   "../../AIExample/src/rayTrace/rayt.frag");
#endif
	
	std::vector<float> vertices_surface;
	std::vector<float> normals_surface;

	
	// init gui selection index
	const char* items[] = { "Skin", "Skeleton", "Organs", "Eyes", "All(non-zero)"};

	//////// CHANGE HERE FOR PRESET VALUES ////////
	const int preset_isovals[] = {5, 135, 120, 180, 0};
	uint32_t item_selected_index = 5-1;

	int dim[3] = {500, 470, 136};
	int isovalue = preset_isovals[item_selected_index];
	int prev_isovalue = isovalue;
	int vol_range = 256;
	int prev_vol_range = vol_range;
	std::vector<char> inputData(dim[0]*dim[1]*dim[2]);
	std::vector<float> tfnc_rgba;
	

	// Read data
#if USE_WIN_VSPROJ
	std::ifstream file("../../../AIExample/data/frog.raw", std::ios::in | std::ios::binary);
#else
	std::ifstream file("../../AIExample/data/frog.raw", std::ios::in | std::ios::binary);
#endif
	if(!file.is_open()) {
	  std::cerr << "Could not read input file "<< std::endl;
	}
	
	for (int i=0; i<dim[0]*dim[1]*dim[2]; i++){
	  file.read((&inputData[i]), sizeof(inputData[i]));
  	}
  	file.close();

  	// output test for correctness, should be exactly the same
  	// std::ofstream ofile ("output.raw", std::ios::out | std::ios::binary);
  	// if (ofile.is_open()){
   	//  ofile.write(&inputData[0], sizeof(inputData[0]) * dim[0] * dim [1] * dim[2]);
  	//  ofile.close();
  	// }

	// fill simple blue-red transfer function
	// around isovalue
	std::vector<glm::vec4> col_pts;
	
	col_pts.push_back(glm::vec4(0.0, 0.0 ,1.0, 0.0));
	col_pts.push_back(glm::vec4(1.0, 1.0 ,0.5, 0.5));
	col_pts.push_back(glm::vec4(1.0, 0.0 ,0.0, 1.0));

	uint32_t colormap_length = 256;
	generate_colormap(tfnc_rgba, col_pts, isovalue, vol_range, colormap_length);
	

	// marching cube
	MarchingCube(inputData, dim, vertices_surface, normals_surface, isovalue);
	//dim[0] = 1;
	//dim[1] = 1;
	//dim[2] = 1;
	//MarchingCube({ 0, 0, 0, 0, 2, 2, 2, 2 }, dim, vertices_surface, normals_surface, 1);
	/*
	for (int i=0;i<vertices_surface.size()/3;i++){
	  normals_surface.push_back(0);
	  normals_surface.push_back(0);
	  normals_surface.push_back(-1);
	}
	*/	

	// set texture
	//glEnable(GL_TEXTURE_3D);
	//PFNGLTEXIMAGE3DPROC glTexImage3D;
	//glTexImage3D = (PFNGLTEXIMAGE3DPROC) glfwGetProcAddress("glTexImage3D");
	
	if(!GL_ARB_texture_non_power_of_two)
	  printf("NO NPOT TEXTURE EXTENSION!\n");
	  else printf("support not power of 2 extension\n");

			
	unsigned int texname, tfnc;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &texname);
	glBindTexture(GL_TEXTURE_3D, texname);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, dim[0], dim[1], dim[2], 0, GL_RED, 
	  GL_UNSIGNED_BYTE, &inputData[0]);


	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &tfnc);
	glBindTexture(GL_TEXTURE_1D, tfnc);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, tfnc_rgba.size()/4, 0, GL_RGBA, GL_FLOAT, &tfnc_rgba[0]);


	// Create vbos
	GLuint vbo_surface = 0;
	glGenBuffers(1, &vbo_surface);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_surface);
	glBufferData(GL_ARRAY_BUFFER, vertices_surface.size() * sizeof(float), &vertices_surface[0], GL_STATIC_DRAW);
	GLuint vbo_surface_normal = 0;
	glGenBuffers(1, &vbo_surface_normal);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_surface_normal);
	glBufferData(GL_ARRAY_BUFFER, normals_surface.size() * sizeof(float), &normals_surface[0], GL_STATIC_DRAW);

	
	GLuint vbo_volume = 0;
	glGenBuffers(1, &vbo_volume);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_volume);
	glBufferData(GL_ARRAY_BUFFER, 3*6*6 * sizeof(float), box_points, GL_STATIC_DRAW);

	// Create vaos
	GLuint vao_surface = 0;
	glGenVertexArrays(1, &vao_surface);
	glBindVertexArray(vao_surface);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_surface);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_surface_normal);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	GLuint vao_volume = 0;
	glGenVertexArrays(1, &vao_volume);
	glBindVertexArray(vao_volume);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_volume);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	
	
	// init cameras
	//cameraPos = glm::vec3(1.0f, 1.0f, -2.0f);  
	//cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	//up = glm::vec3(0.0f, 1.0f, 0.0f);
	reorientCamera(0,0,0);

	glm::vec3 cameraDirection = glm::normalize(cameraTarget - cameraPos);
	glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
	glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);
	
	view = glm::lookAt(cameraPos, cameraPos + cameraDirection, cameraUp);
	projection = glm::perspective(glm::radians(45.0f), (float)width/(float)height, 0.1f, 100.0f);

	
	MVP =  projection * view; // just let model = I 


	// Dark blue background
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);

	GLuint shader_program = shader_program_rayTrace;
	GLuint vao = vao_surface;
	int drawArraySize = vertices_surface.size()/3;


	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	//bool show_demo_window = true;

	GLenum err = glGetError();
	if(err != GL_NO_ERROR) // error
	  std::cout <<"err\n";

       

	do{
		// Clear the screen
		glEnable(GL_DEPTH_TEST);
		
		// Enable blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		glClear( GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT );

		// update parameters by selection
		bool update_isovalue = false;
		if (prev_isovalue != isovalue){
		  prev_isovalue = isovalue;
		  update_isovalue = true;
		}

		if (isVolumeViewPicked){
		  if (render_mode == SURFACE){
		    render_mode = VOLUME;
		    update_isovalue = true;
		  }
		}else{
		  if (render_mode == VOLUME){
		    render_mode = SURFACE;
		    update_isovalue = true;
		  }
		}

		if (render_mode == SURFACE){
			shader_program = shader_program_surface;
			vao = vao_surface;
			
			if (update_isovalue){
			  
			  vertices_surface.clear();
			  normals_surface.clear();
			  
			  // marching cube
			  MarchingCube(inputData, dim, vertices_surface, normals_surface, isovalue);
			  
			  if(!normals_surface.size()){
			    for (int i=0;i<vertices_surface.size()/3;i++){
			      normals_surface.push_back(0);
			      normals_surface.push_back(0);
			      normals_surface.push_back(-1);
			    }
			  }
			  
			  glBindBuffer(GL_ARRAY_BUFFER, vbo_surface);
			  glBufferData(GL_ARRAY_BUFFER, vertices_surface.size() * sizeof(float), &vertices_surface[0], GL_STATIC_DRAW);

			  glBindBuffer(GL_ARRAY_BUFFER, vbo_surface_normal);
			  glBufferData(GL_ARRAY_BUFFER, normals_surface.size() * sizeof(float), &normals_surface[0], GL_STATIC_DRAW);
	
			}
		        
			drawArraySize = vertices_surface.size()/3;
			
			  
		}else if (render_mode == VOLUME){
			shader_program = shader_program_rayTrace;
			vao = vao_volume;
			drawArraySize = 6*6;
			
			if (update_isovalue || (prev_vol_range != vol_range)){
			  generate_colormap(tfnc_rgba, col_pts, isovalue, vol_range, colormap_length);
			  glActiveTexture(GL_TEXTURE1);
			  glBindTexture(GL_TEXTURE_1D, tfnc);
			  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, tfnc_rgba.size()/4, 0, GL_RGBA, GL_FLOAT, &tfnc_rgba[0]);
			  prev_vol_range = vol_range;
			}
		        
			
			glUniform1i(glGetUniformLocation(shader_program, "vol"), 0); // set it manually
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_3D, texname);

			
			glUniform1i(glGetUniformLocation(shader_program, "tf"), 1);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_1D, tfnc);

		}
		// set shader to use
		glUseProgram(shader_program);

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		
		glm::vec3 cameraDirection = glm::normalize(cameraTarget - cameraPos);
		glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
		glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);
	
		view = glm::lookAt(cameraPos, cameraPos + cameraDirection, cameraUp);
		projection = glm::perspective(glm::radians(45.0f), (float)width/(float)height, 0.1f, 100.0f);

	
		MVP =  projection * view; // just let model = I 

		
		// Get a handle for our "MVP" uniform
		GLuint MatrixID = glGetUniformLocation(shader_program, "MVP");
		GLuint ModelViewMatrixID = glGetUniformLocation(shader_program, "V");
		GLuint EyePos = glGetUniformLocation(shader_program, "EyePos");
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelViewMatrixID, 1, GL_FALSE, &view[0][0]);
		glUniform3fv(EyePos, 1, glm::value_ptr(cameraPos));

		
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, drawArraySize);

		if (showMenu){
		    ImGui_ImplOpenGL3_NewFrame();
		    ImGui_ImplGlfw_NewFrame();
		    ImGui::NewFrame();
		    ImGui::Begin("Controls"); 
		    ImGui::SliderFloat("Zoom", &r, -0.0000000000000000000001f, -3);
		    ImGui::Checkbox("Volume View", &isVolumeViewPicked);
		    ImGui::Text("Isosurfaces");               // Display some text (you can use a format strings too)
      	        

		    if (ImGui::BeginCombo("##combo", items[item_selected_index])) // The second parameter is the label previewed before opening the combo.
			{
			    for (int n = 0; n < IM_ARRAYSIZE(items); n++)
				{
				    bool is_selected = (item_selected_index == n); // You can store your selection however you want, outside or inside your objects
				    if (ImGui::Selectable(items[n], is_selected)){
				        item_selected_index = n;
					isovalue = preset_isovals[n];
				    }
				    if (is_selected)
					ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
				}
			    ImGui::EndCombo();
			}
		    ImGui::SliderInt("Isovalue", &isovalue, 0, 256);
		    ImGui::SliderInt("Volume range", &vol_range, 0, 256);
		    ImGui::Text("Transfer Function");
		
		    ImDrawList* draw_list = ImGui::GetWindowDrawList();
		    const ImVec2 p = ImGui::GetCursorScreenPos();
		    const float length = 120;
		    const float line_width = length/float(colormap_length);
		    for (uint32_t i=0; i<colormap_length; i++){
			draw_list->AddLine(ImVec2(p.x + line_width*i, p.y),
					   ImVec2(p.x + line_width*i, p.y+20),
					   ImColor(ImVec4(tfnc_rgba[i*4], tfnc_rgba[i*4+1], tfnc_rgba[i*4+2],tfnc_rgba[i*4+3])),
					   line_width+5);
		    }
		    windowX = ImGui::GetWindowPos().x;
		    windowY = ImGui::GetWindowPos().y;
		    windowWidth = ImGui::GetWindowWidth();
		    windowHeight = ImGui::GetWindowHeight();

		    ImGui::End();

		    ImGui::Render();
		    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}
		
		// poll events
		glfwPollEvents();
		// Swap buffers
		glfwSwapBuffers(window);

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
	       glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vbo_surface);
	glDeleteBuffers(1, &vbo_volume);
	glDeleteProgram(shader_program_surface);
	glDeleteVertexArrays(1, &vao_surface);
	glDeleteVertexArrays(1, &vao_volume);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
