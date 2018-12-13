#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"
struct render_target {
	GLuint vbo, nbo, tbo, ibo;
};

class Renderer {
protected:
	Shader* shader;
	render_target target;

public:
	Renderer(Shader* shader);

	void setRenderTarget(const render_target& target);
	void setModelview(const glm::mat4& mv);
	void setProjection(const glm::mat4& p);

	virtual void draw(unsigned int n_elements) const = 0;
};

class PhongShadingRenderer : public Renderer {
public:
	PhongShadingRenderer(Shader* shader);

	virtual void draw(unsigned int n_elements) const;
};

class PickShadingRenderer : public Renderer {
public:
	PickShadingRenderer(Shader* shader);

	void setTessFact(int n);

	virtual void draw(unsigned int n_elements) const;
};