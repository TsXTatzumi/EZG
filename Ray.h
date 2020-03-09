
#ifndef RAY_H
#define RAY_H

#include <stdint.h>
#include <glm/glm.hpp>
#include "MathDefines.h"

class Ray
{
public:
	glm::vec3 m_start;		//Startpounkt
	glm::vec3 m_end;
	float minT;
	float maxT;

public:
	Ray();

	Ray(const glm::vec3& start, const glm::vec3& end, float mint = 0.001f, float maxt = INF);

	~Ray();

	glm::vec3 dir;		//Richtungsvektor
};

#endif // !RAY_H
