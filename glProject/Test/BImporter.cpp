#include "BImporter.h"
#include <fbxsdk.h>
#include <GL/glew.h>

BImporter::BImporter()
{
    //ctor
}

BImporter::~BImporter()
{
    //dtor
}


BGameObject * BImporter::loadObject(string pPath)
{
    //do i really need importer??
    mImporter = FbxImporter::Create(lSdkManager,"");
    int lFileFormat = -1;
	const char* mFileName = pPath.c_str();
    if (!lSdkManager->GetIOPluginRegistry()->DetectReaderFileFormat(mFileName, lFileFormat) )
    {
        // Unrecognizable file format. Try to fall back to FbxImporter::eFBX_BINARY
        lFileFormat = lSdkManager->GetIOPluginRegistry()->FindReaderIDByDescription( "FBX binary (*.fbx)" );;
    }

	if(mImporter->Initialize(pPath.c_str(), lFileFormat) == true)
    {

    }
    //else error
    //mScene->GetRootNode()


        if (mImporter->Import(lScene) == true)
        {
			printf("Importing...");
            // Convert Axis System to what is used in this example, if needed
            FbxAxisSystem SceneAxisSystem = lScene->GetGlobalSettings().GetAxisSystem();
            FbxAxisSystem OurAxisSystem(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eRightHanded);
            if( SceneAxisSystem != OurAxisSystem )
            {
                OurAxisSystem.ConvertScene(lScene);
            }

            // Convert Unit System to what is used in this example, if needed
            FbxSystemUnit SceneSystemUnit = lScene->GetGlobalSettings().GetSystemUnit();
            if( SceneSystemUnit.GetScaleFactor() != 1.0 )
            {
                //The unit in this example is centimeter.
                FbxSystemUnit::cm.ConvertScene( lScene);
            }

            // Get the list of all the animation stack.
            lScene->FillAnimStackNameArray(mAnimStackNameArray);
            // Convert mesh, NURBS and patch into triangle mesh
			FbxGeometryConverter lGeomConverter(lSdkManager);
			lGeomConverter.Triangulate(lScene, true);

			// Split meshes per material, so that we only have one material per mesh (for VBO support)
			lGeomConverter.SplitMeshesPerMaterial(lScene, true);


			BGameObject *lGameObject = prepareVBOMeshes(lScene);
            return lGameObject;
            // Bake the scene for one frame
            //LoadCacheRecursive(mScelGeomConverterne, mCurrentAnimLayer, mFileName, mSupportVBO);

            // Convert any .PC2 point cache data into the .MC format for
            // vertex cache deformer playback.
            //PreparePointCacheData(mScene, mCache_Start, mCache_Stop);

            // Get the list of pose in the scene
            //FillPoseArray(mScene, mPoseArray);

            // Initialize the frame period.
            //mFrameTime.SetTime(0, 0, 0, 1, 0, lScene->GetGlobalSettings().GetTimeMode());
            //lResult = true;
        }
        printf("Cant import this object");
        return NULL;
}

BGameObject * BImporter::prepareVBOMeshes(FbxScene *pScene)
{
	BGameObject * lGameObject = new BGameObject(pScene->GetRootNode());
    return lGameObject;
}

void BImporter::initImporter()
{
    lSdkManager = NULL;
    lScene = NULL;

    // Prepare the FBX SDK.
    InitializeSdkObjects(lSdkManager, lScene);
}

void BImporter::InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene)
{
    //The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
    pManager = FbxManager::Create();
    if( !pManager )
    {
        FBXSDK_printf("Error: Unable to create FBX Manager!\n");
        exit(1);
    }
        else FBXSDK_printf("Autodesk FBX SDK version %s\n", pManager->GetVersion());

        //Create an IOSettings object. This object holds all import/export settings.
        FbxIOSettings* ios = FbxIOSettings::Create(pManager, IOSROOT);
        pManager->SetIOSettings(ios);

        //Load plugins from the executable directory (optional)
        FbxString lPath = FbxGetApplicationDirectory();
        pManager->LoadPluginsDirectory(lPath.Buffer());

    //Create an FBX scene. This object holds most objects imported/exported from/to files.
    pScene = FbxScene::Create(pManager, "My Scene");
        if( !pScene )
    {
        FBXSDK_printf("Error: Unable to create FBX scene!\n");
        exit(1);
    }
}

void BImporter::freeImporter()
{
    // Destroy all objects created by the FBX SDK.
    DestroySdkObjects(lSdkManager, true);
}

void BImporter::DestroySdkObjects(FbxManager* pManager, bool pExitStatus)
{
    //Delete the FBX Manager. All the objects that have been allocated using the FBX Manager and that haven't been explicitly destroyed are also automatically destroyed.
    if( pManager ) pManager->Destroy();
        if( pExitStatus ) FBXSDK_printf("Program Success!\n");
}