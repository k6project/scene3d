#include <stdio.h>

#include <gfx/common.h>

struct mat_t
{
    const char* stages[NUM_SHADER_STAGES];
    const char* fname;
};

static const char* g_labels[] =
{
    "VERT", "TESC", "TESE", "GEOM", "FRAG"
};

enum
{
    ARG_VERTEX_STAGE = SHADER_STAGE_VERTEX,
    ARG_TESS_CTRL_STAGE = SHADER_STAGE_TESS_CTRL,
    ARG_TESS_EVAL_STAGE = SHADER_STAGE_TESS_EVAL,
    ARG_GEOMETRY_STAGE = SHADER_STAGE_GEOMETRY,
    ARG_FRAGMENT_STAGE = SHADER_STAGE_FRAGMENT,
    ARG_OUTPUT_FNAME = NUM_SHADER_STAGES,
    ARG_UNKNOWN
};

void get_args(int argc, char** argv, struct mat_t* mat)
{
    int token = ARG_UNKNOWN;
    const char* arg = *argv;
    for (int i = 0; i < argc;)
    {
        if (*arg == '-')
        {
            switch (*(arg+1))
            {
                case 'v': token = ARG_VERTEX_STAGE; break;
                case 'g': token = ARG_GEOMETRY_STAGE; break;
                case 'f': token = ARG_FRAGMENT_STAGE; break;
                case 't':
                    switch (*arg+2)
                    {
                        case 'c': token = ARG_TESS_CTRL_STAGE; break;
                        case 'e': token = ARG_TESS_EVAL_STAGE; break;
                        default: token = ARG_UNKNOWN; break;
                    }
                    break;
                case 'o': token = ARG_OUTPUT_FNAME; break;
                default: token = ARG_UNKNOWN; break;
            }
            
        }
        else if (token != ARG_UNKNOWN)
        {
            if (token == ARG_OUTPUT_FNAME)
            {
                mat->fname = arg;
            }
            else
            {
                mat->stages[token] = arg;
            }
        }
        arg = argv[++i];
    }
}

void save(struct mat_t* mat)
{
    FILE* fp = NULL;
    if (mat->fname)
    {
        fp = fopen(mat->fname, "wb");
        if (!fp)
        {
            fprintf(stderr, "ERROR: failed to open %s\n", mat->fname);
        }
    }
    for (int i = SHADER_STAGE_VERTEX; i < NUM_SHADER_STAGES; i++)
    {
        fprintf(stdout, "%s: %s\n", g_labels[i], (mat->stages[i]) ? mat->stages[i] : "none");
    }
    if (fp)
    {
        fclose(fp);
    }
}

int main(int argc, char** argv)
{
    static struct mat_t mat;
    get_args(argc, argv, &mat);
    save(&mat);
    return 0;
}
