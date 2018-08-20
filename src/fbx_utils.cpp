#include "fbx_utils.h"

#include <assert.h>
#include <fbxsdk.h>
#include <fbxsdk/fileio/fbxiosettings.h>

#ifdef _MSC_VER
#include <windows.h>
#include <stdarg.h>
static int DbgPrint(const char* fmt, ...)
{
	char buff[256];
	va_list args;
	va_start(args, fmt);
	int result = vsnprintf_s(buff, sizeof(buff), fmt, args);
	OutputDebugString(buff);
	va_end(args);
	return result;
}
#else // _MSC_VER
#include <stdio.h>
#define DbgPrint printf
#endif // _MSC_VER

#include "utils.h"

using namespace fbxsdk;

struct IFbxNodeHandler
{
    bool IsRoot(FbxNode* fbxNode) const
    {
        return (fbxNode->GetParent() == nullptr);
    }
    virtual void NodeBegin(FbxNode* fbxNode) = 0;
    virtual void NodeEnd(FbxNode* fbxNode) = 0;
};

struct FbxDebugInfo : public IFbxNodeHandler
{
    int NodeDepth = 0;
    virtual void NodeBegin(FbxNode* fbxNode) override
    {
		DbgPrint("[%d] %s\n", NodeDepth, (IsRoot(fbxNode)) ? "Scene" : fbxNode->GetName());
		if (FbxMesh* fbxMesh = fbxNode->GetMesh())
		{
			assert(fbxMesh->IsTriangleMesh());
			int* vertices = fbxMesh->GetPolygonVertices();
			FbxVector4* points = fbxMesh->GetControlPoints();
			for (int i = 0; i < fbxMesh->GetControlPointsCount(); i++)
			{
				DbgPrint("  Mesh point: { %f %f %f }\n", points[i][0], points[i][1], points[i][2]);
			}
			for (int i = 0; i < fbxMesh->GetPolygonCount(); i ++)
			{
                FbxVector4 fbxNormal;
				int fbxFaceSize = fbxMesh->GetPolygonSize(i);
                fbxMesh->GetPolygonVertexNormal(i, 0, fbxNormal);
				int* start = &vertices[fbxMesh->GetPolygonVertexIndex(i)];
			}
		}
        ++NodeDepth;
    }
    virtual void NodeEnd(FbxNode* fbxNode) override
    {
        --NodeDepth;
    }
};

static void FbxNodeIterator(FbxNode* fbxNode, IFbxNodeHandler& handler)
{
    handler.NodeBegin(fbxNode);
    for (int i = 0; i < fbxNode->GetChildCount(); i++)
    {
        FbxNodeIterator(fbxNode->GetChild(i), handler);
    }
    handler.NodeEnd(fbxNode);
}

void FbxLoadScene(const char* fName)
{
	FbxManager* fbxManager = FbxManager::Create();
    FbxIOSettings* ioSettings = FbxIOSettings::Create(fbxManager, IOSROOT);
    fbxManager->SetIOSettings(ioSettings);
	FbxImporter* fbxImporter = FbxImporter::Create(fbxManager, "Scene3D Importer");
    if (fbxImporter->Initialize(fName, -1, fbxManager->GetIOSettings()))
    {
        FbxScene* fbxScene = FbxScene::Create(fbxManager, "Imported Scene");
        if (fbxImporter->Import(fbxScene))
        {
            FbxDebugInfo fbxDebugInfo;
            FbxNodeIterator(fbxScene->GetRootNode(), fbxDebugInfo);
            fbxScene->Destroy();
        }
    }
	fbxImporter->Destroy();
	fbxManager->Destroy();
}
