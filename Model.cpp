#include "Model.hpp"

Model::Model(std::vector<Vertex> vertices, std::vector<GLuint> indices, GLuint texture, GLuint normal_map)
{
	this->vertices = vertices;
	this->indices = indices;
	this->texture = texture;
	this->normal_map = normal_map;

	// Now that we have all the required data, set the vertex buffers and its attribute pointers.
	this->setupMesh();
}


Model::~Model()
{
}

void Model::setupMesh()
{
	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &this->VBO);
	glGenBuffers(1, &this->EBO);

	glBindVertexArray(this->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);

	glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);

	/*for(size_t i = 0; i < indices.size(); i += 3)
	{
		glm::vec3 edge1 = vertices[indices[i+1]].Position - vertices[indices[i]].Position;
		glm::vec3 edge2 = vertices[indices[i+2]].Position - vertices[indices[i]].Position;
		
		glm::vec2 deltaUV1 = vertices[indices[i+1]].TexCoords - vertices[indices[i]].TexCoords;
		glm::vec2 deltaUV2 = vertices[indices[i+2]].TexCoords - vertices[indices[i]].TexCoords;

		GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		vertices[indices[i]].Tangent = glm::normalize(glm::vec3(f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
																  f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
																  f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z)));

		vertices[indices[i+1]].Tangent = vertices[indices[i]].Tangent;
		vertices[indices[i+2]].Tangent = vertices[indices[i]].Tangent;
	}*/
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)0);
	
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, Normal));
	
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)offsetof(Vertex, TexCoords));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Tangent));

	glBindVertexArray(0);
}

void Model::render(const Shader * shader)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normal_map);

	// Draw mesh
	glBindVertexArray(this->VAO);
	glDrawElements(modelType, this->indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Model::setModelType(GLbitfield type)
{
	this->modelType = type;
}
