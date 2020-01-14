
#ifndef KD_TREE_H
#define KD_TREE_H

#include <stdint.h>
#include <vector>
#include <stdexcept>
#include <algorithm>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Ray.h"
#include "MathDefines.h"
#include "Shader.hpp"

class KDTree {
private:

	struct Node {
		//Point m_point;
		std::vector<Point>::iterator m_PointIt;
		uint8_t m_axis;
		float m_cuttingEdge;
		Node* m_left = nullptr;
		Node* m_right = nullptr;
		int m_level;  // level of the node in the tree, starts at 0 for the root
		Node(std::vector<Point>::iterator& pt, int level) : m_PointIt(pt), m_left(NULL), m_right(NULL), m_level(level) {

		}

		bool isLeaf() {
			return (m_left == nullptr) && (m_right == nullptr);
		}
	};
public:
	KDTree();
	KDTree(std::vector<Point>& points);
	~KDTree();

	KDTree(const KDTree& rhs);
	KDTree& operator=(const KDTree& rhs);
	
	std::vector<Point> intersect(const Ray& ray);

	std::size_t size() const;
	bool empty() const;

	bool contains(const Point& pt) const;

	//void insert(const Point& pt);

	Point& operator[](const Point& pt);
	//Point& at(const Point& pt);
	//const Point& at(const Point& pt) const;
	//Point kNNValue(const Point& key, std::size_t k) const;


	std::size_t dimension() const;

	void drawWireframe(const Shader * shader,glm::mat4 model);

	GLuint VAO, VBO;
private:

	std::vector<Point>::iterator m_StartIt;

	Node* m_root;
	std::size_t m_size;

	glm::vec3 m_minBound;
	glm::vec3 m_maxBound;

	int m_maxLevel = 0;

	int mCurrentRayId = 0;

	const uint8_t m_dimension = 3;
	std::vector<glm::vec3> * m_TreeWires;

	bool m_BufferInit = false;

	void initBuffer();

	Node* buildTree(typename std::vector<Point>::iterator start, typename std::vector<Point>::iterator end, int currLevel);
	Node* findNode(Node* currNode, const Point& pt) const;
	Node* deepcopyTree(Node* root);
	void freeResource(Node* currNode);

	void drawWireframeRecursive(const Shader * shader, glm::mat4 model, glm::vec3 minBound, glm::vec3 maxBound, Node * node, int currLevel);

	std::vector<Point> intersect(const Ray& ray, glm::vec3 minBound, glm::vec3 maxBound, Node* node, int currLevel);

	float rayTriangleIntersect(const Ray& r, glm::vec3& v0, glm::vec3& v1, glm::vec3& v2);

};


#endif // !KD_TREE_H
