#include <windows.h>

#include <common.hpp>
#include <renderer.hpp>
#include <scene3dx.hpp>

#define MAX_PRIMITIVES 4096

#define  TEST_MAP_W 5
#define  TEST_MAP_H 5
static const char* TEST_MAP =
"#####"
"#...#"
"#...#"
"##.##"
" #.# ";

struct GlobalParameters
{
    Mat4f Projection;
    Mat4f ViewTransform;
};

struct LocalParameters
{
    Mat4f ModelTransform;
};

static void DefaultCommitParamerters(ScenePrimitive* prim, void* ptr, size_t max)
{
	LocalParameters* locals = static_cast<LocalParameters*>(ptr);
	Mat4f_Identity(&locals->ModelTransform);
}

Scene3DXApp::Scene3DXApp()
	: VerticalFOV(MATH_DEG_2_RAD(90.f))
	, ClipDistance(1000.f)
#if 0
	, CameraPosition({ 5.f, 5.f, -5.f })
	, ViewDirection({ -MATH_SQRT3_RCP, -MATH_SQRT3_RCP, MATH_SQRT3_RCP })
	, UpVector({ -1.f/3.f, 2.f/3.f, 1.f/3.f })
	, RightVector({ MATH_SQRT3_RCP, 0.f, MATH_SQRT3_RCP })
	, ViewOrigin({ 0.f, 0.f, 8.6602527f })//Offset of every object in camera space, as if camera was at (0,0,0)
#else
	, CameraPosition({0.f, 0.f, -10.f})
	, ViewDirection({0.f, 0.f, 1.f})
	, UpVector({0.f, 1.f, 0.f})
	, RightVector({1.f, 0.f, 0.f})
	, ViewOrigin({ 0.f, 0.f, 10.f })//Offset of every object in camera space, as if camera was at (0,0,0)
#endif
{
}

void Scene3DXApp::CommitParameters(void* buffer, size_t max) const
{
	Parameters.Reset();
	GlobalParameters* globals = Parameters.TAlloc<GlobalParameters>(1, 256);
	//Mat4_From3DBasis(&globals->ViewTransform, &RightVector, &UpVector, &ViewDirection);
	//Mat4f_Translate(Mat4_From3DBasis(&globals->ViewTransform, &RightVector, &UpVector, &ViewDirection), &ViewOrigin);
    MapView.Commit(globals->Projection, globals->ViewTransform);
    uint32_t count = 0;
	for (ScenePrimitive* p = Primitives; p != nullptr; p = p->Next)
	{
		p->LocalParameters.Offset = Parameters.GetBytesUsed();
		void* locals = Parameters.Alloc(sizeof(LocalParameters), 256);
		p->LocalParameters.Length = Parameters.GetBytesUsed() - p->LocalParameters.Offset;
		p->CommitParameters(p, locals, sizeof(LocalParameters));

		++count;
	}
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
	LevelMap.Initialize(TEST_MAP_W, TEST_MAP_H, TEST_MAP);
	ViewportDimensions.x = static_cast<float>(w);
	ViewportDimensions.y = static_cast<float>(h);
	ViewportDimensions.z = 1.f / ViewportDimensions.x;
	ViewportDimensions.w = 1.f / ViewportDimensions.y;
	AspectRate.Val = ViewportDimensions.x * ViewportDimensions.w;
	AspectRate.Rcp = 1.f / AspectRate.Val;
    MapView.AspectRatio = AspectRate.Val;
    MapView.ClipDistance = ClipDistance;
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
			/*if (dx < -3 || dx > 3)
			{
				Vec4f rotation = {};
				Mat4f newLookAt = {};
				Vec3f viewTarget = {};
				float ndx = static_cast<float>(dx) * ViewportDimensions.z * AspectRate.Val;
				float angle = ndx * MATH_PI * 2.f;
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
				Vec3f viewTarget = {};
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
			}*/
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
	Primitives = LevelMap.GetPrimitives();
	Renderer->RenderScene(this);
}

void Scene3DXApp::Finalize()
{
	KeepRunning = false;
	Renderer->Finalize();
	LevelMap.Finalize();
}

