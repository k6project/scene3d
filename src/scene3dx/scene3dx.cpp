#include <windows.h>

#include <common.hpp>
#include <renderer.hpp>
#include <scene3dx.hpp>

#define MAX_PRIMITIVES 256

/*
// Left-facing (-X, Y)
{ -1.f, 0.f,  1.f, PackColor(255, 0, 0, 255) }, { -MATH_SQRT2_RCP, MATH_SQRT2_RCP, 0.f, 0.f },
{ -1.f, 0.f, -1.f, PackColor(255, 0, 0, 255) }, { -MATH_SQRT2_RCP, MATH_SQRT2_RCP, 0.f, 0.f },
{  0.f, 1.f,  0.f, PackColor(255, 0, 0, 255) }, { -MATH_SQRT2_RCP, MATH_SQRT2_RCP, 0.f, 0.f },
// Right-facing (X, Y)
{  1.f, 0.f, -1.f, PackColor(255, 0, 0, 255) }, {  MATH_SQRT2_RCP, MATH_SQRT2_RCP, 0.f, 0.f },
{  1.f, 0.f,  1.f, PackColor(255, 0, 0, 255) }, {  MATH_SQRT2_RCP, MATH_SQRT2_RCP, 0.f, 0.f },
{  0.f, 1.f,  0.f, PackColor(255, 0, 0, 255) }, {  MATH_SQRT2_RCP, MATH_SQRT2_RCP, 0.f, 0.f },
// Front-facing (Y,-Z)
{ -1.f, 0.f, -1.f, PackColor(0, 0, 255, 255) }, { 0.f, MATH_SQRT2_RCP, -MATH_SQRT2_RCP, 0.f },
{  1.f, 0.f, -1.f, PackColor(0, 0, 255, 255) }, { 0.f, MATH_SQRT2_RCP, -MATH_SQRT2_RCP, 0.f },
{  0.f, 1.f,  0.f, PackColor(0, 0, 255, 255) }, { 0.f, MATH_SQRT2_RCP, -MATH_SQRT2_RCP, 0.f },
// Back-facing (Y, Z)
{  1.f, 0.f,  1.f, PackColor(0, 0, 255, 255) }, { 0.f, MATH_SQRT2_RCP,  MATH_SQRT2_RCP, 0.f },
{ -1.f, 0.f,  1.f, PackColor(0, 0, 255, 255) }, { 0.f, MATH_SQRT2_RCP,  MATH_SQRT2_RCP, 0.f },
{  0.f, 1.f,  0.f, PackColor(0, 0, 255, 255) }, { 0.f, MATH_SQRT2_RCP,  MATH_SQRT2_RCP, 0.f },
// Bottom (Y)
*/

struct GlobalParameters
{
    Mat4f Projection;
    Mat4f ViewTransform;
};

struct LocalParameters
{
    Mat4f ModelTransform;
};

Scene3DXApp::Scene3DXApp()
	: VerticalFOV(MATH_DEG_2_RAD(90.f))
	, ClipDistance(1000.f)
	, CameraPosition({0.f, 0.f, -10.f})
	, ViewDirection({0.f, 0.f, 1.f})
	, UpVector({0.f, 1.f, 0.f})
	, RightVector({1.f, 0.f, 0.f})
	, ViewOrigin({ 0.f, 0.f, 10.f })
{
}

void Scene3DXApp::CommitParameters(void* buffer, size_t max) const
{
    Parameters.Reset();
    GlobalParameters* globals = Parameters.TAlloc<GlobalParameters>(1, 256);
	if (Renderer->HasRHClipSpace())
	{
		Mat4f_PerspectiveRH(&globals->Projection, VerticalFOV, AspectRate.Val, 0.001f, ClipDistance);
	}
	else
	{
		Mat4f_PerspectiveLH(&globals->Projection, VerticalFOV, AspectRate.Val, 0.001f, ClipDistance);
	}
	Mat4f_Translate(Mat4_From3DBasis(&globals->ViewTransform, &RightVector, &UpVector, &ViewDirection), &ViewOrigin);
	for (ScenePrimitive* p = Primitives; p != nullptr; p = p->Next)
	{
		p->LocalParameters.Offset = Parameters.GetBytesUsed();
		LocalParameters* locals = Parameters.TAlloc<LocalParameters>(1, 256);
		p->LocalParameters.Length = Parameters.GetBytesUsed();
		Mat4f_Identity(&locals->ModelTransform);
	}
	float test[] = { PackColor(1,1,1,1), PackColor(255,255,255,255) };
    Parameters.CopyTo(buffer, max);
}

const ScenePrimitive* Scene3DXApp::GetPrimitives() const
{
	return Primitives;
}

bool Scene3DXApp::ShouldKeepRunning() const
{
	return KeepRunning;
}

void Scene3DXApp::Initialize(void* window, uint32_t w, uint32_t h)
{
	Renderer = RendererAPI::Get();
    size_t globals = ALIGN(sizeof(GlobalParameters), 256);
    size_t perPrimitive = MAX_PRIMITIVES * ALIGN(sizeof(LocalParameters), 256);
    Parameters.Init(globals + perPrimitive);
	Renderer->Initialize(window, Parameters.GetCapacity(), globals);
	ViewportDimensions.x = static_cast<float>(w);
	ViewportDimensions.y = static_cast<float>(h);
	ViewportDimensions.z = 1.f / ViewportDimensions.x;
	ViewportDimensions.w = 1.f / ViewportDimensions.y;
	AspectRate.Val = ViewportDimensions.x * ViewportDimensions.w;
	AspectRate.Rcp = 1.f / AspectRate.Val;

	MaterialDescriptor mInfo = {};
	ScenePrimitive* test = MemAllocBase::Default()->TAlloc<ScenePrimitive>();
	mInfo.VertexShader.LoadFromFile("OverlayVertexShader.cso");
	mInfo.PixelShader.LoadFromFile("OverlayPixelShader.cso");
    Renderer->CreateMaterial(mInfo, &test->MaterialPtr);
	test->Next = nullptr;
	Primitives = test;
	
	KeepRunning = true;
}

void Scene3DXApp::MouseMoved(int32_t x, int32_t y, bool lb)
{
	if (lb)
	{
		if (Mouse.Captured)
		{
			bool modified = false;
			int32_t dx = x - Mouse.Pos.x;
			int32_t dy = y - Mouse.Pos.y;
			if (dx < -3 || dx > 3)
			{
				Vec4f rotation = {};
				Mat4f newLookAt = {};
				Vec3f viewTarget = { 0.f, 0.f, 0.f };
				float ndx = static_cast<float>(dx) * ViewportDimensions.z * AspectRate.Val;
				float angle = -ndx * MATH_PI * 2.f;
				Vec4f_RQuat(&rotation, &UpVector, angle);
				Vec3f_Rotate(&CameraPosition, &CameraPosition, &rotation);
				Mat4f_LookAt(&newLookAt, &CameraPosition, &viewTarget, &UpVector);
				ViewOrigin = { newLookAt.col[3].x, newLookAt.col[3].y, newLookAt.col[3].z };
				Vec3f_Copy(&ViewDirection, Mat4f_GetRow(&newLookAt, 2));
				Vec3f_Copy(&RightVector, Mat4f_GetRow(&newLookAt, 0));
				Vec3f_Copy(&UpVector, Mat4f_GetRow(&newLookAt, 1));
				modified = true;
			}
			if (dy < -3 || dy > 3)
			{
				Vec4f rotation = {};
				Mat4f newLookAt = {};
				Vec3f viewTarget = { 0.f, 0.f, 0.f };
				float ndy = static_cast<float>(dy) * ViewportDimensions.w;
				float angle = ndy * MATH_PI * 2.f;
				Vec4f_RQuat(&rotation, &RightVector, angle);
				Vec3f_Rotate(&CameraPosition, &CameraPosition, &rotation);
				Mat4f_LookAt(&newLookAt, &CameraPosition, &viewTarget, &UpVector);
				ViewOrigin = { newLookAt.col[3].x, newLookAt.col[3].y, newLookAt.col[3].z };
				Vec3f_Copy(&ViewDirection, Mat4f_GetRow(&newLookAt, 2));
				Vec3f_Copy(&RightVector, Mat4f_GetRow(&newLookAt, 0));
				Vec3f_Copy(&UpVector, Mat4f_GetRow(&newLookAt, 1));
				modified = true;
			}
			//compute rotation as product of two quaternions
			//apply to camera position, derive matrix
		}
		Mouse.Captured = true;
	}
	else
	{
		Mouse.Captured = false;
	}
	Mouse.Pos.x = x;
	Mouse.Pos.y = y;
}

void Scene3DXApp::CreateTextures()
{
	static const uint32_t defColorBGRA[] = 
	{	
		0x0000FFFFu, 0x00FF00FFu, 0xFF0000FFu, // RGB
		0xFFFF00FFu, 0xFF00FFFFu, 0x00FFFFFFu, // CMY
		0x000000FFu, 0xFFFFFFFFu			   // BW
	};
	static const uint32_t size = 64;
	static const int count = sizeof(defColorBGRA) / sizeof(defColorBGRA[0]);
	TextureDescriptor tDesc = {};
	tDesc.Width = size;
	tDesc.Height = size;
	tDesc.Format = RendererAPI::FormatBGRA8Unorm;
	/*tDesc.Data = new uint32_t[size * size];
	for (int i = 0; i < count; i++)
	{
		for (uint32_t y = 0; y < size; y++)
		{
			uint32_t scanline = y * size;
			for (uint32_t x = 0; x < size; x++)
			{
				tDesc.Data[scanline + x] = defColorBGRA[i];
			}
		}
		Texture* texture;
		Renderer->CreateTexture(tDesc, &texture);
	}*/
}

void Scene3DXApp::CreateMaterials()
{
	// 2D Sprite material
	{
		//MaterialDescriptor desc;
		//desc.SetShader(RendererAPI::VertexShader, "SpriteVertexShader");
		//desc.SetShader(RendererAPI::PixelShader, "SpritePixelShader");
		//Renderer->CreateMaterial(desc, &SpriteMaterial);
	}
}

void Scene3DXApp::Update(float deltaT)
{
	Renderer->RenderScene(this);
}

void Scene3DXApp::Finalize()
{
	KeepRunning = false;
	Renderer->Finalize();
}

