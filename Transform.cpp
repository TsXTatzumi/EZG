#include "Transform.hpp"


Transform::Transform()
{
}


Transform::~Transform()
{
}


glm::vec3 Transform::Forward()
{
	return glm::rotate(rotation, { 0, 0, -1 });
}


glm::vec3 Transform::Left()
{
	return glm::rotate(rotation, { -1, 0, 0 });
}


glm::vec3 Transform::Up()
{
	return glm::rotate(rotation, { 0, 1, 0 });
}

