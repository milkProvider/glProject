#pragma once

#include <fbxsdk.h>
#include <vector>
#include <glm\mat4x4.hpp>
#include "BMesh.h"

using namespace std;

class BGameObject
{
    public:
        BMesh *mesh;
        BGameObject(FbxNode *pNode);
        BGameObject();
        virtual ~BGameObject();

		void render();

		//it has to be use only once to create root scene
		static BGameObject * createRootScene();

		//void setPosition(glm::vec3 pPos);
		//glm::vec3 getPosition();

		void setProjectionMatrix(glm::mat4x4 pProjectionMatrix);
		void setModelMatrix(glm::mat4x4 pProjectionMatrix);
		void setViewMatrix(glm::mat4x4 pProjectionMatrix);
		void addChild(BGameObject * pGameObject);
    protected:
    private:
		vector<BGameObject*> _childs;

		glm::mat4x4 _gl_ProjectionMatrix;
		glm::mat4x4 _gl_ModelMatrix;
		glm::mat4x4 _gl_ViewMatrix;
		glm::mat4x4 _gl_LocalMatrix;
};
