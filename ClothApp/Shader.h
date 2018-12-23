#pragma once
#include <fstream>
#include <GL\glew.h>
#include <glm/common.hpp>

class NonCopyable {
public:
	NonCopyable() {}
	NonCopyable(const NonCopyable& other) {}
	NonCopyable& operator=(const NonCopyable& other) { return NonCopyable(); }
};

class GLShader : public NonCopyable {
private:
	GLuint handle; // Shader handle

public:
	GLShader(GLenum shaderType);
	/*GLShader(GLenum shaderType, const char* source);
	GLShader(GLenum shaderType, std::ifstream& source);*/
	void compile(const char* source);
	void compile(std::ifstream& source);
	operator GLuint() const; // cast to GLuint
	~GLShader();
};

class GLProgram : public NonCopyable {
protected:
	GLuint handle;
	GLuint uModelViewMatrix, uProjectionMatrix;
	void getCameraUniforms();
	void setUniformMat4(GLuint unif, glm::mat4 m);

public:
	GLProgram();
	virtual void link(const GLShader& vshader, const GLShader& fshader);
	operator GLuint() const; // cast to GLuint
	void setModelView(glm::mat4 m);
	void setProjection(glm::mat4 m);

	~GLProgram();
};

class ProgramInput : public NonCopyable {
private:
	GLuint handle; // vertex array object handle
	GLuint vbo[4]; // vertex buffer object handles | position, normal, texture, index
	void bufferData(unsigned int index, void* buff, size_t size);

public:
	ProgramInput();

	void setPositionData(float* buff, unsigned int len);
	void setNormalData(float* buff, unsigned int len);
	void setTextureData(float* buff, unsigned int len);
	void setIndexData(unsigned int* buff, unsigned int len);

	operator GLuint() const; // cast to GLuint

	~ProgramInput();
};

class PhongShader : public GLProgram {
public:
	PhongShader();
};


class PickShader : public GLProgram {
	GLuint uTessFact;

public:
	PickShader();
	virtual void link(const GLShader& vshader, const GLShader& fshader);
	void setTessFact(unsigned int n);
};