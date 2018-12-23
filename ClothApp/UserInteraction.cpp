#include "UserInteraction.h"
#include <GL/glew.h>
#include <iostream>
#include <cmath>

UserInteraction::UserInteraction(CgPointFixNode* fixer, float* vbuff, unsigned int n)
	: vbuff(vbuff), fixer(fixer),  n(n), i(-1) {}

int UserInteraction::colorToIndex(color c) {
	if (c[2] != 51) return -1;
	int vx = std::round((n - 1) * c[0] / 255.0);
	int vy = std::round((n - 1) * c[1] / 255.0);
	return n * vy + vx;
}

void UserInteraction::grabPoint(int mouse_x, int mouse_y){
	// read color
	color c(3);
	glReadPixels(mouse_x, mouse_y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &c[0]);
	i = colorToIndex(c);
	if (i != -1) fixer->fixPoint(i);
}

void UserInteraction::releasePoint() { if (i == -1) return; fixer->releasePoint(i); i = -1; }
void UserInteraction::movePoint(vec3 v) {
	if (i == -1) return;
	fixer->releasePoint(i);
	for(int j = 0; j < 3; j++)
		vbuff[3 * i + j] += v[j];
	fixer->fixPoint(i);
}