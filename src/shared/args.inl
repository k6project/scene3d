#include "args.h"

#include <stdlib.h>

#ifdef _MSC_VER
#include <wchar.h>
#define ATOI(s) _wtoi(s)
#else
#define ATOI(s) atoi(s)
#endif

static struct Options gOptions_ =
{
    .windowWidth  = 512,
    .windowHeight = 512,
	.isFullscreen = 0,
    .windowTitle = gOptions_.appName
};

const struct Options* gOptions = &gOptions_;

struct ArgItem
{
    int key;
    int values;
    int (*parser)(const TChar* arg, void* val);
    void* val;
};

typedef struct ArgItem ArgItem;

static int argParseBool(const TChar* arg, void* val)
{
    int* ptr = val;
    *ptr = 1;
    return 0;
}

static int argParseStr(const TChar* arg, void* val)
{
    const TChar** ptr = val;
    if (!ptr)
        return -1;
    *ptr = arg;
    return 0;
}

static int argParseInt(const TChar* arg, void* val)
{
    int* ptr = val;
    if (!ptr)
        return -1;
    *ptr = ATOI(arg);
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

void argvParse(int argc, const TChar** argv)
{
    for (int i = 0; i < argc; i++)
    {
        const TChar* arg = argv[i];
        if (*arg == '-')
        {
            TChar key = *++arg;
            if (*++arg == 0)
            {
                for (const ArgItem* carg = gArgMap; carg->key; carg++)
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
