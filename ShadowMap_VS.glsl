#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

out vec2 TexCoords;

out vec3 view_normal;
out vec3 view_position;

out VS_OUT {
    vec3 FragPos;
	mat3 TBN;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;

uniform float bumpiness;

void main()
{
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));   
    vs_out.TexCoords = aTexCoords;
    
    mat3 normalMatrix = inverse(mat3(model));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    
    mat3 TBN = mat3(T * bumpiness, B * bumpiness, N);    

    vs_out.TBN = TBN;

	vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);

	view_normal = (transpose(inverse(view * model)) * vec4(aNormal, 1.0)).xyz;

    view_position = (view * model * vec4(aPos, 1.0)).xyz;

	gl_Position = projection * vec4(view_position, 1.0);
}