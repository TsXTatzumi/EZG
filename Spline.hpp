#pragma once
#include <vector>
#include <glm/gtx/quaternion.hpp>
#include <iostream>

struct sLocationTimeframe
{
	glm::vec3 location;
	float time;
};

struct sRotationTimeframe
{
	glm::quat rotation;
	float time;
};

class Spline
{
private:
	std::vector<sLocationTimeframe> location_timeframes;
	int i_locationIndex;
	float f_locationInterim;
	float f_locationSplineTime;
	std::vector<sRotationTimeframe> rotation_timeframes;
	int i_rotationIndex;
	float f_rotationInterim;

	bool b_looped = false;

	static float GetTime(glm::vec3 p0, glm::vec3 p1);


public:
	
	Spline();

	~Spline();

	void SetLooped(bool bLooped);

	void AddLocation(glm::vec3 location);

	void AddRotation(glm::vec3 euler, float time);

	void AddLocationWithRotation(glm::vec3 location, glm::vec3 euler);

	void move(float speed);

	glm::vec3 GetSplinePointLocation();

	glm::quat GetSplinePointRotation();

	void next();

	void prev();

	void removeLocation();

	void removeRotation();

	void print();
};

