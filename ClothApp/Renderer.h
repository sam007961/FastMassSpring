#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"


class Renderer {
protected:
	GLProgram* program;
	ProgramInput* input;

public:
	Renderer();

	void setProgram(GLProgram* program);
	void setProgramInput(ProgramInput* input);
	void setModelview(const glm::mat4& mv);
	void setProjection(const glm::mat4& p);

	void draw(unsigned int n_elements);
};