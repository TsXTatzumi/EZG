#include "Shader.hpp"

Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath)
{
	// 1. retrieve the vertex/fragment source code from filePath
	std::string vertexCode;
	std::string fragmentCode;
	std::string geometryCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	std::ifstream gShaderFile;
	// ensure ifstream objects can throw exceptions:
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		// open files
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;
		// read file's buffer contents into streams
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// close file handlers
		vShaderFile.close();
		fShaderFile.close();
		// convert stream into string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
		// if geometry shader path is present, also load a geometry shader
		if (geometryPath != nullptr)
		{
			gShaderFile.open(geometryPath);
			std::stringstream gShaderStream;
			gShaderStream << gShaderFile.rdbuf();
			gShaderFile.close();
			geometryCode = gShaderStream.str();
		}
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: = " << vertexPath << "   " << fragmentPath << std::endl;
	}
	
	progHandle = glCreateProgram();
	GLuint vertex, fragment, geometry;

	
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();

	// 2. compile shaders

	// vertex Shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	checkCompileErrors(vertex, "VERTEX");

	// fragment Shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	checkCompileErrors(fragment, "FRAGMENT");

	// if geometry shader is given, compile geometry shader
	if (geometryPath != nullptr)
	{
		const char* gShaderCode = geometryCode.c_str();
		geometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry, 1, &gShaderCode, NULL);
		glCompileShader(geometry);
		checkCompileErrors(geometry, "GEOMETRY");
	}
	
	// shader Program
	glAttachShader(progHandle, vertex);
	glAttachShader(progHandle, fragment);
	if (geometryPath != nullptr)
		glAttachShader(progHandle, geometry);
	
	glLinkProgram(progHandle);
	checkCompileErrors(progHandle);
	// delete the shaders as they're linked into our program now and no longer necessery
	glDetachShader(progHandle, vertex);
	glDetachShader(progHandle, fragment);
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	if (geometryPath != nullptr)
	{
		glDetachShader(progHandle, geometry);
		glDeleteShader(geometry);
	}
}

Shader::Shader(const char* computePath)
{
	// 1. retrieve the compute source code from filePath
	std::string computeCode;
	std::ifstream cShaderFile;
	// ensure ifstream object can throw exceptions:
	cShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		// open file
		cShaderFile.open(computePath);
		std::stringstream cShaderStream;
		// read file's buffer content into streams
		cShaderStream << cShaderFile.rdbuf();
		// close file handler
		cShaderFile.close();
		// convert stream into string
		computeCode = cShaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}
	// Creating the compute shader, and the program object containing the shader

	progHandle = glCreateProgram();
	GLuint compute = glCreateShader(GL_COMPUTE_SHADER);

	const char* computeSource = computeCode.c_str();
	
	glShaderSource(compute, 1, &computeSource, NULL);
	glCompileShader(compute);
	checkCompileErrors(compute, "COMPUTE");
	
	glAttachShader(progHandle, compute);

	glLinkProgram(progHandle);
	checkCompileErrors(progHandle);
	
	// delete the shaders as they're linked into our program now and no longer necessery
	glDetachShader(progHandle, compute);
	glDeleteShader(compute);
}

void Shader::use()
{
	glUseProgram(progHandle);
}

void Shader::setBool(const std::string &name, bool value) const
{
	glUniform1i(glGetUniformLocation(progHandle, name.c_str()), (int)value);
}
void Shader::setInt(const std::string &name, int value) const
{
	glUniform1i(glGetUniformLocation(progHandle, name.c_str()), value);
}
void Shader::setInts(const std::string& name, int * pointer, int size) const
{
	glUniform1iv(glGetUniformLocation(progHandle, name.c_str()), size, pointer);
}
void Shader::getInts(const std::string& name, int* pointer, int size) const
{
	glGetnUniformiv(progHandle, glGetUniformLocation(progHandle, name.c_str()), size, pointer);
}
void Shader::setFloat(const std::string &name, float value) const
{
	glUniform1f(glGetUniformLocation(progHandle, name.c_str()), value);
}
void Shader::setMat4(const std::string &name, glm::mat4 &value) const
{
	glUniformMatrix4fv(glGetUniformLocation(progHandle, name.c_str()), 1, GL_FALSE, &value[0][0]);
}

void Shader::setVec2(const std::string& name, glm::vec2& value) const
{
	glUniform2fv(glGetUniformLocation(progHandle, name.c_str()), 1, &value[0]);
}

void Shader::setVec3(const std::string& name, glm::vec3& value) const
{
	glUniform3fv(glGetUniformLocation(progHandle, name.c_str()), 1, &value[0]);
}

void Shader::setVec4(const std::string& name, glm::vec4& value) const
{
	glUniform4fv(glGetUniformLocation(progHandle, name.c_str()), 1, &value[0]);
}

// utility function for checking shader compilation/linking errors.
// ------------------------------------------------------------------------
void Shader::checkCompileErrors(GLuint shader, std::string type)
{
	GLint success;
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			fprintf(stderr, "Error in compiling the %s shader\n", type.c_str());
			GLchar log[10240];
			GLsizei length;
			glGetShaderInfoLog(shader, 10239, &length, log);
			fprintf(stderr, "Compiler log:\n%s\n", log);
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			fprintf(stderr, "Error in linking the shader program\n");
			GLchar log[10240];
			GLsizei length;
			glGetProgramInfoLog(shader, 10239, &length, log);
			fprintf(stderr, "Linker log:\n%s\n", log);
		}
	}
}
