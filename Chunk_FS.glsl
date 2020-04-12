#version 430 core

in vec3 world_normal;
in vec3 normal;
in vec3 world_position;
in vec3 v1;
in vec3 v2;
in vec3 v3;


out vec4 FragColor;

uniform sampler2D X;
uniform sampler2D Y;
uniform sampler2D Z;

uniform vec3 camera_position;
uniform int steps;
uniform int fine_steps;	

uniform bool wireframe;

vec3 surface(vec3 UVW)
{
	return (texture(X, UVW.yz).xyz * abs(dot(world_normal, vec3(1, 0, 0))) + texture(Y, UVW.xz).xyz * abs(dot(world_normal, vec3(0, 1, 0))) + texture(Z, UVW.xy).xyz * abs(dot(world_normal, vec3(0, 0, 1)))) / (abs(dot(world_normal, vec3(1, 0, 0))) + abs(dot(world_normal, vec3(0, 1, 0))) + abs(dot(world_normal, vec3(0, 0, 1))));
}

float calcDepth(vec3 UVW) {
	return ((1 - surface(UVW).y) + (1 - surface(UVW + vec3(0.001, 0, 0)).y) + (1 - surface(UVW + vec3(-0.001, 0, 0)).y)
								 + (1 - surface(UVW + vec3(0, 0.001, 0)).y) + (1 - surface(UVW + vec3(0, -0.001, 0)).x)
								 + (1 - surface(UVW + vec3(0, 0, 0.001)).x) + (1 - surface(UVW + vec3(0, 0, -0.001)).x)) / 7;
}

vec3 world_tri_normal;

vec3 projectToTriangle(vec3 v)
{
	vec3 vec = v - world_position;

	return v - dot(vec, world_tri_normal) * world_tri_normal;
}

vec3 calcBarycentric(vec3 p)
{
	vec3 v12 = v2 - v1;
	vec3 v13 = v3 - v1;
	vec3 v1p = p - v1;
	
	float d00 = dot(v12, v12);
	float d01 = dot(v12, v13);
	float d11 = dot(v13, v13);
	float d20 = dot(v1p, v12);
	float d21 = dot(v1p, v13);
	
	float denom = d00 * d11 - d01 * d01;
	
	float v = (d11 * d20 - d01 * d21) / denom;
	float w = (d00 * d21 - d01 * d20) / denom;

	return vec3(1.0f - v - w, v, w);
}

void main()
{
	world_tri_normal = normalize(cross(v1 - v2, v3 - v2));

	vec3 eyeDir = (world_position - camera_position) / dot(world_position - camera_position, world_tri_normal);
	float hit_depth = 0;
	float prev_hits = 0;
	for (int step = 1; step <= steps; ++step)
	{
		float depth = float(step) / steps;
		vec3 UVW = projectToTriangle(world_position + 0.1 * depth * eyeDir);
		float is_first_hit = ((depth - calcDepth(UVW) - prev_hits) > 0)? 1 : 0;
		hit_depth += is_first_hit * depth;
		prev_hits += is_first_hit;
	}

	float base_depth = hit_depth;
	hit_depth = 0;
	prev_hits = 0;
	for (int step = 1; step <= fine_steps; ++step)
	{
		float depth = base_depth - float(step) / steps / fine_steps;
		vec3 UVW = projectToTriangle(world_position + 0.1 * depth * eyeDir);
		float is_first_hit = ((depth - calcDepth(UVW) + prev_hits) < 0) ? 1 : 0;
		hit_depth += is_first_hit * depth;
		prev_hits += is_first_hit;
	}

	vec3 UVW = projectToTriangle(world_position + 0.1 * hit_depth * eyeDir);

	vec3 color = surface(UVW);
	vec3 barycentric = calcBarycentric(UVW);
	if (wireframe && barycentric.x > 0.02 && barycentric.y > 0.02 && barycentric.z > 0.02) discard;
	if (barycentric.x < -0.0 || barycentric.y < -0.0 || barycentric.z < -0.0) discard;
	
	FragColor = vec4(color, 1.0);
}