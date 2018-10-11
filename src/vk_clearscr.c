#include "shared/args.h"
#include "shared/main.inl"

typedef struct
{
	int dummy;
} TestApp;

void initialize(void* dataPtr)
{
	TestApp* context = (TestApp*)dataPtr;
	context->dummy = 8;
}

int appMain(int argc, const CString* argv)
{
	argvParse(argc, argv);
	static TestApp appContext;
	static AppCallbacks cbMap =
	{
		.beforeStart = &initialize
	};
	appInitialize(&cbMap, &appContext);
	while (appShouldKeepRunning())
	{
		appPollEvents();
	}
	return 0;
}
