#include "Chunk.h"

bool Chunk::isInitialized = false;
GLuint Chunk::VAO, Chunk::VBO, Chunk::chunksize;
Shader* Chunk::generateShader, * Chunk::renderShader;
std::vector<glm::vec3> Chunk::voxels;

void Chunk::Init(GLuint chunksize, Shader* generateShader, Shader* renderShader)
{
	if (!isInitialized)
	{
		Chunk::chunksize = chunksize;
		
		Chunk::generateShader = generateShader;
		Chunk::renderShader = renderShader;

		for (int x = 0; x < chunksize + 1; ++x)
		{
			for (int y = 0; y < chunksize + 1; ++y)
			{
				for (int z = 0; z < chunksize + 1; ++z)
				{
					voxels.push_back({ x, y, z });
				}
			}
		}

		isInitialized = true;
	}
}

Chunk::Chunk()
{
	
}


Chunk::~Chunk()
{
	
}

void Chunk::setupData()
{
	if (!isInitialized)
	{
		throw std::runtime_error("You must call Chunk::Init(chunksize) first!");
	}
	glEnable(GL_TEXTURE_3D);
	glGenTextures(1, &densityTex);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, densityTex);
	
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, chunksize + 1, chunksize + 1, chunksize + 1, 0, GL_RED, GL_FLOAT, NULL);
	glBindImageTexture(0, densityTex, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);

	
	glDispatchCompute(chunksize + 1, chunksize + 1, chunksize + 1);
	// make sure writing to image has finished before read
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	glBindTexture(GL_TEXTURE_3D, 0);


	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, this->voxels.size() * sizeof(glm::vec3), &this->voxels[0], GL_STATIC_DRAW);

}

void Chunk::render(glm::mat4 model)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, densityTex);

	glBindBuffer(GL_ARRAY_BUFFER, VAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_POINTS, 0, 1);

	glDisableVertexAttribArray(0); 
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
