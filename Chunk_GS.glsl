#version 430 core

layout(points) in;
layout(binding = 0, r8) uniform image3D tex3D;

in vec3 orgin[];

float getDensity(ivec3 texel_position)
{
	return imageLoad(tex3D, texel_position).r;
}

layout(triangle_strip, max_vertices = 25) out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform int size;
uniform float threshold;

layout(std430, binding = 1) readonly buffer faceCount_data
{
	int[] faceCount;
};

layout(std430, binding = 2) readonly buffer edgeList_data
{
	ivec4[] edgeList;
};

ivec3 corners[] = {
	ivec3(0, 0, 0),
	ivec3(0, 1, 0),
	ivec3(1, 1, 0),
	ivec3(1, 0, 0),
	ivec3(0, 0, 1),
	ivec3(0, 1, 1),
	ivec3(1, 1, 1),
	ivec3(1, 0, 1)
};

vec3 startCorner[] = {
	vec3(0, 0, 0),
	vec3(0, 1, 0),
	vec3(1, 0, 0),
	vec3(0, 0, 0),
	vec3(0, 0, 1),
	vec3(0, 1, 1),
	vec3(1, 0, 1),
	vec3(0, 0, 1),
	vec3(0, 0, 0),
	vec3(0, 1, 0),
	vec3(1, 1, 0),
	vec3(1, 0, 0)
};

vec3 edgeDir[] = {
	vec3(0, 1, 0),
	vec3(1, 0, 0),
	vec3(0, 1, 0),
	vec3(1, 0, 0),
	vec3(0, 1, 0),
	vec3(1, 0, 0),
	vec3(0, 1, 0),
	vec3(1, 0, 0),
	vec3(0, 0, 1),
	vec3(0, 0, 1),
	vec3(0, 0, 1),
	vec3(0, 0, 1)
};

out vec3 world_normal;
out vec3 normal;
out vec3 world_position;
out vec3 v1;
out vec3 v2;
out vec3 v3;

float makeTea(uint edgeID)
{
	float t = (getDensity(ivec3(orgin[0] + startCorner[edgeID])) - threshold) / (getDensity(ivec3(orgin[0] + startCorner[edgeID])) - getDensity(ivec3(orgin[0] + startCorner[edgeID] + edgeDir[edgeID])));
	return t;
}

void PlaceVertOnEdge(uint edgeID)
{
	float t = makeTea(edgeID);

	world_position = (model * vec4(orgin[0] + startCorner[edgeID] + t * edgeDir[edgeID], 1.0)).xyz;
	gl_Position = projection * view * model * vec4(orgin[0] + startCorner[edgeID] + t * edgeDir[edgeID], 1.0);
	world_normal = edgeDir[edgeID] * sign(getDensity(ivec3(orgin[0] + startCorner[edgeID])));
	normal = (transpose(inverse(projection * view * model)) * vec4(world_normal, 1.0)).xyz;

	EmitVertex();
}

void main() 
{
	ivec3 voxel_position = ivec3(orgin[0]);

	uint mc_case = 0;
	mc_case |= ((getDensity(voxel_position + corners[0]) > threshold) ? 1 : 0) << 0;
	mc_case |= ((getDensity(voxel_position + corners[1]) > threshold) ? 1 : 0) << 1;
	mc_case |= ((getDensity(voxel_position + corners[2]) > threshold) ? 1 : 0) << 2;
	mc_case |= ((getDensity(voxel_position + corners[3]) > threshold) ? 1 : 0) << 3;
	mc_case |= ((getDensity(voxel_position + corners[4]) > threshold) ? 1 : 0) << 4;
	mc_case |= ((getDensity(voxel_position + corners[5]) > threshold) ? 1 : 0) << 5;
	mc_case |= ((getDensity(voxel_position + corners[6]) > threshold) ? 1 : 0) << 6;
	mc_case |= ((getDensity(voxel_position + corners[7]) > threshold) ? 1 : 0) << 7;

	uint numPolys = faceCount[mc_case];

	for (int i = 0; i < numPolys; ++i) {
		ivec4 edges = edgeList[5 * mc_case + i];

		v1 = (model * vec4(orgin[0] + startCorner[edges.x] + makeTea(edges.x) * edgeDir[edges.x], 1.0)).xyz;
		v2 = (model * vec4(orgin[0] + startCorner[edges.y] + makeTea(edges.y) * edgeDir[edges.y], 1.0)).xyz;
		v3 = (model * vec4(orgin[0] + startCorner[edges.z] + makeTea(edges.z) * edgeDir[edges.z], 1.0)).xyz;

		PlaceVertOnEdge(edges.x);
		PlaceVertOnEdge(edges.y);
		PlaceVertOnEdge(edges.z);

		EndPrimitive();
	}

}