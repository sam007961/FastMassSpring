#include "Renderer.h"
#include <glm/gtc/type_ptr.hpp>

Renderer::Renderer(Shader* shader) {
	this->shader = shader;
}

void Renderer::setRenderTarget(const render_target& target) {
	this->target = target;
}

void Renderer::setModelview(const glm::mat4& mv) {
	glUniformMatrix4fv(shader->h_uModelViewMatrix,
		1, GL_FALSE, glm::value_ptr(mv[0]));
}

void Renderer::setProjection(const glm::mat4& p) {
	glUniformMatrix4fv(shader->h_uProjectionMatrix,
		1, GL_FALSE, glm::value_ptr(p[0]));
}

PhongShadingRenderer::PhongShadingRenderer(Shader* shader) : Renderer::Renderer(shader) {}
PickShadingRenderer::PickShadingRenderer(Shader* shader) : Renderer::Renderer(shader) {}

void PickShadingRenderer::setTessFact(int n) {
	glUniform1i(((PickShader*)shader)->h_uTessFact, n);
}

void PhongShadingRenderer::draw(unsigned int n_elements) const{
	assert(target.vbo > 0 && target.nbo > 0 && target.ibo > 0);

	PhongShader* myShader = (PhongShader*)shader;

	glUseProgram(*myShader);
	glEnableVertexAttribArray(myShader->h_aPosition);
	glEnableVertexAttribArray(myShader->h_aNormal);

	glBindBuffer(GL_ARRAY_BUFFER, target.vbo);
	glVertexAttribPointer(myShader->h_aPosition,
		3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, target.nbo);
	glVertexAttribPointer(myShader->h_aNormal,
		3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, target.ibo);
	glDrawElements(GL_TRIANGLES, n_elements, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(myShader->h_aPosition);
	glDisableVertexAttribArray(myShader->h_aNormal);
}

void PickShadingRenderer::draw(unsigned int n_elements) const {
	assert(target.vbo > 0 && target.tbo > 0 && target.ibo > 0);

	PickShader* myShader = (PickShader*)shader;

	glUseProgram(*myShader);
	glEnableVertexAttribArray(myShader->h_aPosition);
	glEnableVertexAttribArray(myShader->h_aTexCoord);

	glBindBuffer(GL_ARRAY_BUFFER, target.vbo);
	glVertexAttribPointer(myShader->h_aPosition,
		3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, target.tbo);
	glVertexAttribPointer(myShader->h_aTexCoord,
		2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, target.ibo);
	glDrawElements(GL_TRIANGLES, n_elements, GL_UNSIGNED_INT, 0);

	glEnableVertexAttribArray(myShader->h_aPosition);
	glEnableVertexAttribArray(myShader->h_aTexCoord);
}