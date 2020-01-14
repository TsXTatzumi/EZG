 
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Camera.hpp"

#include <iostream>
#include "Model.hpp"
#include "WorldObject.hpp"

#include "kdTree.h"
#include "Spline.hpp"

void setSamples();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
void renderScene(const Shader * shader, bool lightning);

void renderQuad();

float max_RayLength = 1000.0f;
void rayCast();

GLuint framebuffer;
GLuint samples = 1;

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera
Camera camera({0.0f, 0.0f, 3.0f } , {0.0f, 0.0f, 0.0f});
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;
float speed = 3.0f;

Spline path;

bool b_edit;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;


float bumpiness = 1.0f;

std::vector<WorldObject*> worldObjects;

std::vector<WorldObject*> lights;

//KDTree
KDTree * kdTree;
Shader* kdTreeShader;

bool fireRay = false;

void setSamples()
{
	std::cout << samples << std::endl;
	// configure MSAA framebuffer
	// --------------------------
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	// create a multisampled color attachment texture
	unsigned int textureColorBufferMultiSampled;
	glGenTextures(1, &textureColorBufferMultiSampled);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, SCR_WIDTH, SCR_HEIGHT, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled, 0);
	// create a (also multisampled) renderbuffer object for depth and stencil attachments
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "EZG", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	float vertarray[] = {
		// back face
		 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
		 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right  
		-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left       
		 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
		-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
		// front face
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
		 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
		 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
		 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
		-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
		-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
		// left face
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
		-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
		-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
		-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
		-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
		// right face
		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
		 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
		 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
		 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
		// bottom face
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
		 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
		 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
		 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
		-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
		-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
		// top face
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
		 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
		 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
		-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
		-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left       
	};

	std::vector<GLuint> indices;
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(3);
	indices.push_back(4);
	indices.push_back(5);
	indices.push_back(6);
	indices.push_back(7);
	indices.push_back(8);
	indices.push_back(9);
	indices.push_back(10);
	indices.push_back(11);
	indices.push_back(12);
	indices.push_back(13);
	indices.push_back(14);
	indices.push_back(15);
	indices.push_back(16);
	indices.push_back(17);
	indices.push_back(18);
	indices.push_back(19);
	indices.push_back(20);
	indices.push_back(21);
	indices.push_back(22);
	indices.push_back(23);
	indices.push_back(24);
	indices.push_back(25);
	indices.push_back(26);
	indices.push_back(27);
	indices.push_back(28);
	indices.push_back(29);
	indices.push_back(30);
	indices.push_back(31);
	indices.push_back(32);
	indices.push_back(33);
	indices.push_back(34);
	indices.push_back(35);

	std::vector<Vertex> vertices;
	for (int i = 0; i < 36; i++)
	{
		vertices.push_back({ {vertarray[i * 8 + 0], vertarray[i * 8 + 1], vertarray[i * 8 + 2]},
								{vertarray[i * 8 + 3], vertarray[i * 8 + 4], vertarray[i * 8 + 5]},
								{vertarray[i * 8 + 6], vertarray[i * 8 + 7]},
								{0, 0, 0 }});
		
		if (i%3 != 2) continue;
		
		glm::vec3 edge1 = vertices[i - 1].Position - vertices[i - 2].Position;
		glm::vec3 edge2 = vertices[i    ].Position - vertices[i - 2].Position;
		glm::vec2 deltaUV1 = vertices[i - 1].TexCoords - vertices[i - 2].TexCoords;
		glm::vec2 deltaUV2 = vertices[i    ].TexCoords - vertices[i - 2].TexCoords;

		GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		vertices[i].Tangent = glm::normalize(glm::vec3(f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
			f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
			f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z)));

		vertices[i - 1].Tangent = vertices[i].Tangent;
		vertices[i - 2].Tangent = vertices[i].Tangent;
	}

	glActiveTexture(GL_TEXTURE0);
	const GLuint texture = loadTexture("container.jpg");
	const GLuint normal_map = loadTexture("normal.png");

	Model * cube = new Model(vertices, indices, texture, normal_map);
	cube->setModelType(GL_TRIANGLES);

	worldObjects.push_back(new WorldObject());
	worldObjects[0]->setModel(cube);
	worldObjects[0]->location = { 0, -5, -10 };
	worldObjects[0]->rotation = glm::quat({ 0,0.2,0 });
	worldObjects[0]->scale = { 1, 1, 3 };

	worldObjects.push_back(new WorldObject());
	worldObjects[1]->setModel(cube);
	worldObjects[1]->location = { -10, 2, -15 };

	worldObjects.push_back(new WorldObject());
	worldObjects[2]->setModel(cube);
	worldObjects[2]->location = { 10, 5, -20 };

	worldObjects.push_back(new WorldObject());
	worldObjects[3]->setModel(cube);
	worldObjects[3]->location = { 0, 0, -25 };

	worldObjects.push_back(new WorldObject());
	worldObjects[4]->setModel(cube);
	worldObjects[4]->location = { 0, -7, -10 };
	worldObjects[4]->scale = { 50, 1, 50 };

	// spline
	// -------------


	path.AddLocation({ 0, 0, 0 });
	path.AddLocation({ -13.019, 6.03497e-07, -6.33639 });
	path.AddLocation({ -22.723, 8.5216e-07, -25.7333 });
	path.AddLocation({ 3.69777, 16.7579, -39.0616 });
	path.AddLocation({ 23.5036, 16.7579, -23.4847 });
	path.AddLocation({ 2.57999, -4.35475, -14.3093 });
	path.AddLocation({ -5.70131, -5.26008, -13.2413 });
	path.AddLocation({ -9.05775, 7.77142, -10.1961 });
	path.AddLocation({ -8.35377, 6.98028, -12.3498 });
	path.AddLocation({ -5.85255, 3.81571, -25.1518 });
	path.AddLocation({ -1.44609, 24.7389, -33.1356 });
	path.AddLocation({ -0.678881, 50.5285, -22.3502 });
	path.AddLocation({ -1.82137, 74.9814, -19.6965 });
	path.AddLocation({ -1.90223, 23.0938, -9.54347 });
	path.AddLocation({ -5.32363, 13.9461, -0.865797 });
	path.AddLocation({ -0.37169, 1.30858, -15.3703 });
	path.AddLocation({ -16.1956, -0.559416, -23.7236 });
	path.AddLocation({ 6.10949, -0.559376, -30.7428 });
	path.AddLocation({ 15.3143, -0.496965, -13.7454 });
	path.AddLocation({ 20.9462, -20.6374, 0.00605643 });
	path.AddLocation({ -0.765951, -16.5774, 2.11632 });
	path.AddLocation({ 2.02551, -6.01751, 1.72638 });

	path.AddRotation({ 0, -0, 0 }, 0);
	path.AddRotation({ -0.0359997, -0.847976, 2.25263e-08 }, 3.80514);
	path.AddRotation({ -3.12959, -1.2935, -3.14159 }, 8.46227);
	path.AddRotation({ 2.46959, 0.366386, -3.14159 }, 14.2939);
	path.AddRotation({ -0.492001, 1.10306, -2.21436e-06 }, 19.3136);
	path.AddRotation({ 0.736001, -0.89668, -1.33684e-06 }, 24.8911);
	path.AddRotation({ -2.72959, -0.592678, -3.14159 }, 27.7892);
	path.AddRotation({ -1.216, -0.939718, -3.73782e-06 }, 31.5036);
	path.AddRotation({ -0.336002, -0.31593, -2.41427e-06 }, 33.0528);
	path.AddRotation({ 2.79359, -0.933281, -3.14159 }, 36.7164);
	path.AddRotation({ 2.18159, -0.201542, -3.14159 }, 41.4939);
	path.AddRotation({ -1.54013, 1.54298, -0.000128567 }, 46.782);
	path.AddRotation({ -1.464, -0.836508, -3.91399e-06 }, 51.7442);
	path.AddRotation({ 1.3176, 0.994755, -3.14159 }, 59.0155);
	path.AddRotation({ -0.747998, -0.18476, -1.34917e-06 }, 62.63);
	path.AddRotation({ -0.00799707, -0.668543, -1.9526e-06 }, 67.087);
	path.AddRotation({ -3.03759, -1.0841, -3.14159 }, 71.3285);
	path.AddRotation({ -3.10559, 0.515022, -3.14159 }, 76.1642);
	path.AddRotation({ 0.0879967, 1.22534, -5.12653e-06 }, 80.5608);
	path.AddRotation({ 0.72, 0.51589, -2.46681e-06 }, 85.5637);
	path.AddRotation({ 0.744005, -0.687468, -3.79829e-06 }, 90.2742);
	path.AddRotation({ 0.336003, -0.00785652, -1.13427e-06 }, 93.5802);

	path.SetLooped(true);
	
	// lighting info
	// -------------
	glm::vec3 lightPos(10.0f, 18.0f, 10.0f);

	lights.push_back(new WorldObject());
	lights[0]->setModel(cube);
	//lights[0]->location = lightPos;
	//lights[0]->scale = { 3, 3, 3 };

	// build and compile shaders
	// -------------------------
	Shader * simpleDepthShader = new Shader("Depth_VS.glsl", "Empty_FS.glsl");
	Shader * shader = new Shader("ShadowMap_VS.glsl", "ShadowMap_FS.glsl");
	Shader * debugDepthQuad = new Shader("Debug_VS.glsl", "Debug_FS.glsl");
	kdTreeShader = new Shader("SimpleDraw_VS.glsl","SimpleDraw_FS.glsl");

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	//float planeVertices[] = {
	//	// positions            // normals         // texcoords
	//	 25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
	//	-25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
	//	-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

	//	 25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
	//	-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
	//	 25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
	//};
	//// plane VAO
	//unsigned int planeVBO;
	//glGenVertexArrays(1, &planeVAO);
	//glGenBuffers(1, &planeVBO);
	//glBindVertexArray(planeVAO);
	//glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	//glEnableVertexAttribArray(1);
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	//glEnableVertexAttribArray(2);
	//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	//glBindVertexArray(0);

	// configure depth map FBO
	// -----------------------
	const unsigned int SHADOW_WIDTH = 1024 *4, SHADOW_HEIGHT = 1024 *4;
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// shader configuration
	// --------------------
	shader->use();
	shader->setInt("shadowMap", 8);
	debugDepthQuad->use();
	debugDepthQuad->setInt("depthMap", 0);


	setSamples();
	
	//Init KDTree
	glm::mat4 model = glm::mat4(1.0f);

	std::vector<Point> points = std::vector<Point>();

	for (int i = 0; i < worldObjects.size(); i++) {
		std::vector<Point> verts = worldObjects[i]->GetVertices(i, model);

		points.insert(points.end(), verts.begin(), verts.end());
	}

	kdTree = new KDTree(points);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		if (!b_edit)
		{
			float error = -1;
			for (char i = 0; i < 10 && abs(error) > 0.01f; ++i)
			{
				path.move(-error * speed);
				auto diff = path.GetSplinePointLocation() - camera.location;
				error = sqrtf(powf(diff.x, 2) + powf(diff.y, 2) + powf(diff.z, 2)) / speed * 60 - 1;
			}

			camera.location = path.GetSplinePointLocation();
			camera.rotation = path.GetSplinePointRotation();
		}

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 1. render depth of scene to texture (from light's perspective)
		// --------------------------------------------------------------
		glm::mat4 lightProjection, lightView;
		glm::mat4 lightSpaceMatrix;
		float near_plane = 1.0f, far_plane = 100.0f;
		lightProjection = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, near_plane, far_plane);
		lightView = glm::lookAt(lightPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;
		// render scene from light's point of view
		simpleDepthShader->use();
		simpleDepthShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
		
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_FRONT);
		renderScene(simpleDepthShader, true);
		glCullFace(GL_BACK);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		// reset viewport
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 2 render scene as normal using the generated depth/shadow map  
		// --------------------------------------------------------------
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader->use();
		glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		shader->setMat4("projection", projection);
		shader->setMat4("view", view);
		// set light uniforms
		shader->setVec3("viewPos", camera.location);
		shader->setVec3("lightDir", lightPos);
		shader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
		shader->setFloat("bumpiness", bumpiness);
		
		glActiveTexture(GL_TEXTURE8);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		renderScene(shader, false);

		//kd Tree
		glm::vec3 color = glm::vec3(0.8f, 1.0f, 0.0f);

		kdTreeShader->use();
		kdTreeShader->setMat4("projection", projection);
		kdTreeShader->setMat4("view", view);
		kdTreeShader->setVec3("color", color);
		kdTree->drawWireframe(kdTreeShader, model);

		//Ray

		/*
		if (fireRay) {
			color = glm::vec3(0.0f, 1.0f, 0.0f);
			kdTreeShader->setVec3("color", color);
			glLineWidth(5);
			rayCast();
		}*/

		// render Depth map to quad for visual debugging
		// ---------------------------------------------
		debugDepthQuad->use();
		debugDepthQuad->setFloat("near_plane", near_plane);
		debugDepthQuad->setFloat("far_plane", far_plane);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		//renderQuad();


		glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		
		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

// renders the 3D scene
// --------------------
void renderScene(const Shader * shader, bool lightning)
{
	glm::mat4 model = glm::mat4(1.0f);


	for (size_t i = 0; i < worldObjects.size(); i++)
	{
		worldObjects[i]->render(shader, model);
	}

	if(lightning) return;

	for (size_t i = 0; i < lights.size(); i++)
	{
		lights[i]->render(shader, model);
	}

}







// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

unsigned int rayVAO = 0;
unsigned int rayVBO;
void rayCast() {
	if (rayVAO == 0)
	{
		float rayVertices[] = {
			// positions        // texture Coords
			 0.0f,  0.0f, 0.0f,
			 max_RayLength,  0.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &rayVAO);
		glGenBuffers(1, &rayVBO);
		glBindVertexArray(rayVAO);
		glBindBuffer(GL_ARRAY_BUFFER, rayVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(rayVertices), &rayVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(rayVAO);
	glDrawArrays(GL_LINE_STRIP, 0, 2);
	glBindVertexArray(0);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		std::cout << "Ray\n";
		fireRay = true;
		
		while (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) glfwPollEvents();
	}

	if (glfwGetKey(window, GLFW_KEY_F4) == GLFW_PRESS)
	{
		b_edit = !b_edit;
		while (glfwGetKey(window, GLFW_KEY_F4) == GLFW_PRESS) glfwPollEvents();
	}
	
	if (b_edit)
	{
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			camera.location += camera.Forward() * (speed * 0.2f);
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			camera.location += camera.Left() * (speed * 0.2f);
		}

		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			camera.location -= camera.Forward() * (speed * 0.2f);
		}

		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			camera.location -= camera.Left() * (speed * 0.2f);
		}

		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		{
			camera.roll -= 0.015f;
		}

		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		{
			camera.roll += 0.015f;
		}

		if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)
		{
			camera.roll = 0.0f;
		}

		if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS)
		{
			path.next();
			while (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS) glfwPollEvents();
		}

		if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS)
		{
			path.prev();
			while (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS) glfwPollEvents();
		}
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		path.AddLocationWithRotation(camera.location, eulerAngles(camera.CorrectRotation()));
		while (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) glfwPollEvents();
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS)
		{
			bumpiness *= 1.05f;
		}

		if (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS)
		{
			bumpiness /= 1.05;
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS)
		{
			samples++;
			setSamples();
			while (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS) glfwPollEvents();
		}

		if (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS)
		{
			samples--;
			setSamples();
			while (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS) glfwPollEvents();
		}
	}
	else
	{
		if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS)
		{
			speed *= 1.02f;
		}

		if (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS)
		{
			speed /= 1.02;
		}		
	}

	if (glfwGetKey(window, GLFW_KEY_DELETE) == GLFW_PRESS)
	{
		path.removeLocation();
		path.removeRotation();
		while (glfwGetKey(window, GLFW_KEY_DELETE) == GLFW_PRESS) glfwPollEvents();
	}

	if (glfwGetKey(window, GLFW_KEY_KP_ENTER) == GLFW_PRESS)
	{
		path.print();
		while (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) glfwPollEvents();
	}
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	camera.rotation *= glm::quat({ (lastY - ypos) * 0.004f, 0, 0 });
	camera.rotation = glm::quat({ 0, (lastX - xpos) * 0.004f, 0 }) * camera.rotation;
	lastX = xpos;
	lastY = ypos;
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
//	camera.ProcessMouseScroll(yoffset);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
	unsigned int textureID=0;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

