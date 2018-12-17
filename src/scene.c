#include "global.h"

#include "scene.h"

#include "cube.inl"

static TMesh gMeshes[] =
{
    {
        CUBE_MESH_NUM_VERTICES,
        CUBE_MESH_NUM_INDICES,
        CUBE_MESH_VERTICES,
        CUBE_MESH_INDICES
    }
};

static TScene gScene =
{
    ARRAY_LEN(gMeshes),
    gMeshes
};
