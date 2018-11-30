#pragma once

#include "math_lib.h"

static const uint32_t CUBE_MESH_COLOR = 0xffffffffu;

static const uint32_t CUBE_MESH_NUM_VERTICES = 24;
static Vec4f CUBE_MESH_VERTICES[][RND_VBO_VECTORS] =
{
    /* Z-positive oriented side */
    { { -1.f,  1.f, 1.f,  0 }, { 0.f, 0.f, 1.f,  0.f } },
    { {  1.f,  1.f, 1.f,  0 }, { 0.f, 0.f, 1.f,  0.f } },
    { { -1.f, -1.f, 1.f,  0 }, { 0.f, 0.f, 1.f,  0.f } },
    { {  1.f, -1.f, 1.f,  0 }, { 0.f, 0.f, 1.f,  0.f } },

    /* Z-negative oriented side */
    { {  1.f,  1.f, -1.f, 0 }, { 0.f, 0.f, -1.f, 0.f } },
    { { -1.f,  1.f, -1.f, 0 }, { 0.f, 0.f, -1.f, 0.f } },
    { {  1.f, -1.f, -1.f, 0 }, { 0.f, 0.f, -1.f, 0.f } },
    { { -1.f, -1.f, -1.f, 0 }, { 0.f, 0.f, -1.f, 0.f } },

    /* Y-positive oriented side */
    { {  1.f, 1.f,  1.f,  0 }, { 0.f, 1.f, 0.f,  0.f } },
    { { -1.f, 1.f,  1.f,  0 }, { 0.f, 1.f, 0.f,  0.f } },
    { {  1.f, 1.f, -1.f,  0 }, { 0.f, 1.f, 0.f,  0.f } },
    { { -1.f, 1.f, -1.f,  0 }, { 0.f, 1.f, 0.f,  0.f } },

    /* Y-negative oriented side */
    { { -1.f, -1.f,  1.f, 0 }, { 0.f, -1.f, 0.f, 0.f } },
    { {  1.f, -1.f,  1.f, 0 }, { 0.f, -1.f, 0.f, 0.f } },
    { { -1.f, -1.f, -1.f, 0 }, { 0.f, -1.f, 0.f, 0.f } },
    { {  1.f, -1.f, -1.f, 0 }, { 0.f, -1.f, 0.f, 0.f } },

    /* X-positive oriented side */
    { { 1.f,  1.f,  1.f,  0 }, { 1.f, 0.f, 0.f,  0.f } },
    { { 1.f,  1.f, -1.f,  0 }, { 1.f, 0.f, 0.f,  0.f } },
    { { 1.f, -1.f,  1.f,  0 }, { 1.f, 0.f, 0.f,  0.f } },
    { { 1.f, -1.f, -1.f,  0 }, { 1.f, 0.f, 0.f,  0.f } },

    /* X-negative oriented side */
    { { -1.f,  1.f, -1.f, 0 }, { -1.f, 0.f, 0.f, 0.f } },
    { { -1.f,  1.f,  1.f, 0 }, { -1.f, 0.f, 0.f, 0.f } },
    { { -1.f, -1.f, -1.f, 0 }, { -1.f, 0.f, 0.f, 0.f } },
    { { -1.f, -1.f,  1.f, 0 }, { -1.f, 0.f, 0.f, 0.f } }
};

static const uint32_t CUBE_MESH_NUM_INDICES = 36;
static const uint32_t CUBE_MESH_INDICES[] =
{
    0,  2,  3,  3,  1,  0, /* Z-positive oriented side */
    4,  6,  7,  7,  5,  4, /* Z-negative oriented side */
    8, 10, 11, 11,  9,  8, /* Y-positive oriented side */
   12, 14, 15, 15, 13, 12, /* Y-negative oriented side */
   16, 18, 19, 19, 17, 16, /* X-positive oriented side */
   20, 22, 23, 23, 21, 20  /* X-negative oriented side */
};
