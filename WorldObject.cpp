#include "WorldObject.hpp"
#include <iostream>

WorldObject::WorldObject()
{
}

WorldObject::~WorldObject()
{
}

void WorldObject::render(const Shader * shader, glm::mat4 model)
{

	model = glm::translate(model, location);
	model = glm::rotate(model, angle(rotation), axis(rotation));
	model = glm::scale(model, scale);

	shader->setMat4("model", model);

	this->model->render(shader);
}

void WorldObject::setModel(Model * model)
{
	this->model = model;
}

std::vector<Point> WorldObject::GetVertices(GLuint id, glm::mat4 model)
{
	std::vector<Point> verts = std::vector<Point>();

	model = glm::translate(model, location);
	model = glm::rotate(model, angle(rotation), axis(rotation));
	model = glm::scale(model,  scale);

	for (int i = 0; i < this->model->indices.size(); i++) {
		Vertex vertex = this->model->vertices[this->model->indices[i]];

		glm::vec4 pos = glm::vec4(vertex.Position, 1);
		//std::cout << "x: " << pos.x << "\n";
		//std::cout << "y: " << pos.y << "\n";
		//std::cout << "z: " << pos.z << "\n";
		//std::cout << "w: " << pos.w << "\n";

		glm::vec3 vert3 = model * pos;

		//std::cout << "x: " << vert3.x << "\n";
		//std::cout << "y: " << vert3.y << "\n";
		//std::cout << "z: " << vert3.z << "\n";

		Point p;
		p.x = vert3.x;
		p.y = vert3.y;
		p.z = vert3.z;


		p.objID = id;

		verts.push_back(p);
	}


	return verts;
}
