

#ifndef MATH_DEFINES_H
#define MATH_DEFINES_H

#include <glad/glad.h>
#include <stdint.h>
#include <stdexcept>

static const float INF = std::numeric_limits<float>::infinity();

struct Point {
public:
	float x = 0;
	float y = 0;
	float z = 0;

	GLuint objID = 0;

	float& operator[] (std::size_t index) {
		switch (index)
		{
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		default:
			throw std::out_of_range("Point has only 3 dimensions.");
		}
	}

	float operator[] (std::size_t index) const {
		switch (index)
		{
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		default:
			throw std::out_of_range("Point has only 3 dimensions.");
		}
	}


	//TODO
	bool operator==(const Point& rhs) {
		return (this == &rhs);
		//return (this->x == rhs.x) && (this->y == rhs.y) && (this->z == rhs.z);
	}

	bool operator!=(const Point& rhs) {
		return (this != &rhs);
		//return (this->x != rhs.x) || (this->y != rhs.y) || (this->z == rhs.z);
	}
};


#endif // !MATH_DEFINES_H

