#include <napp/napp.h>
#include <wrl.h>

#include "App.h"

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
#ifdef NAPP_PLATFORM_WRL
	auto direct3DApplicationSource = ref new Direct3DApplicationSource();
	Windows::ApplicationModel::Core::CoreApplication::Run(direct3DApplicationSource);
#endif
	return 0;
}
