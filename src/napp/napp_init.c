#include "napp_private.h"

#include <string.h>

NAppState GState;

static bool GIsInitialized = false;

bool NAppInitialize(const int argc, const char** argv)
{
	if (!GIsInitialized)
	{
		memset(&GState, 0, sizeof(GState));
		GIsInitialized = true;
	}
	return GIsInitialized;
}

void NAppFinalize()
{
	GIsInitialized = false;
}
