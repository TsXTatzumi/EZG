#version 430 core

in vec3 color;
in vec3 barycentric;

out vec4 FragColor;

uniform bool wireframe;

void main()
{
	if (wireframe && barycentric.x > 0.02 && barycentric.y > 0.02 && barycentric.z > 0.02) discard;
	
	FragColor = vec4(color, 1.0);
}