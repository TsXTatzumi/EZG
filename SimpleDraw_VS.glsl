#version 330 core

layout(location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

uniform vec3 color;

out VS_OUT {
    vec3 color;
} vs_out;


void main()
{
	vs_out.color = color;
	gl_Position = projection * view  * vec4(aPos, 1.0);
	
}