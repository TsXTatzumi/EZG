 
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
#include "Chunk.h"
#include "ParticleSpawn.h"

void setSamples();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
void renderScene();

void renderQuad();

GLuint framebuffer;
GLuint samples = 1;

// settings
unsigned int SCR_WIDTH = 720;
unsigned int SCR_HEIGHT = 500;

// camera
Camera camera({0.0f, 0.0f, 3.0f } , {0.0f, 0.0f, 0.0f});
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;
float speed = 0.5;

bool b_edit = true;

// world
const unsigned int chunksize = 32;
const unsigned int num_chunks = 7;
Chunk* chunks[num_chunks][1];

std::vector<ParticleSpawn*> particleSpawns;
float particle_update_time = 0;
int particle_update_rate = 20;

int steps = 16;
int finesteps = 32;

float deltaTime, lastFrame;

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


Shader* particleShader;
Shader* particleUShader;

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
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
	glfwSetMouseButtonCallback(window, mouse_button_callback);
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
	glEnable(GL_DEBUG_OUTPUT);

	// build and compile shaders
	// -------------------------
	Shader * generateShader = new Shader("GenerateChunk_CS.glsl");
	Shader* chunkShader = new Shader("Chunk_VS.glsl", "Chunk_FS.glsl" , "Chunk_GS.glsl");
	Shader* rayShader = new Shader("ChunkRay_CS.glsl");

	particleShader = new Shader("Particle_VS.glsl", "Particle_FS.glsl", "Particle_GS.glsl");
	particleUShader = new Shader("Particle_CS.glsl");
	
	//Shader * debugDepthQuad = new Shader("Debug_VS.glsl", "Debug_FS.glsl");

	chunkShader->use();

	chunkShader->setInt("steps", steps);
	chunkShader->setInt("fine_steps", finesteps);
	
	// shader configuration
	// --------------------

	setSamples();

	//
	GLuint Xtex = loadTexture("Z.jpg");
	GLuint Ytex = loadTexture("Y.jpg");
	GLuint Ztex = loadTexture("Z.jpg");
	
	Chunk::Init(chunksize, generateShader, chunkShader, rayShader, 12345, Xtex, Ytex, Ztex);
	for (unsigned int x = 0; x < num_chunks; ++x)
	{
			chunks[x][0] = new Chunk();
			chunks[x][0]->setupData(x * chunksize, 0 * chunksize);
	}

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glm::vec3 chunkPos = camera.location / glm::vec3(chunksize);
		for (unsigned int x = 0; x < num_chunks; ++x)
		{

				glm::vec3 dist = chunks[x][0]->location - camera.location;
				
				if ( dist.x / chunksize > (-(int)num_chunks) / 2 + (int)num_chunks) 
					chunks[x][0] ->relocate(chunks[x][0]->location.x - num_chunks * chunksize, chunks[x][0]->location.y);
				
				if (-dist.x / chunksize > (-(int)num_chunks) / 2 + (int)num_chunks) 
					chunks[x][0]->relocate(chunks[x][0]->location.x + num_chunks * chunksize, chunks[x][0]->location.y);
				
		}

		particle_update_time += deltaTime;
		if (particle_update_time > 1.0f / particle_update_rate)
		{
			for (std::vector<ParticleSpawn*>::iterator particleSpawn = particleSpawns.begin(); particleSpawn != particleSpawns.end(); )
			{
				int offset = particleSpawn - particleSpawns.begin();
				if ((*particleSpawn)->Dead()) {
					particleSpawns.erase(particleSpawn);
					particleSpawn = particleSpawns.begin() + offset;
				}
				else
				{
					(*particleSpawn)->Update(particle_update_time);
					particleSpawn++;
				}
			}
			
			particle_update_time = 0;
		}

		
		/*if (!b_edit)
		{
			camera.location.x += deltaTime * speed;
			camera.location.y = 50;
			camera.location.z = 16;
		}*/

		// input
		// -----
		processInput(window);

		// render
		// ------
	
		// reset viewport
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);		
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 2 render scene as normal using the generated depth/shadow map  
		// --------------------------------------------------------------
		
		glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
		glm::mat4 view = camera.GetViewMatrix();

		chunkShader->use();
		
		chunkShader->setMat4("projection", projection);
		chunkShader->setMat4("view", view);
		chunkShader->setVec3("camera_position", camera.location);

		particleShader->use();

		particleShader->setMat4("projection", projection);
		particleShader->setMat4("view", view);
		
		renderScene();
		
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
void renderScene()
{
	glm::mat4 model = glm::mat4(1.0f);

	for (unsigned int x = 0; x < num_chunks; ++x)
	{
			chunks[x][0]->render(model);
	}

	for (std::vector<ParticleSpawn*>::value_type particle_spawn : particleSpawns)
	{
		particle_spawn->Draw();
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

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_F4) == GLFW_PRESS)
	{
		if ((b_edit = !b_edit)) {
			glfwSetCursorPosCallback(window, mouse_callback);
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else {
			glfwSetCursorPosCallback(window, nullptr);
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		
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
	}

	if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS)
	{
		Chunk::toggleWireframe();
		while (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS) glfwPollEvents();
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
	{
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		{
			if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS)
			{
				finesteps++;
				Chunk::setSteps(steps, finesteps);
				
				while (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS) glfwPollEvents();
			}

			if (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS)
			{
				finesteps = glm::max(0, finesteps - 1);
				Chunk::setSteps(steps, finesteps);
				
				while (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS) glfwPollEvents();
			}
		}
		else
		{
			if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS)
			{
				steps++;
				Chunk::setSteps(steps, finesteps);

				while (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS) glfwPollEvents();
			}

			if (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS)
			{
				steps = glm::max(0, steps - 1);
				Chunk::setSteps(steps, finesteps);

				while (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS) glfwPollEvents();
			}
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
	else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS)
		{
			particle_update_rate++;
			while (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS) glfwPollEvents();
		}

		if (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS)
		{
			particle_update_rate = glm::max(--particle_update_rate, 1);
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

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
	
	lastX = (float)SCR_WIDTH / 2.0;
	lastY = (float)SCR_HEIGHT / 2.0;

	setSamples();
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

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	double xpos, ypos;
	//getting cursor position
	glfwGetCursorPos(window, &xpos, &ypos);
	
	glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
	glm::mat4 view = camera.GetViewMatrix();

	glm::vec2 normaizedMouse = glm::vec2(xpos / SCR_WIDTH * 2 - 1, -(ypos / SCR_HEIGHT * 2 - 1));
	glm::vec4 clipCoords = glm::vec4(normaizedMouse, -1, 1);
	glm::vec4 eyeCoords = glm::vec4((inverse(projection) * clipCoords).x, (inverse(projection) * clipCoords).y, -1, 0);
	glm::vec3 worldDir = glm::vec3(glm::inverse(view) * eyeCoords);
	
	Ray ray(camera.location, camera.location + worldDir);
	
	glm::vec3 pos, norm;
	for (unsigned int x = 0; x < num_chunks; ++x)
	{
		if(chunks[x][0]->ray(ray, pos, norm))
		{
			std::cout << "Hit!\n";
			particleSpawns.push_back(new ParticleSpawn(particleShader, pos, norm, 60.0f, 300, particleUShader));
		}
	}
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

