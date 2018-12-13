#pragma once
#include <GL\glew.h>
#include <fstream>
struct Shader {
	GLuint _handle; // Shader handle
	GLint h_aPosition; // 
	GLint h_uModelViewMatrix;
	GLint h_uProjectionMatrix;
	operator GLuint() const;
	Shader& operator=(GLuint _handle);
};

struct PhongShader : public Shader {
	GLint h_aNormal;
	PhongShader& operator=(GLuint _handle);
};

struct PickShader : public Shader {
	GLint h_aTexCoord; // Texture Coordinates
	GLint h_uTessFact; // Tesselation factor = n
	PickShader& operator=(GLuint _handle);
};

void compile_shader(GLuint handle, const char* source);
void compile_shader(GLuint handle, std::ifstream& source);
void link_shader(GLuint program_handle, GLuint vshader_handle, GLuint fshader_handle);