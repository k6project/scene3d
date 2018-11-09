#include "global.h"

#include "args.h"

#include <stdlib.h>
#include <wtypes.h>

#ifdef __cplusplus
#define STRUCT_INIT(n,v) v
#define TPTR(n,t) (static_cast<t*>(n))
#else
#define STRUCT_INIT(n,v) .n = v
#define TPTR(n,t) (n)
#endif

static struct Options gOptions_ =
{
    STRUCT_INIT(windowWidth,  512),
	STRUCT_INIT(windowHeight, 512),
	STRUCT_INIT(isFullscreen, 0),
	STRUCT_INIT(windowTitle,  gOptions_.appName)
};

const struct Options* gOptions = &gOptions_;

struct ArgItem
{
    int key;
    int values;
    int (*parser)(const char* arg, void* val);
    void* val;
};

typedef struct ArgItem ArgItem;

static int argParseBool(const char* arg, void* val)
{
    int* ptr = TPTR(val, int);
    *ptr = 1;
    return 0;
}

static int argParseStr(const char* arg, void* val)
{
    const char** ptr = TPTR(val, const TChar*);
    if (!ptr)
        return -1;
    *ptr = arg;
    return 0;
}

static int argParseInt(const char* arg, void* val)
{
    int* ptr = TPTR(val, int);
    if (!ptr)
        return -1;
    *ptr = atoi(arg);
    return 0;
}

static ArgItem gArgMap[] =
{
    {'w', 1, &argParseInt, &gOptions_.windowWidth},
    {'h', 1, &argParseInt, &gOptions_.windowHeight},
    {'i', 1, &argParseStr, (void*)&gOptions_.inputFile},
	{'t', 1, &argParseStr, (void*)&gOptions_.windowTitle },
    {'f', 0, &argParseBool, &gOptions_.isFullscreen},
    { 0 , 0, NULL, NULL }
};

static inline void argInitOptions(Options* opts)
{
    memset(opts, 0, sizeof(Options));
    opts->windowWidth = 512;
    opts->windowHeight = 512;
    opts->isFullscreen = 0;
    opts->windowTitle = opts->appName;
    //appGetName(opts->appName, APP_NAME_MAX);
}

const Options* argParse(int argc, const void** argv, HMemAlloc mem)
{
    Options* opts = memForwdAlloc(mem, sizeof(Options));
    struct Argument
    {
        int key, values;
        int (*parser)(const char* arg, void* val);
        void* val;
    } argMap[] =
    {
        {'w', 1, &argParseInt, &opts->windowWidth},
        {'h', 1, &argParseInt, &opts->windowHeight},
        {'i', 1, &argParseStr, (void*)&opts->inputFile},
        {'t', 1, &argParseStr, (void*)&opts->windowTitle },
        {'f', 0, &argParseBool, &opts->isFullscreen},
        { 0 , 0, NULL, NULL }
    };
    argInitOptions(opts);
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

void vkUseExtensionsAPP(const char* names[], uint32_t count)
{
	gOptions_.extensions = names;
	gOptions_.numExtensions = count;
}

void vkUseLayersAPP(const char* names[], uint32_t count)
{
	gOptions_.layers = names;
	gOptions_.numLayers = count;
}
