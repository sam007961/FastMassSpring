#include "UserInteraction.h"
#include <GL/glew.h>
#include <iostream>

UserInteraction::UserInteraction(float* vbuff, int n) : fixer(vbuff, 3 * n * n),  n(n), i(-1) {}

int UserInteraction::colorToIndex(color c) {
	if (c[2] != 51) return -1;
	return n * (n - 1) * (c[0]/ 255) + (n - 1) * c[1] / 255;
}

void UserInteraction::setHorizontalMotion(vec3 ux) { this->ux = ux; }
void UserInteraction::setVerticalMotion(vec3 uy) { this->uy = uy; }
void UserInteraction::setMotion(vec3 ux, vec3 uy) { this->ux = ux; this->uy = uy; }

void UserInteraction::grabPoint(int mouse_x, int mouse_y){
	color c(3);
	glReadPixels(mouse_x, mouse_y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &c[0]);
	i = colorToIndex(c);
	// debug
	std::cout << "---------------------------" << std::endl;
	std::cout << mouse_x << " " << mouse_y << std::endl;
	std::cout << (int) c[0] << " " << (int) c[1] << " " << (int) c[2] << std::endl;
	std::cout << i / 3 << " " << i % 3 << std::endl;
}

void UserInteraction::releasePoint() {}