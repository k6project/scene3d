#include "shared/main.inl"

#include <d3d12.h>

#pragma comment(lib, "d3d12.lib")
#define RELEASE(p) if(p)p->Release()

class DemoApp
{
public:
	void initialize() noexcept;
	void renderFrame() noexcept;
	void finalize() noexcept;
private:
	ID3D12Device* device = nullptr;
};

void DemoApp::initialize() noexcept
{}

void DemoApp::renderFrame() noexcept
{}

void DemoApp::finalize() noexcept
{
	RELEASE(device);
}

int appMain(int argc, const TChar** argv)
{
	DemoApp demoApp;
	argvParse(argc, argv);
	AppCallbacks callbacks = {};
	callbacks.beforeStart = [](void* arg) { static_cast<DemoApp*>(arg)->initialize(); };
	callbacks.beforeStop = [](void* arg) { static_cast<DemoApp*>(arg)->finalize(); };
	appInitialize(&callbacks, &demoApp);
	while (appShouldKeepRunning())
		demoApp.renderFrame();
	return 0;
}
