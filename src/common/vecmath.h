#pragma once

#include <stdint.h>

#define MATH_EPSILON      0.000001f               /* Very small, almost 0      */
#define MATH_PI           3.14159265358979323846f /* PI number                 */
#define MATH_PI_RCP       0.31830988618379067154f /* 1 over PI                 */
#define MATH_DEG_RAD      0.01745329251994329577f /* Degrees per radian        */
#define MATH_DEG_2_RAD(d) ( (d) * MATH_DEG_RAD )  /* Inline degrees to radians */
#define MATH_SQRT2        1.4142135623730951f	  /* Square root of 2          */
#define MATH_SQRT2_RCP    0.7071067811865475f     /* Square root of 1/2        */
#define MATH_SQRT3        1.7320508075688772f     /* Square root of 3          */
#define MATH_M4_IDENTITY \
    {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f}

#ifdef __cplusplus
extern "C"
{
#endif

/* Vector types declaration */

typedef union
{
	struct { int32_t x, y; };
	int32_t ptr[2];
} Vec2i;

typedef union
{
	struct { uint32_t c, r; };
	struct { uint32_t w, h; };
	uint32_t ptr[2];
} Vec2u;

typedef union
{
    struct { float x, y; };
    struct { float u, v; };
    float ptr[2];
} Vec2f;

typedef union
{
    struct { float x, y, z; };
    struct { float r, g, b; };
    float ptr[3];
} Vec3f;

typedef union
{
    struct { float x, y, z, w; };
    struct { float r, g, b, a; };
    float ptr[4];
} Vec4f;

typedef union
{
    struct { uint32_t x, y, z; };
    struct { uint32_t u, v, w; };
    uint32_t ptr[3];
} Vec3u;

#define V2F(a,b) ((Vec2f){(a),(b)})
#define V4F(r,g,b,a) ((Vec4f){(r),(g),(b),(a)})

/* Matrix type declaration */

typedef union
{
    Vec4f col[4];
    float ptr[4][4];
} Mat4f;

typedef struct  
{
	float x, padxy[3];
	float y, padyz[3];
	float z, padzw[3];
	float w;
} Mat4fRow;

/* Vector manipulations */

#define Vec3f_Copy(d,s) ((d)->x=(s)->x, (d)->y=(s)->y, (d)->z=(s)->z, (d))

float Vec3f_Dot(const Vec3f* a, const Vec3f* b);

Vec3f* vec3f_Normalize(Vec3f* dst, const Vec3f* src);

Vec3f* Vec3f_Add(Vec3f* dst, const Vec3f* a, const Vec3f* b);

Vec3f* vec3f_Sub(Vec3f* dst, const Vec3f* a, const Vec3f* b);

Vec3f* Vec3f_SSub(Vec3f* dst, const Vec3f* a, float s);

Vec3f* Vec3f_Mul(Vec3f* dst, const Vec3f* a, const Vec3f* b);

Vec3f* Vec3f_SMul(Vec3f* dst, const Vec3f* a, float s);

Vec3f* Vec3f_Cross(Vec3f* dst, const Vec3f* a, const Vec3f* b);

float vec4f_Dot(const Vec4f* a, const Vec4f* b);

Vec4f* Vec4f_RQuat(Vec4f* dst, const Vec3f* axis, float angle);

Vec3f* Vec3f_Rotate(Vec3f* dst, const Vec3f* src, const Vec4f* quat);

/* Matrix init functions */

Mat4f* Mat4f_Identity(Mat4f* dst);

Mat4f* Mat4_From3DBasis(Mat4f* dst, const Vec3f* x, const Vec3f* y, const Vec3f* z);

Mat4f* Mat4f_PerspectiveRH(Mat4f* dst, float fov, float aspect, float near, float far);

Mat4f* Mat4f_PerspectiveLH(Mat4f* dst, float fov, float aspect, float near, float far);

Mat4f* Mat4f_LookAt(Mat4f* dst, const Vec3f* eye, const Vec3f* target, const Vec3f* up);

Mat4f* Mat4f_LookDir(Mat4f* dst, const Vec3f* eye, const Vec3f* dir, const Vec3f* up);

Mat4f* mat4f_Rotation(Mat4f* dst, const Vec4f* quat);

Mat4f* mat4f_Mul(Mat4f* dst, const Mat4f* a, const Mat4f* b);

Mat4f* Mat4f_Translate(Mat4f* dst, const Vec3f* delta);

Mat4fRow* Mat4f_GetRow(Mat4f* src, unsigned int r);

/* create rotation, translation, scale */
    
#ifdef __cplusplus
}
#endif
