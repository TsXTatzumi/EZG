#include "Ray.h"

Ray::Ray() : minT(0.001f), maxT(INF) {
}

Ray::Ray(const glm::vec3& start, const glm::vec3& end, float mint, float maxt) : m_start(start), m_end(end), minT(mint), maxT(maxt) {
	dir = glm::normalize(end - start);
}

Ray::~Ray()
{
}
