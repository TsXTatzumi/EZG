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
	if (start >= end) return NULL; // empty tree

	//std::cout << "Lambda: \n";
	int axis = currLevel % m_dimension; // the axis to split on
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

	Node* newNode = new Node(*mid, currLevel);

	newNode->m_axis = axis;
	//std::cout << "m_cutingEdge: \n";
	if (mid == (end - 1)) {
		newNode->m_cutingEdge = ((*mid)[axis] + (*(mid - 1))[axis]) / 2.0;
	}
	else {
		newNode->m_cutingEdge = (((*mid)[axis] + (*(mid + 1))[axis]) / 2.0f);
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


KDTree::KDTree(const KDTree& rhs) {
	m_root = deepcopyTree(rhs.m_root);
	m_size = rhs.m_size;
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

void KDTree::drawWireframe(const Shader* shader, glm::mat4 model)
{
	/*std::cout << "Min:\n";
	std::cout << "x: " << m_minBound.x << "\n";
	std::cout << "y: " << m_minBound.y << "\n";
	std::cout << "z: " << m_minBound.z << "\n";
	std::cout << "Max:\n";
	std::cout << "x: " << m_maxBound.x << "\n";
	std::cout << "y: " << m_maxBound.y << "\n";
	std::cout << "z: " << m_maxBound.z << "\n";*/

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

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, boundingBox.size() * sizeof(glm::vec3), &boundingBox[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);

	glLineWidth(m_maxLevel / (0.0f+4.0f));
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
		plane.push_back(glm::vec3(node->m_cutingEdge, minBound.y, minBound.z));
		plane.push_back(glm::vec3(node->m_cutingEdge, minBound.y, maxBound.z));
		plane.push_back(glm::vec3(node->m_cutingEdge, minBound.y, maxBound.z));
		plane.push_back(glm::vec3(node->m_cutingEdge, maxBound.y, maxBound.z));
		plane.push_back(glm::vec3(node->m_cutingEdge, maxBound.y, maxBound.z));
		plane.push_back(glm::vec3(node->m_cutingEdge, maxBound.y, minBound.z));
		plane.push_back(glm::vec3(node->m_cutingEdge, maxBound.y, minBound.z));
		plane.push_back(glm::vec3(node->m_cutingEdge, minBound.y, minBound.z));
		break;
	case 1:
		plane.push_back(glm::vec3(minBound.x, node->m_cutingEdge, minBound.z));
		plane.push_back(glm::vec3(minBound.x, node->m_cutingEdge, maxBound.z));
		plane.push_back(glm::vec3(minBound.x, node->m_cutingEdge, maxBound.z));
		plane.push_back(glm::vec3(maxBound.x, node->m_cutingEdge, maxBound.z));
		plane.push_back(glm::vec3(maxBound.x, node->m_cutingEdge, maxBound.z));
		plane.push_back(glm::vec3(maxBound.x, node->m_cutingEdge, minBound.z));
		plane.push_back(glm::vec3(maxBound.x, node->m_cutingEdge, minBound.z));
		plane.push_back(glm::vec3(minBound.x, node->m_cutingEdge, minBound.z));
		break;
	case 2:
		plane.push_back(glm::vec3(minBound.x, minBound.y, node->m_cutingEdge));
		plane.push_back(glm::vec3(minBound.x, maxBound.y, node->m_cutingEdge));
		plane.push_back(glm::vec3(minBound.x, maxBound.y, node->m_cutingEdge));
		plane.push_back(glm::vec3(maxBound.x, maxBound.y, node->m_cutingEdge));
		plane.push_back(glm::vec3(maxBound.x, maxBound.y, node->m_cutingEdge));
		plane.push_back(glm::vec3(maxBound.x, minBound.y, node->m_cutingEdge));
		plane.push_back(glm::vec3(maxBound.x, minBound.y, node->m_cutingEdge));
		plane.push_back(glm::vec3(minBound.x, minBound.y, node->m_cutingEdge));
		break;
	}

	glLineWidth(m_maxLevel / (currLevel + 4.0f));

	glBufferData(GL_ARRAY_BUFFER, plane.size() * sizeof(glm::vec3), &plane[0], GL_STATIC_DRAW);

	glBindVertexArray(VAO);
	glDrawArrays(GL_LINES, 0, plane.size());
	glBindVertexArray(0);

	if (node->m_left != nullptr) {
		glm::vec3 newMax = maxBound;
		newMax[node->m_axis] = node->m_cutingEdge; //TODO Check ob das mit [] überhaupt richtig ist
		drawWireframeRecursive(shader, model, minBound, newMax, node->m_left, currLevel +1);
	}
	if (node->m_right != nullptr) {
		glm::vec3 newMin = minBound;
		minBound[node->m_axis] = node->m_cutingEdge; //TODO Check ob das mit [] überhaupt richtig ist
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
	if (currNode == NULL || currNode->m_point == pt) return currNode;

	const Point& currPoint = currNode->m_point;
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
	return node != NULL && node->m_point == pt;
}


void KDTree::insert(const Point& pt) {
	auto targetNode = findNode(m_root, pt);
	if (targetNode == NULL) { // this means the tree is empty
		m_root = new Node(pt, 0);
		m_size = 1;
	}
	else {
		if (targetNode->m_point == pt) { // pt is already in the tree, simply update its value
			//targetNode->m_value = value;
		}
		else { // construct a new node and insert it to the right place (child of targetNode)
			int currLevel = targetNode->m_level;
			Node* newNode = new Node(pt, currLevel + 1);
			if (pt[currLevel % m_dimension] < targetNode->m_point[currLevel % m_dimension]) {
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
	if (node == NULL || node->m_point != pt) {
		throw std::out_of_range("Point not found in the KD-Tree");
	}
	else {
		return node->m_point;
	}
}


Point& KDTree::at(const Point& pt) {
	const KDTree& constThis = *this;
	return const_cast<Point&>(constThis.at(pt));
}


Point& KDTree::operator[](const Point& pt) {
	auto node = findNode(m_root, pt);
	if (node != NULL && node->m_point == pt) { // pt is already in the tree
		return node->m_point;
	}
	else { // insert pt with default Point value, and return reference to the new Point
		insert(pt);
		if (node == NULL) return m_root->m_point; // the new node is the root
		else return (node->m_left != NULL && node->m_left->m_point == pt) ? node->m_left->m_point : node->m_right->m_point;
	}
}

/*
void KDTree::nearestNeighborRecurse(const typename KDTree::Node* currNode, const Point& key, BoundedPQueue<Point>& pQueue) const {
	if (currNode == NULL) return;
	const Point& currPoint = currNode->m_point;

	// Add the current point to the BPQ if it is closer to 'key' that some point in the BPQ
	pQueue.enqueue(currNode->m_value, Distance(currPoint, key));

	// Recursively search the half of the tree that contains Point 'key'
	int currLevel = currNode->m_level;
	bool isLeftTree;
	if (key[currLevel % m_dimension] < currPoint[currLevel % m_dimension]) {
		nearestNeighborRecurse(currNode->m_left, key, pQueue);
		isLeftTree = true;
	}
	else {
		nearestNeighborRecurse(currNode->m_right, key, pQueue);
		isLeftTree = false;
	}

	if (pQueue.size() < pQueue.maxSize() || fabs(key[currLevel % m_dimension] - currPoint[currLevel % m_dimension]) < pQueue.worst()) {
		// Recursively search the other half of the tree if necessary
		if (isLeftTree) nearestNeighborRecurse(currNode->m_right, key, pQueue);
		else nearestNeighborRecurse(currNode->m_left, key, pQueue);
	}
}
*/

/*
Point KDTree::kNNValue(const Point& key, std::size_t k) const {
	BoundedPQueue<Point> pQueue(k); // BPQ with maximum size k
	if (empty()) return Point(); // default return value if KD-tree is empty

	// Recursively search the KD-tree with pruning
	nearestNeighborRecurse(m_root, key, pQueue);

	// Count occurrences of all Point in the kNN set
	std::unordered_map<Point, int> counter;
	while (!pQueue.empty()) {
		++counter[pQueue.dequeueMin()];
	}

	// Return the most frequent element in the kNN set
	Point result;
	int cnt = -1;
	for (const auto& p : counter) {
		if (p.second > cnt) {
			result = p.first;
			cnt = p.second;
		}
	}
	return result;
}
*/

/*
bool KDTree::intersect(const Ray& ray)
{
	// Intersection test the ray against the tree bounding box.
	float tmin, tmax;
	if (!mWorldBox.intersect(ray, tmin, tmax)) return false;

	// Prepare for tree traversal.
	unsigned int rayId = ++mCurrentRayId;	// get new ray id
	Vector invDir = 1.0f / ray.dir;			// compute reciprocal ray direction
	int current = 0;						// current node
	mNodesTodo.resize(0);					// clear node todo list, this shouldn't clear previously reserved mem
	bool hit = false;

	// Traverse the tree until the ray exists or hits something.
	while (true)
	{
		if (ray.maxT < tmin) break;
		node& currentNode = mNodes[current];

		if (!currentNode.isLeaf())		// process interior node
		{
			// Compute splitting plane intersection time.
			int axis = currentNode.getAxis();
			float split = currentNode.getSplit();
			float orig = ray.orig(axis);
			float t = (split - orig) * invDir(axis);

			// Choose traversal order based on ray origin.
			int first, second;
			if (orig <= split)
			{
				first = current + 1;
				second = currentNode.getFarChild();
			}
			else
			{
				first = currentNode.getFarChild();
				second = current + 1;
			}

			// Check if we need to traverse both children of just one side.
			if (t > tmax || t < 0.0f)	// first only
				current = first;
			else if (t < tmin)			// second only
				current = second;
			else					// both, put second in todo list
			{
				mNodesTodo.push_back(nodeTodo(second, t, tmax));
				current = first;
				tmax = t;
			}
		}
		else		// process leaf node
		{
			// Process leaf node by checking for intersections with objects in leaf.
			int numObjs = currentNode.getNum();
			if (numObjs == 1)
			{
				// Just one object in node
				object& obj = currentNode.getObj();
				if (obj.lastId != rayId)		// check mailbox id so we are not testing the same object twice
				{
					obj.lastId = rayId;
					if (obj.ptr->intersect(ray))
					{
						hit = true;
						break;				// return true on first intersection
					}
				}
			}
			else
			{
				// More than one object, loop over objects
				for (int i = 0; i < numObjs; i++)
				{
					object& obj = currentNode.getObj(i);
					if (obj.lastId != rayId)		// check mailbox id so we are not testing the same object twice
					{
						obj.lastId = rayId;
						if (obj.ptr->intersect(ray))
						{
							hit = true;
							break;				// return true on first intersection
						}
					}
				}
			}

			// Check todo list for next node to process. If empty, the ray has passed
			// through the tree without hitting anything.
			if (!mNodesTodo.empty())
			{
				current = mNodesTodo.back().idx;
				tmin = mNodesTodo.back().tmin;
				tmax = mNodesTodo.back().tmax;
				mNodesTodo.pop_back();
			}
			else break;
		}
	}

	return hit;
}
*/