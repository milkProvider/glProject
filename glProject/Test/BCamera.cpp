#include "BCamera.h"


BCamera::BCamera(void)
{
}


BCamera::~BCamera(void)
{
}

void BCamera::setViewMatrix(glm::mat4x4 pViewMatrix)
{
	_gl_ViewMatrix = pViewMatrix;
}

glm::mat4x4 BCamera::getViewMatrix()
{
	return _gl_ViewMatrix;
}
