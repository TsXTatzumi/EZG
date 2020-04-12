#version 430
layout(local_size_x = 11, local_size_y = 11, local_size_z = 3) in;
layout(binding = 0, r16f) uniform writeonly image3D tex3D;


#define TETRAHEDRA_TO_CUBES  0.33333333333  // 1 / 3
#define CUBES_TO_TETRAHEDRA  0.16666666667  // 1 / 6

#define G_MASK_3D 31
vec3[32] gradients3D = vec3[](
	normalize(vec3(1f, 1f, 0f)),
	normalize(vec3(-1f, 1f, 0f)),
	normalize(vec3(1f, -1f, 0f)),
	normalize(vec3(-1f, -1f, 0f)),
	normalize(vec3(1f, 0f, 1f)),
	normalize(vec3(-1f, 0f, 1f)),
	normalize(vec3(1f, 0f, -1f)),
	normalize(vec3(-1f, 0f, -1f)),
	normalize(vec3(0f, 1f, 1f)),
	normalize(vec3(0f, -1f, 1f)),
	normalize(vec3(0f, 1f, -1f)),
	normalize(vec3(0f, -1f, -1f)),

	normalize(vec3(1f, 1f, 0f)),
	normalize(vec3(-1f, 1f, 0f)),
	normalize(vec3(1f, -1f, 0f)),
	normalize(vec3(-1f, -1f, 0f)),
	normalize(vec3(1f, 0f, 1f)),
	normalize(vec3(-1f, 0f, 1f)),
	normalize(vec3(1f, 0f, -1f)),
	normalize(vec3(-1f, 0f, -1f)),
	normalize(vec3(0f, 1f, 1f)),
	normalize(vec3(0f, -1f, 1f)),
	normalize(vec3(0f, 1f, -1f)),
	normalize(vec3(0f, -1f, -1f)),

	normalize(vec3(1f, 1f, 1f)),
	normalize(vec3(-1f, 1f, 1f)),
	normalize(vec3(1f, -1f, 1f)),
	normalize(vec3(-1f, -1f, 1f)),
	normalize(vec3(1f, 1f, -1f)),
	normalize(vec3(-1f, 1f, -1f)),
	normalize(vec3(1f, -1f, -1f)),
	normalize(vec3(-1f, -1f, -1f))
	);

#define SIMPLEX_SCALE_3D 40.78957241 // 8192f * sqrt(3) / 375 / 0.9276201

#define MASK 255
layout(std430, binding = 0) readonly buffer seedling_data
{
	int[256] hash;
};

struct Noise3D
{
	float value;
	vec3 derivative;
};

Noise3D simplexFalloffPoivec3D(vec3 position, ivec3 iposition)
{
	float unskew = (iposition.x + iposition.y + iposition.z) * CUBES_TO_TETRAHEDRA;
	vec3 distance = position - iposition + unskew;

	float falloffD2 = 0.5 - distance.x * distance.x - distance.y * distance.y - distance.z * distance.z;

	float falloffD1 = falloffD2 * falloffD2;
	float falloff = falloffD2 * falloffD1;

	vec3 grad = gradients3D[hash[hash[hash[iposition.x & MASK] + iposition.y & MASK] + iposition.z & MASK] % G_MASK_3D];

	float dotproduct = dot(grad, distance);

	Noise3D noise;
	noise.value = (falloffD2 < 0) ? 0 : falloff * dotproduct * SIMPLEX_SCALE_3D;
	noise.derivative = (falloffD2 < 0) ? vec3(0) : grad * falloff - 6 * distance * falloffD1 * dotproduct * SIMPLEX_SCALE_3D;

	return noise;

}

Noise3D simplex3D(vec3 position, float frequency)
{
	position *= frequency;

	float skew = (position.x + position.y + position.z) * TETRAHEDRA_TO_CUBES;

	vec3 sposition = position + skew;
	ivec3 iposition_0 = ivec3(sposition);
	ivec3 iposition_1 = iposition_0 + 1;
	ivec3 iposition_2;
	ivec3 iposition_3;

	if (sposition.y - iposition_0.y < sposition.x - iposition_0.x)
	{
		if (sposition.z - iposition_0.z < sposition.x - iposition_0.x)
		{
			iposition_2 = iposition_0 + ivec3(1, 0, 0);
			if (sposition.z - iposition_0.z < sposition.y - iposition_0.y)
			{
				iposition_3 = iposition_0 + ivec3(1, 1, 0);
			}
			else
			{
				iposition_3 = iposition_0 + ivec3(1, 0, 1);
			}
		}
		else
		{
			iposition_2 = iposition_0 + ivec3(0, 0, 1);
			iposition_3 = iposition_0 + ivec3(1, 0, 1);
		}
	}
	else
	{
		if (sposition.z - iposition_0.z < sposition.y - iposition_0.y)
		{
			iposition_2 = iposition_0 + ivec3(0, 1, 0);
			if (sposition.z - iposition_0.z < sposition.x - iposition_0.x)
			{
				iposition_3 = iposition_0 + ivec3(1, 1, 0);
			}
			else
			{
				iposition_3 = iposition_0 + ivec3(0, 1, 1);
			}
		}
		else
		{
			iposition_2 = iposition_0 + ivec3(0, 0, 1);
			iposition_3 = iposition_0 + ivec3(0, 1, 1);
		}
	}

	Noise3D noise;
	Noise3D noise_0 = simplexFalloffPoivec3D(position, iposition_0);
	Noise3D noise_1 = simplexFalloffPoivec3D(position, iposition_1);
	Noise3D noise_2 = simplexFalloffPoivec3D(position, iposition_2);
	Noise3D noise_3 = simplexFalloffPoivec3D(position, iposition_3);

	noise.value = (noise_0.value + noise_1.value + noise_2.value + noise_3.value); // * 8; // / MASK;
	noise.derivative = (noise_0.derivative + noise_1.derivative + noise_2.derivative + noise_3.derivative) * 8 / MASK;

	noise.derivative *= frequency;

	return noise;
}

uniform float amplitude;
uniform float frequency;
uniform float persistence;
uniform float lacunarity;
uniform int   octaves;



Noise3D Fractal(vec3 position)
{
	Noise3D sum = simplex3D(position, frequency);
	float current_amplitude = 1;
	float range = current_amplitude;
	float frequency_fade = frequency;
	for (int o = 1; o < octaves; o++)
	{
		frequency_fade *= lacunarity;
		current_amplitude *= persistence;
		range += current_amplitude;
		Noise3D noise = simplex3D(position, frequency_fade);
		sum.value += noise.value * current_amplitude;
		sum.derivative += noise.derivative * current_amplitude;
	}

	sum.value *= amplitude / range;
	sum.derivative *= amplitude / range;

	return sum;
}

uniform int size;
uniform vec3 chunk_position;

void main()
{
	ivec3 inChunk_position = ivec3(gl_GlobalInvocationID);

	ivec3 texel_position = ivec3((inChunk_position.x + chunk_position.x), (inChunk_position.y + chunk_position.y), (inChunk_position.z + chunk_position.z));

	float value = Fractal(texel_position).value + 1;

	float r = sqrt((texel_position - size / 2).y * (texel_position - size / 2).y + (texel_position - size / 2).z * (texel_position - size / 2).z);

	imageStore(tex3D, inChunk_position, vec4(value/2 - r/size/2, 0, 0, 1));
}
