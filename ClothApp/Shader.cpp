#include "Shader.h"
#include <vector>
#include <iostream>

PhongShader::operator GLuint() const {
	return _handle;
}

PhongShader PhongShader::operator=(GLuint _handle) {
	this->_handle = _handle;
	return *this;
}

PickShader::operator GLuint() const {
	return _handle;
}

PickShader PickShader::operator=(GLuint _handle) {
	this->_handle = _handle;
	return *this;
}

void compile_shader(GLuint handle, std::ifstream& source) {
	std::vector<char> text;
	source.seekg(0, std::ios_base::end);
	std::streampos fileSize = source.tellg();
	text.resize(fileSize);

	source.seekg(0, std::ios_base::beg);
	source.read(&text[0], fileSize);

	compile_shader(handle, &text[0]);
}

void compile_shader(GLuint handle, const char* source) {

	GLint compiled = 0;  // Compiled flag
	const char *ptrs[] = { source };
	const GLint lens[] = { std::strlen(source) };
	glShaderSource(handle, 1, ptrs, lens);
	glCompileShader(handle);
	glGetShaderiv(handle, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		GLint logSize = 0;
		glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logSize);
		std::vector<GLchar> errorLog(logSize);
		glGetShaderInfoLog(handle, logSize, &logSize, &errorLog[0]);
		std::cerr << &errorLog[0] << std::endl;
		throw std::runtime_error("Failed to compile shader.");
	}
}

void link_shader(GLuint program_handle, GLuint vshader_handle, GLuint fshader_handle) {
	GLint linked = 0; // Linked flag
	glAttachShader(program_handle, vshader_handle);
	glAttachShader(program_handle, fshader_handle);
	glLinkProgram(program_handle);
	glDetachShader(program_handle, vshader_handle);
	glDetachShader(program_handle, fshader_handle);
	glGetProgramiv(program_handle, GL_LINK_STATUS, &linked);
	if (!linked)
		throw std::runtime_error("Failed to link shaders.");
}