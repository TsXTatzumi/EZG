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

	static void Init(GLuint chunksize, Shader* generateShader, Shader* renderShader, int seed, GLuint Xtex, GLuint Ytex, GLuint Ztex);
	static void toggleWireframe();
	static void setSteps(int steps, int finesteps);
	
	Chunk();
	~Chunk();
	
	void setupData(float x, float y);
	void relocate(float x, float y);
	void render(glm::mat4 model);
	
private:
	static GLuint VAO, VBO, EBO, faceCount_SSBO, edgeList_SSBO, seedling_UBO, Xtex, Ytex, Ztex;
	static std::vector<glm::vec3> voxels;
	static std::vector<GLuint> indices;
	static GLuint chunksize;
	static Shader* generateShader, * renderShader;
	static bool isSolid;
	
	GLuint densityTex;
};

