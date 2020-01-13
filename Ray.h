
#ifndef RAY_H
#define RAY_H

#include <stdint.h>
#include <glm/glm.hpp>
#include "MathDefines.h"

class Ray
{
public:
	Point orig;		//Startpounkt
	glm::vec3 dir;		//Richtungsvektor
	float minT;
	float maxT;

public:
	Ray();

	Ray(const Point& o, const glm::vec3& d, float mint = 0.001f, float maxt = INF);

	~Ray();
};

#endif // !RAY_H
