#pragma once

#include <common/vecmath.h>

#define VBO_UNIT_VECS 2

typedef Vec4f TVertex[VBO_UNIT_VECS];

typedef struct
{
    uint32_t numVertices;
    uint32_t numIndices;
    const TVertex* vertices;
    const uint32_t* indices;
} TMesh;

typedef struct
{
    uint32_t numMeshes;
    TMesh* meshes;
} TScene;
