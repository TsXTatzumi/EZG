#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>


class Shader
{
public:
	// the program progHandle
	unsigned int progHandle;

	// constructor reads and builds the shader
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath, const GLchar* geometryPath = nullptr);
	Shader(const char* computePath);
	// use/activate the shader
	void use();
	// utility uniform functions
	void setBool(const std::string &name, bool value) const;
	void setInt(const std::string &name, int value) const;
	void setInts(const std::string& name, int* pointer, int size) const;
	void getInts(const std::string& name, int* pointer, int size) const;
	void setFloat(const std::string &name, float value) const;
	void setMat4(const std::string &name, glm::mat4 & value) const;
	void setVec3(const std::string& name, glm::vec3& value) const;
	void setVec4(const std::string& name, glm::vec4& value) const;

private:

	void checkCompileErrors(GLuint shader, std::string type = "PROGRAM");
};

#endif