#pragma once

#include <fbxsdk.h>
#include <gl/glew.h>
#include <gl/GL.h>
#include "BShader.h"
#include <glm\mat4x4.hpp>
#include <vector>
#include <map>

struct BVertexData;

class BMesh
{
private:
	int ATT_POS;				//0
	int ATT_UV;					//1
	int ATT_NORMAL;				//2

	int ATT_BONE_IDS;			//3
	int ATT_BONE_WEIGHT;		//4


	int MAX_BONES_PER_VERTEX;	//4

	GLuint attributes[3];
	GLuint _vboIndices;
	GLuint _vao;

	int _verticesCount;
	int _indicesCount;
	
	//Rendering
	BShader *_shader;
	void createShader();
	glm::mat4x4 _gl_ProjectionMatrix;
	glm::mat4x4 _gl_ViewMatrix;
	glm::mat4x4 _gl_ModelMatrix;
	
	//Animations
	int frame;
	std::vector<FbxNode *> _bones;
	float _anyTime;
	FbxTime _animTime;
	//return pBoneIds and pBoneWeights
	void loadAnimation(FbxNode * pNode, BVertexData * pVertexData);
	std::map<int, int> _indexTranslation;
	//TODO: tmp
	GLuint _tex;

	//tmp
	FbxSkin * _skinDeformer;
public:
	FbxNode * node;
	BMesh(FbxNode * pNode);
	~BMesh(void);

	void init();

	//rendering
	void render();
	void drawMeshDebug(FbxNode * pNode);
	void drawNodeDebug(FbxNode * pNode);
	void drawRecursiveDebug(FbxNode * pNode);

	//Geometry
	void setProjectionMatrix(glm::mat4x4 pProjectionMatrix);
	void setViewMatrix(glm::mat4x4 pViewMatrix);

	//Animation
	void updateAnimation();
};

