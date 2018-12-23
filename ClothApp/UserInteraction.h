#pragma once
#include "MassSpringSolver.h"
#include <glm/common.hpp>

class UserInteraction {
private:
	typedef glm::vec3 vec3;
	typedef std::vector<unsigned char> color;

	int n; // grid size
	int i; // index of fixed point
	float* vbuff; // vertex buffer
	CgPointFixNode* fixer; // point fixer
	int colorToIndex(color c);

public:
	UserInteraction(CgPointFixNode* fixer, float* vbuff, unsigned int n);

	void grabPoint(int mouse_x, int mouse_y); // grab point with color c
	void movePoint(vec3 v); // move grabbed point along mouse
	void releasePoint(); // release grabbed point;
};