#include "Ray.h"

Ray::Ray() : minT(0.001f), maxT(INF) {
}

Ray::Ray(const Point& o, const glm::vec3& d, float mint, float maxt) : orig(o), dir(d), minT(mint), maxT(maxt) {
	dir = glm::normalize(dir);
}

Ray::~Ray()
{
}
