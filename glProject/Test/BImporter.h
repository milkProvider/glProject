#pragma once

#include <stdlib.h>
#include <fbxsdk.h>
#include "BGameObject.h"
#include <string>

class BImporter
{
    private:
        FbxManager* lSdkManager;
        FbxScene* lScene;
        FbxImporter* mImporter;
        FbxArray<FbxString*> mAnimStackNameArray;
        FbxArray<FbxPose*> mPoseArray;

		void InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene);
		void DestroySdkObjects(FbxManager* pManager, bool pExitStatus);
    public:
        BImporter();
        virtual ~BImporter();
        BGameObject * loadObject(string path);
        void initImporter();
        void freeImporter();
        BGameObject * prepareVBOMeshes(FbxScene *pScene);
    protected:

};
