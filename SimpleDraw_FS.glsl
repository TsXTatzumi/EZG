#version 330 core
out vec4 FragColor;

void main()
{
	//gl_FragDepth = gl_FragCoord.z;
	FragColor = vec4(0.8,1,0,1);
}