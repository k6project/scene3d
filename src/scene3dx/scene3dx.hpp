#pragma once

class RendererAPI;

class Scene3DXApp
{
public:
	bool ShouldKeepRunning() const;
	void Initialize(void* window);
	void Update(float deltaT);
	void Finalize();
private:
	void CreateTextures();
	void CreateMaterials();
	bool KeepRunning = false;
	//Material SpriteMaterial;
	RendererAPI* Renderer;
};

