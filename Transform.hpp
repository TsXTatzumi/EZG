#pragma once
#include <glm/gtx/quaternion.hpp>


class Transform
{
public:
	glm::vec3 location;
	glm::quat rotation = { 1, 0, 0, 0 };
	glm::vec3 scale = { 1, 1, 1 };

	Transform();
	~Transform();

	glm::vec3 Forward();

	glm::vec3 Left();

	glm::vec3 Up();
};
