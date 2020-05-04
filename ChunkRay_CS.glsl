#version 430
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(binding = 0, r8) uniform readonly image3D tex3D;

float getDensity(ivec3 texel_position)
{
	return imageLoad(tex3D, texel_position).r;
}

uniform float threshold;
uniform vec3 start;
uniform vec3 dir;

layout(std430, binding = 0) buffer ray_data
{
	vec4[] ray;
};

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


float makeTea(uint edgeID)
{
	float t = (getDensity(ivec3(ray[gl_GlobalInvocationID.x].xyz + startCorner[edgeID])) - threshold) / (getDensity(ivec3(ray[gl_GlobalInvocationID.x].xyz + startCorner[edgeID])) - getDensity(ivec3(ray[gl_GlobalInvocationID.x].xyz + startCorner[edgeID] + edgeDir[edgeID])));
	return t;
}

/* Ray-triangle intersection routine https://github.com/johnnovak/raytriangle-test/blob/master/cpp/perftest.cpp */

float rayTriangleIntersect(vec3 v0, vec3 v1, vec3 v2)
{
	vec3 v0v1 = v1 - v0;
	vec3 v0v2 = v2 - v0;

	vec3 pvec = cross(dir, v0v2);

	float det = dot(v0v1, pvec);

	if (det < 0.000001)
		return -1;

	float invDet = 1.0 / det;

	vec3 tvec = start - v0;

	float u = dot(tvec, pvec) * invDet;

	if (u < 0 || u > 1)
		return -1;

	vec3 qvec = cross(tvec, v0v1);

	float v = dot(dir, qvec) * invDet;

	if (v < 0 || u + v > 1)
		return -1;

	return dot(v0v2, qvec) * invDet;
}

void main() 
{
	ivec3 voxel_position = ivec3(ray[gl_GlobalInvocationID.x].xyz);

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
		
		vec3 v0 = vec4(ray[gl_GlobalInvocationID.x].xyz + startCorner[edges.x] + makeTea(edges.x) * edgeDir[edges.x], 1.0).xyz;
		vec3 v1 = vec4(ray[gl_GlobalInvocationID.x].xyz + startCorner[edges.y] + makeTea(edges.y) * edgeDir[edges.y], 1.0).xyz;
		vec3 v2 = vec4(ray[gl_GlobalInvocationID.x].xyz + startCorner[edges.z] + makeTea(edges.z) * edgeDir[edges.z], 1.0).xyz;

		if(rayTriangleIntersect(v0, v1, v2) >= 0) ray[gl_GlobalInvocationID.x] = vec4(-normalize(cross(v0 - v1, v2 - v1)), rayTriangleIntersect(v0, v1, v2));
	}
}