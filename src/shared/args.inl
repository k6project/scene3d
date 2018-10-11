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
    512, // windowWidth
    512, // windowHeight
    NULL // inputFile
};
struct Options* gOptions = &gOptions_;

struct ArgItem
{
    int key;
    int (*parser)(const CString arg, void* val);
    void* val;
};

typedef struct ArgItem ArgItem;

static int argParseStr(const CString arg, void* val)
{
    CString* ptr = val;
    if (!ptr)
        return -1;
    *ptr = arg;
    return 0;
}

static int argParseInt(const CString arg, void* val)
{
    int* ptr = val;
    if (!ptr)
        return -1;
    *ptr = ATOI(arg);
    return 0;
}

static ArgItem gArgMap[] =
{
    {'w', &argParseInt, &gOptions_.windowWidth},
    {'h', &argParseInt, &gOptions_.windowHeight},
    {'i', &argParseStr, &gOptions_.inputFile},
    { 0 , NULL, NULL }
};

void argvParse(int argc, const CString* argv)
{
    for (int i = 0; i < argc; i++)
    {
        CString arg = argv[i];
        if (*arg == '-')
        {
            TChar key = *++arg;
            if (*++arg == 0)
            {
                for (const ArgItem* carg = gArgMap; carg->key; carg++)
                {
                    if (carg->key == key)
                    {
                        arg = argv[++i];
                        carg->parser(arg, carg->val);
                        break;
                    }
                }
            }
        }
    }
}
