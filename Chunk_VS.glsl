#version 430 core
layout(location = 0) in vec3 aPos;

uniform int size;

out vec3 orgin;

void main()
{
	orgin = aPos;

	gl_Position = vec4(aPos, 1.0);
}