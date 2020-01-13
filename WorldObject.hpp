#pragma once

#include <vector>
#include "MathDefines.h"
#include "Transform.hpp"
#include "Shader.hpp"
#include "Model.hpp"

class WorldObject : public Transform
{
public:
	WorldObject();
	~WorldObject();

	void render(const Shader * shader, glm::mat4 model);
	void setModel(Model* model);

	std::vector<Point> GetVertices(GLuint id, glm::mat4 model);

private:
	Model* model;
};

