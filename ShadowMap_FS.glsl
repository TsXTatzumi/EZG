#version 330 core
out vec4 FragColor;

in VS_OUT{
	vec3 FragPos;
	mat3 TBN;
	vec2 TexCoords;
	vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D normalMap;  

uniform sampler2D shadowMap;

uniform float expConst;

uniform vec3 lightDir;
uniform vec3 viewPos;

float ShadowCalculation()
{
	vec3 projCoords = fs_in.FragPosLightSpace.xyz / fs_in.FragPosLightSpace.w;

	projCoords = projCoords * 0.5 + 0.5;


	vec3 normal = texture(normalMap, fs_in.TexCoords).rgb;
	normal = normalize(normal * 2.0 - 1.0);
	normal = normalize(fs_in.TBN * normal);

	float bias = max(0.05 * (1.0 - dot(normal, -lightDir)), 0.005);

	float occluder = exp(expConst*texture(shadowMap, projCoords.xy).r);
	float receiver =  exp(-expConst * (projCoords.z - bias));
	float shadow = clamp(1 - occluder * receiver, 0.0, 1.0);

	//float shadow = (currentDepth - bias > closestDepth) ? 1.0 : 0.0;

	return shadow;
}

void main()
{
	vec3 lightDir = normalize(lightDir);

	vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
	vec3 normal = texture(normalMap, fs_in.TexCoords).rgb;
		 normal = normalize(normal * 2.0 - 1.0);   
		 normal = normalize(fs_in.TBN * normal); 
	vec3 lightColor = vec3(0.5);
	// ambient
	vec3 ambient = vec3(0.2);
	// diffuse
	float diff = max(-dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * lightColor;
	// specular
	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 reflectDir = reflect(lightDir, normal);
	float spec = pow(max(-dot(viewDir, reflectDir), 0.0), 32.0);
	vec3 specular = spec * lightColor;
	// calculate shadow
	float shadow = ShadowCalculation();
	vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

	FragColor = vec4(lighting, 1.0);
}