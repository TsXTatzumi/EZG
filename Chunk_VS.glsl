#version 430 core
layout(location = 0) in vec3 aPos;

uniform int size;

out vec3 orgin;

void main()
{
	orgin = aPos;

	/*density[0] = getDensity(voxel_position + corners[0]);
	density[1] = getDensity(voxel_position + corners[1]);
	density[2] = getDensity(voxel_position + corners[2]);
	density[3] = getDensity(voxel_position + corners[3]);
	density[4] = getDensity(voxel_position + corners[4]);
	density[5] = getDensity(voxel_position + corners[5]);
	density[6] = getDensity(voxel_position + corners[6]);
	density[7] = getDensity(voxel_position + corners[7]);*/

	gl_Position = vec4(aPos, 1.0);
}