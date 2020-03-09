﻿#version 430

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(binding = 0, r32f) writeonly uniform image3D tex3D;

void main()
{
	imageStore(tex3D, ivec3(gl_GlobalInvocationID.xyz), vec4(1.0, 1.0, 1.0, 1.0));
}