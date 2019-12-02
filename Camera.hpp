#pragma once
#include "Transform.hpp"

class Camera : public Transform
{
public:
	float roll;

	Camera();
	Camera(glm::vec3 location, glm::vec3 rotation);
	~Camera();

	glm::vec3 Forward();

	glm::vec3 Left();

	glm::vec3 Up();

	glm::quat CorrectRotation();

	glm::mat4 GetViewMatrix();
};

