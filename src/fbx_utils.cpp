#include "fbx_utils.h"

#include <fbxsdk.h>
#include <fbxsdk/fileio/fbxiosettings.h>

void FbxLoadScene()
{
	FbxManager* fbxManager = FbxManager::Create();
	FbxImporter* fbxImporter = FbxImporter::Create(fbxManager, "Scene3D Importer");
	fbxImporter->Destroy();
	fbxManager->Destroy();
}
