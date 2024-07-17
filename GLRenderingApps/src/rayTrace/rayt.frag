#version 330

uniform mat4 MVP;
uniform mat4 MV;
uniform vec3 EyePos;

uniform sampler3D vol;
uniform sampler1D tf;


in vec3 eye_to_v;
in vec3 v_pos;

out vec4 color;

void main(){
     vec3 r_dir = normalize(eye_to_v);
     r_dir.y = r_dir.y;
     r_dir.z = r_dir.z;
     
     vec3 r_pos = vec3 (0, 0, 0);

     float val = 0;
     vec3 r_start = v_pos;

     color = vec4(0,0,0,0);
     
     for(int i=0;i<300;i++)
     {
	if (r_start.x > 0.501) break;
	if (r_start.x < -0.501) break;
	if (r_start.y > 0.501) break;
	if (r_start.y < -0.501) break;
	if (r_start.z > 0.501) break;
	if (r_start.z < -0.501) break;

	float val = texture(vol, r_start+0.5).r;	
	vec4 val_color = texture(tf, val).rgba;
	//vec4 val_color = vec4(1,1,1,0.1) * val;


	color.rgb += (1.0 - color.a) * val_color.a * val_color.rgb;
	color.a += (1.0 - color.a) * val_color.a;

	r_start += r_dir*0.01;
     }

     ///frag_colour = vec4(texture(vol, r_pos).r,texture(vol, r_pos).r,texture(vol, r_pos).r , 0.4);
     //frag_colour = vec4(eye_to_v*2 , 0.4);
     //color = texture(tf, v_pos.y+0.5).rgba;
     //color = vec4(v_pos.y + 0.5, v_pos.y+0.5, v_pos.y+0.5, 1);
}
