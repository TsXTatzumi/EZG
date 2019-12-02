#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <sstream>
#include <vector>
#include "Shader.hpp"

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tangent;
};

class Model
{
public:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	GLuint texture;
	GLuint normal_map;
	
	Model(std::vector<Vertex> vertices, std::vector<GLuint> indices, GLuint texture, GLuint normal_map);
	~Model();

	void render(const Shader * shader);

	void setModelType(GLbitfield type);

private:
	/*  Render data  */
	GLuint VAO, VBO, EBO;
	GLbitfield modelType = GL_TRIANGLES;

	void setupMesh();
};

