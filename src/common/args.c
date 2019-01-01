#include "../global.h"

#include "args.h"

#include <stdlib.h>
#include <string.h>

static int ParseBool(const char* arg, void* val)
{
    int* ptr = val;
    *ptr = 1;
    return 0;
}

static int ParseStr(const char* arg, void* val)
{
	const char** ptr = val;
    if (!ptr)
        return -1;
    *ptr = arg;
    return 0;
}

static int ParseInt(const char* arg, void* val)
{
    int* ptr = val;
    if (!ptr)
        return -1;
    *ptr = atoi(arg);
    return 0;
}

extern void appGetName(char* buff, size_t max);

static inline void InitOptions(struct OptionsImpl* opts)
{
    memset(opts, 0, sizeof(struct OptionsImpl));
    opts->windowWidth = 512;
    opts->windowHeight = 512;
    opts->isFullscreen = 0;
    opts->windowTitle = opts->appName;
    appGetName(opts->appName, APP_NAME_MAX);
}

Options ArgParseCmdLine(int argc, const char** argv, MemAlloc mem)
{
    struct OptionsImpl* opts = mem_ForwdAlloc(mem, sizeof(struct OptionsImpl));
    struct Argument
    {
        int key, values;
        int (*parser)(const char* arg, void* val);
        void* val;
    } argMap[] =
    {
        {'w', 1, &ParseInt, &opts->windowWidth},
        {'h', 1, &ParseInt, &opts->windowHeight},
        {'i', 1, &ParseStr, (void*)&opts->inputFile},
        {'t', 1, &ParseStr, (void*)&opts->windowTitle },
        {'f', 0, &ParseBool, &opts->isFullscreen},
        { 0 , 0, NULL, NULL }
    };
    InitOptions(opts);
    for (int i = 0; i < argc; i++)
    {
        const char* arg = argv[i];
        if (*arg == '-')
        {
            char key = *++arg;
            if (*++arg == 0)
            {
                for (const struct Argument* carg = argMap; carg->key; carg++)
                {
                    if (carg->key == key)
                    {
                        arg = NULL;
                        if (carg->values)
                        {
                            i += carg->values;
                            arg = argv[i];
                        }
                        carg->parser(arg, carg->val);
                        break;
                    }
                }
            }
        }
    }
    return opts;
}

/*void vkUseExtensionsAPP(const char* names[], uint32_t count)
{
	gOptions_.extensions = names;
	gOptions_.numExtensions = count;
}

void vkUseLayersAPP(const char* names[], uint32_t count)
{
	gOptions_.layers = names;
	gOptions_.numLayers = count;
}*/
