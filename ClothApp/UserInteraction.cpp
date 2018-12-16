#include "UserInteraction.h"
#include <GL/glew.h>
#include <iostream>
#include <cmath>

UserInteraction::UserInteraction(float* vbuff, int n) 
	: vbuff(vbuff), fixer(vbuff, 3 * n * n),  n(n), i(-1) {}

int UserInteraction::colorToIndex(color c) {
	if (c[2] != 51) return -1;
	int vx = std::round((n - 1) * c[0] / 255.0);
	int vy = std::round((n - 1) * c[1] / 255.0);
	return 3 * n * vy + 3 * vx;
}

void UserInteraction::setHorizontalMotion(vec3 ux) { this->ux = ux; }
void UserInteraction::setVerticalMotion(vec3 uy) { this->uy = uy; }
void UserInteraction::setMotion(vec3 ux, vec3 uy) { this->ux = ux; this->uy = uy; }

void UserInteraction::grabPoint(int mouse_x, int mouse_y){
	// read color
	color c(3);
	glReadPixels(mouse_x, mouse_y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &c[0]);
	i = colorToIndex(c);
	fixer.addPoint(i);
}

void UserInteraction::releasePoint() { fixer.removePoint(i); i = -1; }
void UserInteraction::movePoint(vec3 v) {
	fixer.removePoint(i);
	for(int j = 0; j < 3; j++)
		vbuff[i + j] += v[j];
	fixer.addPoint(i);
}
void UserInteraction::fixPoints() { fixer.fixPoints(); }