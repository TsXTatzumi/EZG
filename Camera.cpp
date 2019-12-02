#include "Camera.hpp"



Camera::Camera()
{
}

Camera::Camera(glm::vec3 location, glm::vec3 rotation)
{
	this->location = location;
	this->rotation = glm::quat(rotation);
}


Camera::~Camera()
{
}

glm::vec3 Camera::Forward()
{
	return glm::rotate(CorrectRotation(), { 0, 0, -1 });
}

glm::vec3 Camera::Left()
{
	return glm::rotate(CorrectRotation(), { -1, 0, 0 });
}

glm::vec3 Camera::Up()
{
	return glm::rotate(CorrectRotation(), { 0, 1, 0 });
}

glm::quat Camera::CorrectRotation()
{
	return glm::quat({ 0, 0, roll }) * rotation;
}

glm::mat4 Camera::GetViewMatrix()
{
	glm::mat4 view = glm::mat4(1.0f);
	view = glm::scale(view, scale);
	view = glm::rotate(view, angle(CorrectRotation()), -axis(CorrectRotation()));
	view = glm::translate(view, -location);
	return view;
}
