#version 430 core

layout(points) in;

in vec3 orgin[];
in uint mc_case[]; // 0-255

layout(triangle_strip, max_vertices = 25) out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform int size;

layout(std430, binding = 0) readonly buffer shader_data
{
	float[] volume;
};

float getDensity(ivec4 texel_position)
{
	return volume[(texel_position.x * size + texel_position.y) * size + texel_position.z];
}

layout(std430, binding = 1) readonly buffer faceCount_data
{
	int[] faceCount;
};

layout(std430, binding = 2) readonly buffer edgeList_data
{
	ivec4[] edgeList;
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

out vec3 color;
out vec3 barycentric;

void PlaceVertOnEdge(uint edgeID)
{
	float t = 0.5; //VS2GS[0].density[cornerID1] / (VS2GS[0].density[cornerID1] - VS2GS[0].density[cornerID2]);

	gl_Position = projection * view * model * vec4(orgin[0] + startCorner[edgeID] + t * edgeDir[edgeID], 1.0);
	//normal = (corners[cornerID2] - corners[cornerID1]) * sign(VS2GS[0].density[cornerID1]);
	if (edgeID == 9) color = vec3(0.9, 0.9, 0);
	else color = vec3(float(edgeID+1)/11 );

	EmitVertex();
}
void main() {
	uint numPolys = faceCount[mc_case[0]];

	for (int i = 0; i < numPolys; ++i) {
		ivec4 edges = edgeList[5 * mc_case[0] + i];

		color = vec3(float(i) / 5);
		barycentric = vec3(1, 0, 0);
		PlaceVertOnEdge(edges.x);
		barycentric = vec3(0, 1, 0);
		PlaceVertOnEdge(edges.y);
		barycentric = vec3(0, 0, 1);
		PlaceVertOnEdge(edges.z);

		EndPrimitive();
	}

}