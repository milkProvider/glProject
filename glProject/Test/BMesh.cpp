#include "BMesh.h"
#include <gl/glew.h>
#include <gl/GL.h>
#include "SOIL.h"

BMesh::BMesh(FbxNode * pNode)
{
	node = pNode;
	_gl_ModelMatrix = glm::mat4(1.0f);
	ATT_POS = 0;
	ATT_UV = 1;
	ATT_NORMAL = 2;
	ATT_BONE_IDS = 3;
	ATT_BONE_WEIGHT = 4;
	MAX_BONES_PER_VERTEX = 4;
	_anyTime = 0.0f;
	_animTime = 0.0;
	frame = 0;
	init();
}


BMesh::~BMesh(void)
{

}

//64 bytes
struct BVertexData
{ 
	float position[3];	//12
	float uv[2];		//8
	float normal[3];	//12
	int boneIds[4];		//16
	float boneWeights[4];//16
};

void BMesh::init()
{
	FbxMesh* lMesh = node->GetMesh();
	FbxVector4 * lControlPoints = lMesh->GetControlPoints();
	_verticesCount = lMesh->GetControlPointsCount();
	int * lIndexes = lMesh->GetPolygonVertices();
	_indicesCount = lMesh->GetPolygonVertexCount();

	float * lUvs = new float[_verticesCount * 2];
	float * lNormals = new float[_verticesCount * 3];


	printf("Amount of layers: %i", lMesh->GetLayerCount());
	
	FbxLayerElementNormal * lNormalFbx = lMesh->GetLayer( 0 )->GetNormals( );
	FbxLayerElementUV * lUvFbx = lMesh->GetLayer( 0 )->GetUVs( );
	
	BVertexData * lVertexData = new BVertexData[_verticesCount];
	for(int i = 0; i < _verticesCount; i++)
	{
		BVertexData lData;
		FbxVector4 lA = lControlPoints[i].mData;
		for(int k = 0; k < 3; ++k)
			lData.position[k] = (float) lA.mData[k];
		for(int d = 0; d < 4; ++d)
		{
			lData.boneIds[d] = -1;
			lData.boneWeights[d] = 0.0f;
		}

		if(lUvFbx != NULL)
		{
			for(int k = 0; k < 2; ++k)
				lData.uv[k] = lUvFbx->GetDirectArray().GetAt(i).mData[k];
		}
		else
		{
			for(int k = 0; k < 2; ++k)
				lData.uv[k] = 0.0f;
		}

		if(lNormalFbx != NULL)
		{
			for(int k = 0; k < 3; ++k)
				lData.normal[k] = lNormalFbx->GetDirectArray().GetAt(i).mData[k];
		}
		else
		{
			for(int k = 0; k < 3; ++k)
				lData.normal[k] = 1.0f;
		}

		lVertexData[i] = lData;
	}

	loadAnimation(node, lVertexData);

	_vao = 0;
	glGenVertexArrays (1, &_vao);
	glBindVertexArray (_vao);

	//Vertices
	attributes[ATT_POS] = 0;
	glGenBuffers(1, &attributes[0]);
	glBindBuffer(GL_ARRAY_BUFFER, attributes[ATT_POS]);

	//pos
	glBufferData(GL_ARRAY_BUFFER, _verticesCount  * sizeof(BVertexData), lVertexData, GL_STATIC_DRAW);
	glVertexAttribPointer (ATT_POS, 3, GL_FLOAT, GL_FALSE, sizeof(BVertexData), (void*)0);
	glEnableVertexAttribArray (ATT_POS);

	//uvs - 8 bytes
	glVertexAttribPointer (ATT_UV, 2, GL_FLOAT, GL_FALSE, sizeof(BVertexData), (void*)12);
	glEnableVertexAttribArray (ATT_UV);

	//normals - 12 bytes
	glVertexAttribPointer (ATT_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(BVertexData), (void*)20);
	glEnableVertexAttribArray (ATT_NORMAL);

	//Bone ids
	glVertexAttribIPointer  (ATT_BONE_IDS, 4, GL_INT, sizeof(BVertexData), (void*) 32);
	glEnableVertexAttribArray (ATT_BONE_IDS);

	//Bone weights
	glVertexAttribPointer (ATT_BONE_WEIGHT, 4, GL_FLOAT, GL_FALSE, sizeof(BVertexData), (void*) 48);
	glEnableVertexAttribArray (ATT_BONE_WEIGHT);


	glGenBuffers(1, &_vboIndices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vboIndices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indicesCount * sizeof(int), lIndexes, GL_STATIC_DRAW);
	//glBindTexture(GL_TEXTURE_2D, pNewModel->mMaterial.texture1ID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//lIndexes = NULL;
	
	_tex = SOIL_load_OGL_texture("tex.jpg",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_INVERT_Y);
	if(_tex == 0)
		printf("TEXTURE IS NOT SET PROPERLY");
	glBindTexture(GL_TEXTURE_2D, _tex);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	//glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, _tex);
	
    
	createShader();
	delete [] lVertexData;
}

void BMesh::loadAnimation(FbxNode * pNode, BVertexData * pVertexData)
{
	FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();

    // NURBS and patch have been converted into triangluation meshes.
	if (lNodeAttribute->GetAttributeType() != FbxNodeAttribute::eMesh)
		return;

	FbxMesh * pMesh = pNode->GetMesh();
	int lVertexCount = pMesh->GetControlPointsCount();
	int lSkinCount = pMesh->GetDeformerCount(FbxDeformer::eSkin);

	//clusters
	//FbxDualQuaternion* lDQClusterDeformation = new FbxDualQuaternion[lVertexCount];
	//memset(lDQClusterDeformation, 0, lVertexCount * sizeof(FbxDualQuaternion));

	//float * lVaoWeights = pBoneWeights;//new float[_verticesCount * MAX_BONES_PER_VERTEX];
	//memset(pBoneWeights, 0, _verticesCount * sizeof(float) * MAX_BONES_PER_VERTEX);
	//this->attributes[ATT_BONE_WEIGHT]

	//int * lVaoIds = pBoneIndexes;//new int[_verticesCount * MAX_BONES_PER_VERTEX];
	//memset(pBoneIndexes, -1, _verticesCount * sizeof(int) * MAX_BONES_PER_VERTEX);
	int boneNum = 0;

	std::map<int, int>::iterator lIndexTranslationIt;
	

	for ( int lSkinIndex=0; lSkinIndex<lSkinCount; ++lSkinIndex)
	{
		FbxSkin * lSkinDeformer = (FbxSkin *)pMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin);
		int lClusterCount = lSkinDeformer->GetClusterCount();

		for ( int lClusterIndex=0; lClusterIndex<lClusterCount; ++lClusterIndex)
		{
			FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);
			if (!lCluster->GetLink())
				continue;
			
			FbxNode* pBoneNode = lCluster->GetLink();
			int lUniqueId = pBoneNode->GetUniqueID();
			lIndexTranslationIt = _indexTranslation.find(lUniqueId);
			if(lIndexTranslationIt == _indexTranslation.end())
			{
				_indexTranslation.insert(std::pair<int,int>(lUniqueId, _bones.size()));
				_bones.push_back(pBoneNode);
			}

			int lRealIndex = _indexTranslation[lUniqueId];
			//pBoneNode->EvaluateLocalTransform(
			//pBoneNode->EvaluateLocalTransform(0.0);
			//FbxAMatrix BoneInitPoseGlobalTransform = pBoneRootNode->EvaluateGlobalTransform()
			
			int lVertexIndexCount = lCluster->GetControlPointIndicesCount();
			for (int k = 0; k < lVertexIndexCount; ++k) 
			{ 
				int lIndex = lCluster->GetControlPointIndices()[k];

				// Sometimes, the mesh can have less points than at the time of the skinning
				// because a smooth operator was active when skinning but has been deactivated during export.
				if (lIndex >= lVertexCount)
					continue;

				double lWeight = lCluster->GetControlPointWeights()[k];

				if (lWeight == 0.0)
					continue;

				
				int lInternalIndex = 0;//lVaoIds[MAX_BONES_PER_VERTEX * lIndex] == 0.0f ? 0 : 1;
				while(pVertexData[lIndex].boneIds[lInternalIndex] > -1 && pVertexData[lIndex].boneIds[lInternalIndex] != lRealIndex)
				{
					lInternalIndex++;
					if(lInternalIndex > 3)
						break;
				}

				if(lInternalIndex > 3)
				{
					printf("Some vertices have more than 4 bones per vertex\n");
					continue;
				}
				
				pVertexData[lIndex].boneIds[lInternalIndex] = lRealIndex;
				pVertexData[lIndex].boneWeights[lInternalIndex] += (float)lWeight;
				//printf("i: %i, ii%i, weight: %f\n", lRealIndex, lInternalIndex, pVertexData[lIndex].boneWeights[lInternalIndex]);
			}
			boneNum++;
		}

		//printf("Bones count: " + _bones.size());
	}

	//lVaoWeights
	//lVaoIds

	// For all skins and all clusters, accumulate their deformation and weight
	// on each vertices and store them in lClusterDeformation and lClusterWeight.
	//for ( int lSkinIndex=0; lSkinIndex<lSkinCount; ++lSkinIndex)
	/*
	{
		FbxSkin * lSkinDeformer = (FbxSkin *)pMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin);
		int lClusterCount = lSkinDeformer->GetClusterCount();
		for ( int lClusterIndex=0; lClusterIndex<lClusterCount; ++lClusterIndex)
		{
			FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);
			if (!lCluster->GetLink())
				continue;

			
			FbxAMatrix lVertexTransformMatrix;
			ComputeClusterDeformation(pGlobalPosition, pMesh, lCluster, lVertexTransformMatrix, pTime, pPose);

			FbxQuaternion lQ = lVertexTransformMatrix.GetQ();
			FbxVector4 lT = lVertexTransformMatrix.GetT();
			FbxDualQuaternion lDualQuaternion(lQ, lT);

			int lVertexIndexCount = lCluster->GetControlPointIndicesCount();
			for (int k = 0; k < lVertexIndexCount; ++k) 
			{ 
				int lIndex = lCluster->GetControlPointIndices()[k];

				// Sometimes, the mesh can have less points than at the time of the skinning
				// because a smooth operator was active when skinning but has been deactivated during export.
				if (lIndex >= lVertexCount)
					continue;

				double lWeight = lCluster->GetControlPointWeights()[k];

				if (lWeight == 0.0)
					continue;

				int lInternalIndex = 0;//lVaoIds[MAX_BONES_PER_VERTEX * lIndex] == 0.0f ? 0 : 1;
				while(lVaoIds[MAX_BONES_PER_VERTEX * lIndex + lInternalIndex] == 0.0f)
				{
					lInternalIndex++;
					if(lInternalIndex > 3)
						break;
				}

				if(lInternalIndex > 3)
				{
					printf("Some vertices have more than 4 bones per vertex\n");
					continue;
				}

				lVaoIds[MAX_BONES_PER_VERTEX * lIndex + lInternalIndex] = lIndex;
				lVaoWeights[MAX_BONES_PER_VERTEX * lIndex + lInternalIndex] += lWeight;
				
				// Compute the influence of the link on the vertex.
				FbxDualQuaternion lInfluence = lDualQuaternion * lWeight;
				if (lClusterMode == FbxCluster::eAdditive)
				{    
					// Simply influenced by the dual quaternion.
					lDQClusterDeformation[lIndex] = lInfluence;
					

					// Set the link to 1.0 just to know this vertex is influenced by a link.
					lClusterWeight[lIndex] = 1.0;
				}
				else // lLinkMode == FbxCluster::eNormalize || lLinkMode == FbxCluster::eTotalOne
				{
					if(lClusterIndex == 0)
					{
						lDQClusterDeformation[lIndex] = lInfluence;
					}
					else
					{
						// Add to the sum of the deformations on the vertex.
						// Make sure the deformation is accumulated in the same rotation direction. 
						// Use dot product to judge the sign.
						double lSign = lDQClusterDeformation[lIndex].GetFirstQuaternion().DotProduct(lDualQuaternion.GetFirstQuaternion());
						if( lSign >= 0.0 )
						{
							lDQClusterDeformation[lIndex] += lInfluence;
						}
						else
						{
							lDQClusterDeformation[lIndex] -= lInfluence;
						}
					}
					// Add to the sum of weights to either normalize or complete the vertex.
					lClusterWeight[lIndex] += lWeight;
					
				}
				
			}//For each vertex
			
		}//lClusterCount
		*/
}

void BMesh::createShader()
{
	_shader = new BShader("C:/Users/milkProvider/Documents/Visual Studio 2010/Projects/Test/Test/shader.vert", "C:/Users/milkProvider/Documents/Visual Studio 2010/Projects/Test/Test/shader.frag");
}

/*
void _print_shader_info_log (unsigned int shader_index) {
  int max_length = 2048;
  int actual_length = 0;
  char log[2048];
  glGetShaderInfoLog (shader_index, max_length, &actual_length, log);
  printf ("shader info log for GL index %i:\n%s\n", shader_index, log);
}
*/

void BMesh::render()
{
	_shader->bind();
	//updateAnimation();
	
	glBindVertexArray(_vao);

	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _tex);


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vboIndices);
	glDrawElements(GL_TRIANGLES, _indicesCount, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	_shader->unbind();
	
	glBegin(GL_TRIANGLES);

	glEnd();
}

glm::mat4x4 toMat4x4(FbxAMatrix pMatrix)
{
	glm::mat4x4 lMatrix;
	for(int i = 0; i < 4; ++i)
	{
		for(int k = 0; k < 4; ++k)
		{
			lMatrix[i][k] = pMatrix.mData[i][k];
		}
	}
	return lMatrix;
}

void BMesh::updateAnimation()
{
	// Compute the shift of the link relative to the reference.
    //lVertexTransformMatrix = RGCP.Inverse() * CGCP * CGIP.Inverse() * RGIP * RG;
    // CGCP = position of bone
    // RGCP = mesh position
	int lSkinCount = 0;
	FbxMesh * pMesh = node->GetMesh();
	float lAnimTime = 0.0f;

	for ( int lSkinIndex=0; lSkinIndex<lSkinCount; ++lSkinIndex)
	{
		FbxSkin * lSkinDeformer = (FbxSkin *)pMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin);
		int lClusterCount = lSkinDeformer->GetClusterCount();

		for ( int lClusterIndex=0; lClusterIndex<lClusterCount; ++lClusterIndex)
		{
			FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);
			if (!lCluster->GetLink())
				continue;
			
			FbxNode* pBoneNode = lCluster->GetLink();
			int lUniqueId = pBoneNode->GetUniqueID();
			int lRealIndex = _indexTranslation[lUniqueId];

//			FbxNode * lBone = _bones.at(lRealIndex);

			FbxAMatrix lLocalMatrix = pBoneNode->EvaluateLocalTransform( lAnimTime );
			FbxVector4 scale = lLocalMatrix.GetS();
			FbxVector4 trans = lLocalMatrix.GetT();
			FbxQuaternion quat = lLocalMatrix.GetQ();

			//TODO
			//_shader->setBoneMatrix(toMat4x4(trans * quat * scale));
			//pBoneNode == lBone
			//local transformation matrix of the bone at current time
			
			

			//FbxAMatrix BoneInitPoseGlobalTransform = pBoneRootNode->EvaluateGlobalTransform()
		}
	}
	/*
	std::vector<FbxNode*>::iterator lIt;
	glm::mat4x4 lAnimMatrix[61];
	int lIndex = 0;

	//_animTime += 1000.0f;
	_animTime.SetTime(0, 0, 0, frame);
	frame++;
	frame = frame % 200;

	//pCluster->GetTransformLinkMatrix(BoneBindPoseMatrix);
	for(lIt = _bones.begin(); lIt != _bones.end(); ++lIt)
	{
		FbxAMatrix lMatrix = (*lIt)->EvaluateLocalTransform(_animTime);
		lAnimMatrix[lIndex] =  toMat4x4(lMatrix);
		lIndex++;
		if(lIndex == 61)
		{
			printf("more than 61 bones: %d\n", _bones.size());
			break;
		}
	}
	_shader->setBoneMatrix(lAnimMatrix);


	//pVertexTransformMatrix = lReferenceGlobalInitPosition.Inverse() * lAssociateGlobalInitPosition * lAssociateGlobalCurrentPosition.Inverse() *
	//	lClusterGlobalCurrentPosition * lClusterGlobalInitPosition.Inverse() * lReferenceGlobalInitPosition;
	//This lFbxLinkMatrix is the skeleton's transform when the binding happened. 
    //It is the same as the matrix in bindpose if the bindpose is complete.
    // Multiply lClusterGlobalInitPosition by Geometric Transformation
    FbxAMatrix clusterGlobalInitPosition;
    cluster->GetTransformLinkMatrix(clusterGlobalInitPosition);

    FbxAMatrix clusterGeometry = GetGeometry(cluster->GetLink());
    clusterGlobalInitPosition *= clusterGeometry;

    skeleton->at(boneListPosition).bindMatrix = clusterGlobalInitPosition;

    // Compute the shift of the link relative to the reference.
    //lVertexTransformMatrix = RGCP.Inverse() * CGCP * CGIP.Inverse() * RGIP * RG;
    // CGCP = position of bone
    // RGCP = mesh position
    FbxAMatrix offsetMatrix;
    FbxNode* boneNode = cluster->GetLink();

    FbxAMatrix CGIP, RGIP, vertexTransformMatrix;

    cluster->GetTransformLinkMatrix(CGIP);
    cluster->GetTransformMatrix(RGIP);

    vertexTransformMatrix = CGIP.Inverse() * RGIP;

    skeleton->at(boneListPosition).localBoneMatrix = vertexTransformMatrix;

	//*************
	//TODO: set it differently
	//So for the root node of skeleton, I need to get its global transformation matrix
	//FbxAMatrix BoneInitPoseGlobalTransform = pNode->EvaluateGlobalTransform();
	//then inverse it
	//FbxAMatrix InvBoneInitPoseGlobalTransform = BoneInitPoseGlobalTransform.Inverse();
	//And the local transform matrix of my root bone will be
	//FbxAMatrix BoneLocalTransform = InvBoneInitPoseGlobalTransform * BoneLocalTransform;
	//When animate the skin, I traverse the whole skeleton tree, for each bone node I calculate its global transformation matrix by: 
	//FbxAMatrix BoneGlobalTransform = BoneLocalTransform * ParentBoneGlobalTransform;
	//_skinDeformer = (FbxSkin *)pMesh->GetDeformer(0, FbxDeformer::eSkin);
	*/
}

	//glEnableVertexAttribArray (1);
	
	//glBindBuffer(GL_ARRAY_BUFFER, _vboMesh);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vboIndices);
		//glEnableClientState(GL_COLOR_ARRAY);	//glEnableClientState(GL_VERTEX_ARRAY);	//glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, sizeof(BVertexData), 0);	//glVertexAttribPointer((GLuint)1, 4, GL_FLOAT, GL_FALSE, sizeof(BVertexData), (void*) 12);			//glEnableVertexAttribArray (1);	//glColorPointer(4, GL_FLOAT, sizeof(BVertexData), (void*) 12); 	//glVertexPointer(3, GL_FLOAT, sizeof(BVertexData), 0);	//glDrawArrays (GL_TRIANGLES, 0, 3);		//glDrawElements(GL_TRIANGLES, _indicesCount, GL_UNSIGNED_INT, 0);	//glDisableClientState(GL_VERTEX_ARRAY);	//glDisableClientState(GL_COLOR_ARRAY);	
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

void BMesh::drawRecursiveDebug(FbxNode * pNode)
{
	const int lChildCount = pNode->GetChildCount();
	drawNodeDebug(pNode);
    /*
	for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
    {
        drawRecursiveDebug(pNode->GetChild(lChildIndex));
    }
	*/
}

void BMesh::drawNodeDebug(FbxNode *pNode)
{
	FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();

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
            drawMeshDebug(pNode);
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
}

void BMesh::drawMeshDebug(FbxNode * pMesh)
{
	FbxMesh* lMesh = pMesh->GetMesh();
	/*
	* A control point is a XYZ coordinate, it is synonym of vertex.
	* A polygon vertex is an index to a control point, it is synonym of vertex index.
	* A polygon is a group of polygon vertex.
	*/
	const int lVertexCount = lMesh->GetControlPointsCount();
	// No vertex to draw.
    if (lVertexCount == 0)
        return;

	FbxVector4 * lControlPoints = lMesh->GetControlPoints();
	int * lIndexes = lMesh->GetPolygonVertices();
	int lIndexCount = lMesh->GetPolygonVertexCount();

	glPushMatrix();
	glBegin(GL_TRIANGLES);
	for(int i = 0; i < lIndexCount; i++)
	{
		FbxVector4 lA = lControlPoints[lIndexes[i]].mData;
		//FbxVector4 lB = lControlPoints[lIndexes[i + 1]].mData;
		//FbxVector4 lC = lControlPoints[lIndexes[i + 2]].mData;
		glVertex3f(lA.mData[0], lA.mData[1], lA.mData[2]);
		//printf("V3(%f, %f, %f)\n", lA.mData[0], lA.mData[1], lA.mData[2]);
	}
	glEnd();
	glPopMatrix();
	//system("pause");
}

void BMesh::setProjectionMatrix(glm::mat4x4 pProjectionMatrix)
{
	_gl_ProjectionMatrix = pProjectionMatrix;
	glm::mat4 lMVP = _gl_ProjectionMatrix * _gl_ViewMatrix * _gl_ModelMatrix;
	_shader->setMVPMatrix(lMVP);
}

void BMesh::setViewMatrix(glm::mat4x4 pViewMatrix)
{
	_gl_ViewMatrix = pViewMatrix;
	glm::mat4 lMVP = _gl_ProjectionMatrix * _gl_ViewMatrix * _gl_ModelMatrix;
	_shader->setMVPMatrix(lMVP);
}