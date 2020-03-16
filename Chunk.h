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

	void asdf();

	static void Init(GLuint chunksize, Shader* generateShader, Shader* renderShader, int seed);
	static void toggleWireframe();
	
	Chunk();
	~Chunk();
	
	void setupData(float x, float y);
	void relocate(float x, float y);
	void render(glm::mat4 model);
	
private:
	static GLuint VAO, VBO, EBO, faceCount_SSBO, edgeList_SSBO;
	static std::vector<glm::vec3> voxels;
	static std::vector<GLuint> indices;
	static GLuint chunksize;
	static Shader* generateShader, * renderShader;
	static bool isSolid;
	
	float * volume;
	GLuint  volume_SSBO;
};

