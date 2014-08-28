#include "BGameObject.h"

BGameObject::BGameObject(FbxNode *pNode)
{
    FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();
	this->mesh = NULL;

    if (lNodeAttribute)
    {
        // All lights has been processed before the whole scene because they influence every geometry.
        if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMarker)
        {
           //DrawMarker(pGlobalPosition);
        }
        else if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eSkeleton)
        {
            //DrawSkeleton(pNode, pParentGlobalPosition, pGlobalPosition);
        }
        // NURBS and patch have been converted into triangluation meshes.
        else if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
        {
			this->mesh = new BMesh(pNode);
            //drawMeshDebug(pNode);
        }
        else if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eCamera)
        {
            //DrawCamera(pNode, pTime, pAnimLayer, pGlobalPosition);
        }
        else if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eNull)
        {
            //DrawNull(pGlobalPosition);
        }
    }
    else
    {
        // Draw a Null for nodes without attribute.
        //DrawNull(pGlobalPosition);
    }

	int lChildCount = pNode->GetChildCount();
	for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
		_childs.push_back(new BGameObject(pNode->GetChild(lChildIndex)));
}

BGameObject::BGameObject()
{
	this->mesh = NULL;
}

BGameObject * BGameObject::createRootScene()
{
	return new BGameObject();
}

void BGameObject::addChild(BGameObject * pGameObject)
{
	_childs.push_back(pGameObject);
}

void BGameObject::render()
{
	if(this->mesh != NULL)
		this->mesh->render();

	vector<BGameObject*>::iterator lIt;
	for(lIt = _childs.begin(); lIt != _childs.end(); ++lIt)
	{
		(*lIt)->render();
	}
}

void BGameObject::setProjectionMatrix(glm::mat4x4 pProjectionMatrix)
{
	_gl_ProjectionMatrix = pProjectionMatrix;
	if(mesh != NULL)
		mesh->setProjectionMatrix(pProjectionMatrix);
	//pass matrix to other childs
	vector<BGameObject*>::iterator lIt;
	for(lIt = _childs.begin(); lIt != _childs.end(); ++lIt)
	{
		(*lIt)->setProjectionMatrix(pProjectionMatrix);
	}
}

void BGameObject::setViewMatrix(glm::mat4x4 pViewMatrix)
{
	_gl_ViewMatrix = pViewMatrix;
	if(mesh != NULL)
		mesh->setViewMatrix(pViewMatrix);
	//pass matrix to other childs
	vector<BGameObject*>::iterator lIt;
	for(lIt = _childs.begin(); lIt != _childs.end(); ++lIt)
	{
		(*lIt)->setViewMatrix(pViewMatrix);
	}
}


BGameObject::~BGameObject()
{
    //dtor
}
