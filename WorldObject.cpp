#include "WorldObject.hpp"

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
