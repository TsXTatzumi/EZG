#pragma 

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <sstream>
#include <vector>
#include "Shader.hpp"

class Chunk
{
public:
	static bool isInitialized;
	
	glm::vec3 location;

	static void Init(GLuint chunksize, Shader* generateShader, Shader* renderShader);
	
	Chunk();
	~Chunk();
	
	void setupData();
	void render(glm::mat4 model);

private:
	static GLuint VAO, VBO;
	static std::vector<glm::vec3> voxels;
	static GLuint chunksize;
	static Shader* generateShader, * renderShader;
	
	GLuint densityTex;
};

