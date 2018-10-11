#include "args.h"
#include "main.inl"

typedef struct
{
    int dummy;
} TestApp;

void initialize(void* dataPtr)
{
    TestApp* context = (TestApp*) dataPtr;
    context->dummy = 8;
}

int appMain(int argc, const char** argv)
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
