#version 430 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aVel;
layout(location = 2) in vec4 aColor;
layout(location = 3) in float aLifetime;
layout(location = 4) in int aType;
layout(location = 5) in int aSeed;

uniform mat4 projection;
uniform mat4 view;

out vec4 orgin;
out vec4 ParticleColor;
out int seed;
out float life;

void main()
{
	ParticleColor = aColor;
	seed = aSeed;
	life = aLifetime;

	orgin = vec4(aPos, 1.0);
}