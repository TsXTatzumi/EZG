#include "Chunk.h"

#include <algorithm>
#include <glm/gtx/quaternion.hpp>
#include "MarchTables.hpp"
#include "Ray.h"

bool Chunk::isInitialized = false, Chunk::isSolid;
GLuint Chunk::VAO, Chunk::VBO, Chunk::EBO, Chunk::faceCount_SSBO, Chunk::edgeList_SSBO, Chunk::ray_SSBO, Chunk::seedling_UBO, Chunk::chunksize, Chunk::Xtex, Chunk::Ytex, Chunk::Ztex;
Shader* Chunk::generateShader, * Chunk::renderShader, * Chunk::rayShader;
std::vector<glm::vec3> Chunk::voxels;
std::vector<GLuint> Chunk::indices;

void Chunk::asdf() {

	GLubyte* image;
	image = new GLubyte[4 *(chunksize + 1) * (chunksize + 1) * (chunksize + 1)];

	glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	std::cout << "Reading texels" << std::endl;
	for (int slice = 0; slice < (chunksize + 1); slice++) {
		std::cout << std::endl << slice << std::endl;
		for (int i = 0; i < (chunksize + 1); i++) {
			for (int j = 0; j < (chunksize + 1); j++) {

				int start = 4*(((chunksize + 1) * (chunksize + 1) * slice) + (i * (chunksize + 1)) + j);
				std::cout << ((image[start] < 127)) << " "; /*<< " " << (float)image[start + 1] << " " << (float)image[start + 2] << " " << (float)image[start + 3]*/
			}
			std::cout << std::endl;
		}
	}
	
	delete[] image;
}
void Chunk::Init(GLuint chunksize, Shader* generateShader, Shader* renderShader, Shader* rayShader, int seed, GLuint Xtex, GLuint Ytex, GLuint Ztex)
{
	if (!isInitialized)
	{
		Chunk::chunksize = chunksize;
		
		Chunk::generateShader = generateShader;
		Chunk::renderShader = renderShader;
		Chunk::rayShader = rayShader;

		Chunk::Xtex = Xtex;
		Chunk::Ytex = Ytex;
		Chunk::Ztex = Ztex;

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

		auto* randoms = new int[256];
		for (int i = 0; i < 256; i++)
		{
			randoms[i] = i;
		}

		srand(seed);

		for (int i = 0; i < 256; i++)
		{
			int scrambler = (unsigned int)((float)rand() / RAND_MAX * 255.9999);
			int temp = randoms[i];
			randoms[i] = randoms[scrambler];
			randoms[scrambler] = temp;
		}

		generateShader->use();
		
		glGenBuffers(1, &seedling_UBO);
		glBindBuffer(GL_UNIFORM_BUFFER, seedling_UBO);
		glBufferData(GL_UNIFORM_BUFFER, 256 * sizeof(int), &randoms, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		generateShader->setFloat("amplitude", 1.0);
		generateShader->setFloat("frequency", 0.01);
		generateShader->setFloat("persistence", 0.66);
		generateShader->setInt("octaves", 7);
		generateShader->setFloat("lacunarity", 2.0);
		generateShader->setInt("size", (int)chunksize + 1);

		renderShader->use();

		renderShader->setInt("X", 0);
		renderShader->setInt("Y", 1);
		renderShader->setInt("Z", 2);
		renderShader->setInt("size", (int)chunksize + 1);
		renderShader->setFloat("threshold", 0.5);

		glGenBuffers(1, &faceCount_SSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, faceCount_SSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 256 * sizeof(int), &CASE_TO_FACE_COUNT, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glGenBuffers(1, &edgeList_SSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, edgeList_SSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 256 * 5 * 4 * sizeof(int), &EDGE_CONNECT_LIST, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		rayShader->use();

		rayShader->setFloat("threshold", 0.5);

		glGenBuffers(1, &ray_SSBO);
		
		isInitialized = true;
	}
}

void Chunk::toggleWireframe()
{
	isSolid = !isSolid;

	renderShader->use();

	renderShader->setBool("wireframe", !isSolid);
}

void Chunk::setSteps(int steps, int finesteps)
{
	renderShader->use();

	renderShader->setInt("steps", steps);
	renderShader->setInt("fine_steps", finesteps);

	std::cout << steps << "  >  " << finesteps << std::endl;
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

	glGenTextures(1, &densityTex);
	glBindTexture(GL_TEXTURE_3D, densityTex);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, chunksize + 1, chunksize + 1, chunksize + 1, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
	
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

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, seedling_UBO);

	glBindImageTexture(0, densityTex, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R8);

	glDispatchCompute(3, 3, 11);

	// make sure writing to image has finished before read
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
}
void Chunk::render(glm::mat4 model)
{
	model = glm::translate(model, location);

	
	renderShader->use();

	renderShader->setMat4("model", model);
	renderShader->setInt("X", 0);
	renderShader->setInt("Y", 1);
	renderShader->setInt("Z", 2);
	renderShader->setInt("ssao", 3);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Xtex);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, Ytex);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, Ztex);

	glBindImageTexture(0, densityTex, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R8);
	
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, faceCount_SSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, edgeList_SSBO);
	
	glBindVertexArray(VAO);
	glDrawElements(GL_POINTS, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);
}


float Chunk::makeTea(GLubyte* image, glm::vec3 lb, int edgeID)
{
	float t = (((float)image[(int)(4 * (((chunksize + 1) * (chunksize + 1) * (lb + startCorner[edgeID]).x) + ((lb + startCorner[edgeID]).y * (chunksize + 1)) + (lb + startCorner[edgeID]).z))] / 256.0) - 0.5)
			/ (((float)image[(int)(4 * (((chunksize + 1) * (chunksize + 1) * (lb + startCorner[edgeID]).x) + ((lb + startCorner[edgeID]).y * (chunksize + 1)) + (lb + startCorner[edgeID]).z))] / 256.0)
				- (float)image[(int)(4 * (((chunksize + 1) * (chunksize + 1) * (lb + startCorner[edgeID] + edgeDir[edgeID]).x) + ((lb + startCorner[edgeID] + edgeDir[edgeID]).y * (chunksize + 1)) + (lb + startCorner[edgeID] + edgeDir[edgeID]).z))] / 256.0);
	return t;
}

bool Chunk::ray(Ray ray, glm::vec3 & pos, glm::vec3 & norm)
{
	bool found = false, inside = true;
	// ray.dir is unit direction vector of ray
	glm::vec3 dirfrac = glm::vec3(1.0f / ray.dir.x, 1.0f / ray.dir.y, 1.0f / ray.dir.z);

	glm::vec3 lb = this->location;
	glm::vec3 rt = lb + glm::vec3(chunksize);

	// lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
		// ray.m_start is origin of ray
	float t1 = (lb.x - ray.m_start.x) * dirfrac.x;
	float t2 = (rt.x - ray.m_start.x) * dirfrac.x;
	float t3 = (lb.y - ray.m_start.y) * dirfrac.y;
	float t4 = (rt.y - ray.m_start.y) * dirfrac.y;
	float t5 = (lb.z - ray.m_start.z) * dirfrac.z;
	float t6 = (rt.z - ray.m_start.z) * dirfrac.z;

	float tmin = glm::max(glm::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
	float tmax = std::min(std::min(glm::max(t1, t2), glm::max(t3, t4)), glm::max(t5, t6));

	if (tmax < 0 || tmin > tmax) inside = false;

	if (tmin < 0) lb = glm::floor(ray.m_start);
	else lb = glm::floor(ray.m_start + tmin * ray.dir);

	rt = lb + glm::vec3(1);

	int count = 0;
	float aray[96 * 4];
	while (!found && inside)
	{
		// lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
		// ray.m_start is origin of ray
		float t1 = (lb.x - ray.m_start.x) * dirfrac.x;
		float t2 = (rt.x - ray.m_start.x) * dirfrac.x;
		float t3 = (lb.y - ray.m_start.y) * dirfrac.y;
		float t4 = (rt.y - ray.m_start.y) * dirfrac.y;
		float t5 = (lb.z - ray.m_start.z) * dirfrac.z;
		float t6 = (rt.z - ray.m_start.z) * dirfrac.z;

		tmax = std::min(std::min(glm::max(t1, t2), glm::max(t3, t4)), glm::max(t5, t6));

		lb -= location;
		
		if(lb.x >= chunksize || lb.y >= chunksize || lb.z >= chunksize || lb.x < 0 || lb.y < 0 || lb.z < 0) break;

		aray[4 * count + 0] = lb.x;
		aray[4 * count + 1] = lb.y;
		aray[4 * count + 2] = lb.z;
		aray[4 * count + 3] = -1.0;

		count++;
		
		lb = glm::floor(ray.m_start + (0.01f + tmax) * ray.dir);
		rt = lb + glm::vec3(1);
	}
	
	rayShader->use();

	glm::vec3 start = ray.m_start - location;
	rayShader->setVec3("start", start);
	rayShader->setVec3("dir", ray.dir);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ray_SSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, count * 4 * sizeof(float), &aray, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ray_SSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, faceCount_SSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, edgeList_SSBO);

	glBindImageTexture(0, densityTex, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R8);

	glDispatchCompute(count, 1, 1);

	// make sure writing to image has finished before read
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	
	
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ray_SSBO);

	GLfloat* ptr;
	ptr = (GLfloat*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

	for (int i = 0; i < count; i++)
	{
		if(ptr[4 * i + 3] >= 0)
		{
			pos = ray.m_start + ray.dir * ptr[4 * i + 3];
			norm = glm::vec3(ptr[4 * i + 0], ptr[4 * i + 1], ptr[4 * i + 2]);
			found = true;
			break;
		}
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	return found;
}