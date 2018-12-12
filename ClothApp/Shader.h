#pragma once
#include <GL\glew.h>
#include <fstream>

struct PhongShader {
	GLuint _handle;
	GLint h_aPosition;
	GLint h_uModelViewMatrix;
	GLint h_uProjectionMatrix;
	//GLint h_aNormal;
	operator GLuint() const;
	PhongShader operator=(GLuint _handle);
};

struct PickShader {
	GLuint _handle;
	GLint h_aPosition; // Vertex Position
	//GLint h_aTexCoord; // Texture Coordinates
	//GLint h_uNumVerts; // Nuber of vertices on each edge
	operator GLuint() const;
	PickShader operator=(GLuint _handle);
};

void compile_shader(GLuint handle, const char* source);
void compile_shader(GLuint handle, std::ifstream& source);
void link_shader(GLuint program_handle, GLuint vshader_handle, GLuint fshader_handle);