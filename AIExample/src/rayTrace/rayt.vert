#version 330

uniform mat4 MVP;
uniform mat4 MV;
uniform vec3 EyePos;

in vec3 vp;
out vec3 eye_to_v;
out vec3 v_pos;

void main() {
     
  gl_Position = MVP * vec4(vp*vec3(1,0.94,0.272), 1);

  eye_to_v.x = vp.x - EyePos.x;
  eye_to_v.y = vp.y - EyePos.y/0.94;
  eye_to_v.z = vp.z - EyePos.z/0.272;

  v_pos = vp;
};
