#pragma once
#include "MassSpringSolver.h"
#include <glm/common.hpp>

class UserInteraction {
private:
	typedef glm::vec3 vec3;
	typedef std::vector<char> color;

	int n; // grid size
	int i; // index of fixed point
	vec3 ux, uy; // unit motion direction
	PointFixer fixer; // point fixer
	int colorToIndex(color c);

public:
	UserInteraction(float* vbuff, int n);

	void setHorizontalMotion(vec3 ux);
	void setVerticalMotion(vec3 uy);
	void setMotion(vec3 ux, vec3 uy);

	void grabPoint(int mouse_x, int mouse_y); // grab point with color c
	//void movePoint(int dx, int dy); // move grabbed point along mouse
	void releasePoint(); // release grabbed point;
	
	//void fixPoints();

};