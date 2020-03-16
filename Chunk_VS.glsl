#version 430 core
layout (location = 0) in vec3 aPos;

out vec3 orgin;
out uint mc_case; // 0-255

uniform int size;

layout(std430, binding = 0) readonly buffer shader_data
{
	float[] volume;
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

float getDensity(ivec3 texel_position)
{
	return volume[(texel_position.x * size + texel_position.y) * size + texel_position.z];
}

void main()
{
	ivec3 voxel_position = ivec3(aPos);
	
	orgin = aPos;

	/*density[0] = getDensity(voxel_position + corners[0]);
	density[1] = getDensity(voxel_position + corners[1]);
	density[2] = getDensity(voxel_position + corners[2]);
	density[3] = getDensity(voxel_position + corners[3]);
	density[4] = getDensity(voxel_position + corners[4]);
	density[5] = getDensity(voxel_position + corners[5]);
	density[6] = getDensity(voxel_position + corners[6]);
	density[7] = getDensity(voxel_position + corners[7]);*/

	mc_case = 0;
	mc_case |= ((getDensity(voxel_position + corners[0]) > 0) ? 1 : 0) << 0;
	mc_case |= ((getDensity(voxel_position + corners[1]) > 0) ? 1 : 0) << 1;
	mc_case |= ((getDensity(voxel_position + corners[2]) > 0) ? 1 : 0) << 2;
	mc_case |= ((getDensity(voxel_position + corners[3]) > 0) ? 1 : 0) << 3;
	mc_case |= ((getDensity(voxel_position + corners[4]) > 0) ? 1 : 0) << 4;
	mc_case |= ((getDensity(voxel_position + corners[5]) > 0) ? 1 : 0) << 5;
	mc_case |= ((getDensity(voxel_position + corners[6]) > 0) ? 1 : 0) << 6;
	mc_case |= ((getDensity(voxel_position + corners[7]) > 0) ? 1 : 0) << 7;
	
	gl_Position = vec4(aPos, 1.0);
}