#version 430 core

layout(points) in;

layout(triangle_strip, max_vertices = 3) out;

uniform mat4 projection;
uniform mat4 view;

in vec4 orgin[];
in vec4 ParticleColor[];
in int seed[];
in float life[];

out vec4 color;

void main()
{
	color = ParticleColor[0];

	gl_Position = projection * view * (orgin[0] + normalize(vec4(sin(seed[0] * life[0]), sin(seed[0] * 2 * life[0]), sin(seed[0] * seed[0] * life[0]), 0)) * 0.05);
	EmitVertex();

	gl_Position = projection * view * (orgin[0] + normalize(vec4(sin(seed[0] * 2 * life[0]), sin(seed[0] * seed[0] * life[0]), sin(seed[0] * life[0]), 0)) * 0.05);
	EmitVertex();

	gl_Position = projection * view * (orgin[0] + normalize(vec4(sin(seed[0] * seed[0] * life[0]), sin(seed[0] * life[0]), sin(seed[0] * 2 * life[0]), 0)) * 0.05);
	EmitVertex();

	EndPrimitive();
}