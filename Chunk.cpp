#include "Chunk.h"
#include <glm/gtx/quaternion.hpp>
#include "MarchTables.hpp"

bool Chunk::isInitialized = false, Chunk::isSolid;
GLuint Chunk::VAO, Chunk::VBO, Chunk::EBO, Chunk::faceCount_SSBO, Chunk::edgeList_SSBO, Chunk::chunksize;
Shader* Chunk::generateShader, * Chunk::renderShader;
std::vector<glm::vec3> Chunk::voxels;
std::vector<GLuint> Chunk::indices;

void Chunk::asdf() {

	/*GLubyte* image;
	image = new GLubyte[(chunksize + 1) * (chunksize + 1) * (chunksize + 1)];

	glGetTexImage(GL_TEXTURE_3D, 0, GL_R8, GL_UNSIGNED_BYTE, image);*/
	std::cout << "Reading texels" << std::endl;
	for (int slice = 0; slice < (chunksize + 1); slice++) {
		std::cout << std::endl << slice << std::endl;
		for (int i = 0; i < (chunksize + 1); i++) {
			for (int j = 0; j < (chunksize + 1); j++) {

				int start = (((chunksize + 1) * (chunksize + 1) * slice) + (i * (chunksize + 1)) + j);
				std::cout << ((volume[start] > 0)) << " "; /*<< " " << (float)image[start + 1] << " " << (float)image[start + 2] << " " << (float)image[start + 3]*/
			}
			std::cout << std::endl;
		}
	}
	
	//delete[] image;
}

void Chunk::Init(GLuint chunksize, Shader* generateShader, Shader* renderShader, int seed)
{
	if (!isInitialized)
	{
		Chunk::chunksize = chunksize;
		
		Chunk::generateShader = generateShader;
		Chunk::renderShader = renderShader;

		for (int x = 0; x < chunksize; ++x)
		{
			for (int y = 0; y < chunksize; ++y)
			{
				for (int z = 0; z < chunksize; ++z)
				{
					voxels.push_back({ x, y, z });
					indices.push_back(indices.size());
				}
			}
		}

		auto* randoms = new unsigned int[256];
		for (int i = 0; i < 256; i++)
		{
			randoms[i] = i;
		}

		srand(seed);

		for (int i = 0; i < 256; i++)
		{
			unsigned int scrambler = (unsigned int)((float)rand() / RAND_MAX * 255.9999);
			unsigned int temp = randoms[i];
			randoms[i] = randoms[scrambler];
			randoms[scrambler] = temp;
		}
		
		generateShader->use();
		
		generateShader->setInts("hash", (int*)randoms, 256);
		generateShader->setFloat("amplitude", 1.0);
		generateShader->setFloat("frequency", 0.1);
		generateShader->setFloat("persistence", 0.4);
		generateShader->setInt("octaves", 3);
		generateShader->setFloat("lacunarity", 3.0);
		generateShader->setInt("size", (int)chunksize + 1);

		renderShader->use();

		renderShader->setInt("size", (int)chunksize + 1);

		glGenBuffers(1, &faceCount_SSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, faceCount_SSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 256 * sizeof(int), &CASE_TO_FACE_COUNT, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glGenBuffers(1, &edgeList_SSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, edgeList_SSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 256 * 5 * 4 * sizeof(int), &EDGE_CONNECT_LIST, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		isInitialized = true;
	}
}

void Chunk::toggleWireframe()
{
	isSolid = !isSolid;

	renderShader->use();

	renderShader->setBool("wireframe", !isSolid);
}

Chunk::Chunk()
{
	
}


Chunk::~Chunk()
{
	
}

void Chunk::setupData(float x, float y)
{
	if (!isInitialized)
	{
		throw std::runtime_error("You must call Chunk::Init(chunksize) first!");
	}

	volume = new float[(chunksize + 1) * (chunksize + 1) * (chunksize + 1)];

	glGenBuffers(1, &volume_SSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, volume_SSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, (chunksize + 1) * (chunksize + 1) * (chunksize + 1) * sizeof(float), volume, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	
	relocate(x, y);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, voxels.size() * sizeof(glm::vec3), &voxels[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);

	glBindVertexArray(0);	
}

void Chunk::relocate(float x, float y)
{
	location = glm::vec3(x, y, 0);

	generateShader->use();

	generateShader->setVec3("chunk_position", location);
	
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, volume_SSBO);

	glDispatchCompute(3, 3, 11);
	// make sure writing to image has finished before read
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, volume_SSBO);
	GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
	memcpy(volume, p, (chunksize + 1) * (chunksize + 1) * (chunksize + 1) * sizeof(float));
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void Chunk::render(glm::mat4 model)
{
	model = glm::translate(model, location);

	renderShader->setMat4("model", model);

	renderShader->use();

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, volume_SSBO);
	GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
	memcpy(p, volume, (chunksize + 1) * (chunksize + 1) * (chunksize + 1) * sizeof(float));
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, volume_SSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, faceCount_SSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, edgeList_SSBO);
	
	glBindVertexArray(VAO);
	glDrawElements(GL_POINTS, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
}