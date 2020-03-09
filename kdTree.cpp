#include "kdTree.h"
#include <iostream>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

KDTree::KDTree() : m_root(NULL), m_size(0) { }


KDTree::Node* KDTree::deepcopyTree(typename KDTree::Node* root) {
	if (root == NULL) return NULL;
	Node* newRoot = new Node(*root);
	newRoot->m_left = deepcopyTree(root->m_left);
	newRoot->m_right = deepcopyTree(root->m_right);
	return newRoot;
}


KDTree::Node* KDTree::buildTree(typename std::vector<Point>::iterator start, typename std::vector<Point>::iterator end, int currLevel) {
	m_StartIt = start;

	if (start >= end) return NULL; // empty tree

	//std::cout << "Lambda: \n";
	int axis = currLevel % m_dimension; // TODO split on max dimension
	auto cmp = [axis](const Point& p1, const Point p2) {
		return p1[axis] < p2[axis];
	};

	//std::cout << "Nth elem: \n";

	std::size_t len = end - start;
	std::vector<Point>::iterator mid = start + len / 2;
	std::nth_element(start, mid, end, cmp); // linear time partition

	//std::cout << "While: \n";

	// move left (if needed) so that all the equal points are to the right
	// The tree will still be balanced as long as there aren't many points that are equal along each axis

	while (mid > start && (*(mid - 1))[axis] == (*mid)[axis]) {
		//std::cout << "mid > start: " << (mid > start) << "\n";
		//std::cout << "(mid - 1)[axis] == mid[axis]: " << ((*(mid - 1))[axis] == (*mid)[axis]) << "\n";
		--mid;
		//std::cout << "New mid: " << &mid << "\n";
	}

	//std::cout << "Create new node: \n";

	if (m_maxLevel < currLevel) {
		m_maxLevel = m_maxLevel + 1;
	}

	Node* newNode = new Node(mid, currLevel);

	newNode->m_axis = axis;
	//std::cout << "m_cutingEdge: \n";
	if (mid == (end - 1)) {
		newNode->m_cuttingEdge = ((*mid)[axis] + (*(mid - 1))[axis]) / 2.0;
	}
	else {
		newNode->m_cuttingEdge = (((*mid)[axis] + (*(mid + 1))[axis]) / 2.0f);
	}
	//std::cout << "35: \n";

	newNode->m_left = buildTree(start, mid, currLevel + 1);
	newNode->m_right = buildTree(mid + 1, end, currLevel + 1);
	return newNode;
}


KDTree::KDTree(std::vector<Point>& points) {


	if (points.size() > 0) {

		m_minBound.x = points[0].x;
		m_minBound.y = points[0].y;
		m_minBound.z = points[0].z;

		m_maxBound = m_minBound;

		for (int i = 0; i < points.size(); i++) {
			//Min
			if (points[i].x < m_minBound.x) {
				m_minBound.x = points[i].x;
			}
			if (points[i].y < m_minBound.y) {
				m_minBound.y = points[i].y;
			}
			if (points[i].z < m_minBound.z) {
				m_minBound.z = points[i].z;
			}
			//Max
			if (points[i].x > m_maxBound.x) {
				m_maxBound.x = points[i].x;
			}
			if (points[i].y > m_maxBound.y) {
				m_maxBound.y = points[i].y;
			}
			if (points[i].z > m_maxBound.z) {
				m_maxBound.z = points[i].z;
			}
		}


		m_root = buildTree(points.begin(), points.end(), 0);
		m_size = points.size();
	}
}



KDTree& KDTree::operator=(const KDTree& rhs) {
	if (this != &rhs) { // make sure we don't self-assign
		freeResource(m_root);
		m_root = deepcopyTree(rhs.m_root);
		m_size = rhs.m_size;
	}
	return *this;
}


void KDTree::freeResource(typename KDTree::Node* currNode) {
	if (currNode == NULL) return;
	freeResource(currNode->m_left);
	freeResource(currNode->m_right);
	delete currNode;
}


KDTree::~KDTree() {
	freeResource(m_root);
}


std::size_t KDTree::dimension() const {
	return m_dimension;
}

KDTree::KDTree(const KDTree& rhs) {


	m_root = deepcopyTree(rhs.m_root);
	m_size = rhs.m_size;
}

void KDTree::initBuffer() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	m_BufferInit = true;
}

void KDTree::drawWireframe(const Shader* shader, glm::mat4 model)
{

	if (!m_BufferInit) {
		initBuffer();
	}

	std::vector<glm::vec3> boundingBox;
	boundingBox.push_back(m_minBound);
	boundingBox.push_back(glm::vec3(m_minBound.x, m_minBound.y, m_maxBound.z));
	boundingBox.push_back(glm::vec3(m_minBound.x, m_minBound.y, m_maxBound.z));
	boundingBox.push_back(glm::vec3(m_minBound.x, m_maxBound.y, m_maxBound.z));
	boundingBox.push_back(glm::vec3(m_minBound.x, m_maxBound.y, m_maxBound.z));
	boundingBox.push_back(glm::vec3(m_minBound.x, m_maxBound.y, m_minBound.z));
	boundingBox.push_back(glm::vec3(m_minBound.x, m_maxBound.y, m_minBound.z));
	boundingBox.push_back(m_minBound);


	boundingBox.push_back(glm::vec3(m_maxBound.x, m_minBound.y, m_minBound.z));
	boundingBox.push_back(glm::vec3(m_maxBound.x, m_minBound.y, m_maxBound.z));
	boundingBox.push_back(glm::vec3(m_maxBound.x, m_minBound.y, m_maxBound.z));
	boundingBox.push_back(m_maxBound);
	boundingBox.push_back(m_maxBound);
	boundingBox.push_back(glm::vec3(m_maxBound.x, m_maxBound.y, m_minBound.z));
	boundingBox.push_back(glm::vec3(m_maxBound.x, m_maxBound.y, m_minBound.z));
	boundingBox.push_back(glm::vec3(m_maxBound.x, m_minBound.y, m_minBound.z));

	boundingBox.push_back(m_minBound);
	boundingBox.push_back(glm::vec3(m_maxBound.x, m_minBound.y, m_minBound.z));
	boundingBox.push_back(glm::vec3(m_minBound.x, m_minBound.y, m_maxBound.z));
	boundingBox.push_back(glm::vec3(m_maxBound.x, m_minBound.y, m_maxBound.z));
	boundingBox.push_back(glm::vec3(m_minBound.x, m_maxBound.y, m_maxBound.z));
	boundingBox.push_back(m_maxBound);
	boundingBox.push_back(glm::vec3(m_minBound.x, m_maxBound.y, m_minBound.z));
	boundingBox.push_back(glm::vec3(m_maxBound.x, m_maxBound.y, m_minBound.z));



	glBufferData(GL_ARRAY_BUFFER, boundingBox.size() * sizeof(glm::vec3), &boundingBox[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);

	glLineWidth(m_maxLevel / (0.0f + 4.0f));
	//Draw Cube
	glBindVertexArray(VAO);
	glDrawArrays(GL_LINES, 0, boundingBox.size());
	glBindVertexArray(0);


	//Start rec
	drawWireframeRecursive(shader, model, m_minBound, m_maxBound, m_root, 0);
}

void KDTree::drawWireframeRecursive(const Shader* shader, glm::mat4 model, glm::vec3 minBound, glm::vec3 maxBound, Node* node, int currLevel) {

	std::vector<glm::vec3> plane;
	switch (node->m_axis)
	{
	case 0:
		plane.push_back(glm::vec3(node->m_cuttingEdge, minBound.y, minBound.z));
		plane.push_back(glm::vec3(node->m_cuttingEdge, minBound.y, maxBound.z));
		plane.push_back(glm::vec3(node->m_cuttingEdge, minBound.y, maxBound.z));
		plane.push_back(glm::vec3(node->m_cuttingEdge, maxBound.y, maxBound.z));
		plane.push_back(glm::vec3(node->m_cuttingEdge, maxBound.y, maxBound.z));
		plane.push_back(glm::vec3(node->m_cuttingEdge, maxBound.y, minBound.z));
		plane.push_back(glm::vec3(node->m_cuttingEdge, maxBound.y, minBound.z));
		plane.push_back(glm::vec3(node->m_cuttingEdge, minBound.y, minBound.z));
		break;
	case 1:
		plane.push_back(glm::vec3(minBound.x, node->m_cuttingEdge, minBound.z));
		plane.push_back(glm::vec3(minBound.x, node->m_cuttingEdge, maxBound.z));
		plane.push_back(glm::vec3(minBound.x, node->m_cuttingEdge, maxBound.z));
		plane.push_back(glm::vec3(maxBound.x, node->m_cuttingEdge, maxBound.z));
		plane.push_back(glm::vec3(maxBound.x, node->m_cuttingEdge, maxBound.z));
		plane.push_back(glm::vec3(maxBound.x, node->m_cuttingEdge, minBound.z));
		plane.push_back(glm::vec3(maxBound.x, node->m_cuttingEdge, minBound.z));
		plane.push_back(glm::vec3(minBound.x, node->m_cuttingEdge, minBound.z));
		break;
	case 2:
		plane.push_back(glm::vec3(minBound.x, minBound.y, node->m_cuttingEdge));
		plane.push_back(glm::vec3(minBound.x, maxBound.y, node->m_cuttingEdge));
		plane.push_back(glm::vec3(minBound.x, maxBound.y, node->m_cuttingEdge));
		plane.push_back(glm::vec3(maxBound.x, maxBound.y, node->m_cuttingEdge));
		plane.push_back(glm::vec3(maxBound.x, maxBound.y, node->m_cuttingEdge));
		plane.push_back(glm::vec3(maxBound.x, minBound.y, node->m_cuttingEdge));
		plane.push_back(glm::vec3(maxBound.x, minBound.y, node->m_cuttingEdge));
		plane.push_back(glm::vec3(minBound.x, minBound.y, node->m_cuttingEdge));
		break;
	}


	glLineWidth(m_maxLevel / (currLevel + 4.0f));

	glBufferSubData(GL_ARRAY_BUFFER, 0, plane.size() * sizeof(glm::vec3), &plane[0]);

	glBindVertexArray(VAO);
	glDrawArrays(GL_LINES, 0, plane.size());
	glBindVertexArray(0);


	if (node->m_left != nullptr) {
		glm::vec3 newMax = maxBound;
		newMax[node->m_axis] = node->m_cuttingEdge; //TODO Check ob das mit [] überhaupt richtig ist
		drawWireframeRecursive(shader, model, minBound, newMax, node->m_left, currLevel + 1);
	}
	if (node->m_right != nullptr) {
		glm::vec3 newMin = minBound;
		minBound[node->m_axis] = node->m_cuttingEdge; //TODO Check ob das mit [] überhaupt richtig ist
		drawWireframeRecursive(shader, model, newMin, maxBound, node->m_right, currLevel + 1);
	}
}


std::size_t KDTree::size() const {
	return m_size;
}


bool KDTree::empty() const {
	return m_size == 0;
}


KDTree::Node* KDTree::findNode(typename KDTree::Node* currNode, const Point& pt) const {
	if (currNode == NULL || *(currNode->m_PointIt) == pt) return currNode;

	const Point& currPoint = *(currNode->m_PointIt);
	int currLevel = currNode->m_level;
	if (pt[currLevel % m_dimension] < currPoint[currLevel % m_dimension]) { // recurse to the left side
		return currNode->m_left == NULL ? currNode : findNode(currNode->m_left, pt);
	}
	else { // recurse to the right side
		return currNode->m_right == NULL ? currNode : findNode(currNode->m_right, pt);
	}
}


bool KDTree::contains(const Point& pt) const {
	auto node = findNode(m_root, pt);
	return node != NULL && *(node->m_PointIt) == pt;
}

/*
void KDTree::insert(const Point& pt) {
	auto targetNode = findNode(m_root, pt);
	if (targetNode == NULL) { // this means the tree is empty
		m_root = new Node(pt, 0);
		m_size = 1;
	}
	else {
		if (*(targetNode->m_PointIt) == pt) { // pt is already in the tree, simply update its value
			//targetNode->m_value = value;
		}
		else { // construct a new node and insert it to the right place (child of targetNode)
			int currLevel = targetNode->m_level;
			Node* newNode = new Node(pt, currLevel + 1);
			if (pt[currLevel % m_dimension] < targetNode->m_PointIt[currLevel % m_dimension]) {
				targetNode->m_left = newNode;
			}
			else {
				targetNode->m_right = newNode;
			}
			++m_size;
		}
	}
}
const Point& KDTree::at(const Point& pt) const {
	auto node = findNode(m_root, pt);
	if (node == NULL || node->m_PointIt) != pt) {
		throw std::out_of_range("Point not found in the KD-Tree");
	}
	else {
		return node->m_PointIt);
	}
}


Point& KDTree::at(const Point& pt) {
	const KDTree& constThis = *this;
	return const_cast<Point&>(constThis.at(pt));
}


Point& KDTree::operator[](const Point& pt) {
	auto node = findNode(m_root, pt);
	if (node != NULL && node->m_PointIt) == pt) { // pt is already in the tree
		return node->m_PointIt);
	}
	else { // insert pt with default Point value, and return reference to the new Point
		insert(pt);
		if (node == NULL) return m_root->m_PointIt); // the new node is the root
		else return (node->m_left != NULL && node->m_left->m_PointIt) == pt) ? node->m_left->m_PointIt) : node->m_right->m_PointIt);
	}
}
*/

std::vector<Point> KDTree::intersect(const Ray& ray) {

	return intersect(ray, m_minBound, m_maxBound, m_root, 0);
}

bool isInBox(const Ray& ray, glm::vec3 minBound, glm::vec3 maxBound) {

	int axis[3];

	if (ray.dir.x - glm::normalize(maxBound - ray.m_start).x) {
		axis[0] += 1;
	}
	else {
		axis[0] -= 1;
	}

	if (ray.dir.y - glm::normalize(maxBound - ray.m_start).y) {
		axis[1] += 1;
	}
	else {
		axis[1] -= 1;
	}

	if (ray.dir.z - glm::normalize(maxBound - ray.m_start).z) {
		axis[2] += 1;
	}
	else {
		axis[2] -= 1;
	}

	if (ray.dir.x - glm::normalize(maxBound - ray.m_start).x) {
		axis[0] += 1;
	}
	else {
		axis[0] -= 1;
	}

	if (ray.dir.y - glm::normalize(maxBound - ray.m_start).y) {
		axis[1] += 1;
	}
	else {
		axis[1] -= 1;
	}

	if (ray.dir.z - glm::normalize(maxBound - ray.m_start).z) {
		axis[2] += 1;
	}
	else {
		axis[2] -= 1;
	}

	return !(axis[0] + axis[1] + axis[2]);
}

std::vector<Point> KDTree::intersect(const Ray& ray, glm::vec3 minBound, glm::vec3 maxBound, Node* node, int currLevel)
{
	bool inBox = isInBox(ray, minBound, maxBound);

	if (!inBox) {
		return std::vector<Point>();
	}

	if ((node->m_right != nullptr) && (node->m_left != nullptr)) {
		switch (node->m_axis) {
		case 0:
			//X
			if (ray.m_start.x > node->m_cuttingEdge) {

				glm::vec3 newMin = minBound;
				minBound[node->m_axis] = node->m_cuttingEdge;
				intersect(ray, newMin, maxBound, node->m_right, currLevel + 1);

				glm::vec3 newMax = maxBound;
				newMax[node->m_axis] = node->m_cuttingEdge;
				intersect(ray, minBound, newMax, node->m_left, currLevel + 1);


			}
			else {
				glm::vec3 newMax = maxBound;
				newMax[node->m_axis] = node->m_cuttingEdge;
				intersect(ray, minBound, newMax, node->m_left, currLevel + 1);

				glm::vec3 newMin = minBound;
				minBound[node->m_axis] = node->m_cuttingEdge;
				intersect(ray, newMin, maxBound, node->m_right, currLevel + 1);

			}
			break;
		case 1:
			//Y
			if (ray.m_start.y > node->m_cuttingEdge) {

				glm::vec3 newMin = minBound;
				minBound[node->m_axis] = node->m_cuttingEdge;
				intersect(ray, newMin, maxBound, node->m_right, currLevel + 1);

				glm::vec3 newMax = maxBound;
				newMax[node->m_axis] = node->m_cuttingEdge;
				intersect(ray, minBound, newMax, node->m_left, currLevel + 1);


			}
			else {
				glm::vec3 newMax = maxBound;
				newMax[node->m_axis] = node->m_cuttingEdge;
				intersect(ray, minBound, newMax, node->m_left, currLevel + 1);

				glm::vec3 newMin = minBound;
				minBound[node->m_axis] = node->m_cuttingEdge;
				intersect(ray, newMin, maxBound, node->m_right, currLevel + 1);

			}

			break;
		case 2:
			//Z
			if (ray.m_start.z > node->m_cuttingEdge) {

				glm::vec3 newMin = minBound;
				minBound[node->m_axis] = node->m_cuttingEdge;
				intersect(ray, newMin, maxBound, node->m_right, currLevel + 1);

				glm::vec3 newMax = maxBound;
				newMax[node->m_axis] = node->m_cuttingEdge;
				intersect(ray, minBound, newMax, node->m_left, currLevel + 1);


			}
			else {
				glm::vec3 newMax = maxBound;
				newMax[node->m_axis] = node->m_cuttingEdge;
				intersect(ray, minBound, newMax, node->m_left, currLevel + 1);

				glm::vec3 newMin = minBound;
				minBound[node->m_axis] = node->m_cuttingEdge;
				intersect(ray, newMin, maxBound, node->m_right, currLevel + 1);
			}
			break;
		}
	}
	else {
		std::vector<Point> triangel = std::vector<Point>();
		int index = node->m_PointIt - m_StartIt;
		index = index / 3;

		glm::vec3 p1 = glm::vec3((m_StartIt + index)->x, (m_StartIt + index)->y, (m_StartIt + index)->z);
		glm::vec3 p2 = glm::vec3((m_StartIt + index + 1)->x, (m_StartIt + index + 1)->y, (m_StartIt + index + 1)->z);
		glm::vec3 p3 = glm::vec3((m_StartIt + index + 2)->x, (m_StartIt + index + 2)->y, (m_StartIt + index + 2)->z);


		if (rayTriangleIntersect(ray, p1, p2, p3)) {
			triangel.push_back(*(m_StartIt + index));
			triangel.push_back(*(m_StartIt + index + 1));
			triangel.push_back(*(m_StartIt + index + 2));
			return triangel;
		}
	}

	return std::vector<Point>();
}

float KDTree::rayTriangleIntersect(const Ray& r, glm::vec3& v0, glm::vec3& v1, glm::vec3& v2)
{
	glm::vec3 v0v1 = v1 - v0;
	glm::vec3 v0v2 = v2 - v0;

	glm::vec3 pvec = glm::cross(r.dir, v0v2);

	float det = dot(v0v1, pvec);

	if (det < 0.000001)
		return -INFINITY;

	float invDet = 1.0 / det;

	glm::vec3 tvec = r.m_start - v0;

	float u = glm::dot(tvec, pvec) * invDet;

	if (u < 0 || u > 1)
		return -INFINITY;

	glm::vec3 qvec = cross(tvec, v0v1);

	float v = dot(r.dir, qvec) * invDet;

	if (v < 0 || u + v > 1)
		return -INFINITY;

	return glm::dot(v0v2, qvec) * invDet;
}