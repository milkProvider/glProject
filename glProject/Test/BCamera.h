#pragma once

#include <glm\mat4x4.hpp>

class BCamera
{
public:
	BCamera(void);
	~BCamera(void);
	void setViewMatrix(glm::mat4x4 pViewMatrix);
	glm::mat4x4 getViewMatrix();
private:
	glm::mat4x4 _gl_ViewMatrix;
};

