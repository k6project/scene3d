#pragma once

#define MATH_PI           3.14159265358979323846f /* PI number                 */
#define MATH_PI_RCP       0.31830988618379067154f /* 1 over PI                 */
#define MATH_DEG_RAD      0.01745329251994329577f /* Degrees per radian        */
#define MATH_DEG_2_RAD(d) ( d * DEG_RAD )         /* Inline degrees to radians */
#define MATH_M4_IDENTITY \
    {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f}

#ifdef __cplusplus
extern "C"
{
#endif

/* Vector types declaration */

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

/* Vector manipulations */

float vec3f_Dot(const Vec3f* a, const Vec3f* b);

Vec3f* vec3f_Normalize(Vec3f* dst, const Vec3f* src);

Vec3f* vec3f_Sub(Vec3f* dst, const Vec3f* a, const Vec3f* b);

Vec3f* vec3f_Cross(Vec3f* dst, const Vec3f* a, const Vec3f* b);

float vec4f_Dot(const Vec4f* a, const Vec4f* b);

Vec4f* vec4f_RQuat(Vec4f* dst, const Vec3f* axis, float angle);

/* Matrix init functions */

Mat4f* mat4f_Perspective(Mat4f* dst, float fov, float aspect, float near, float far);

Mat4f* mat4f_LookAt(Mat4f* dst, const Vec3f* eye, const Vec3f* dir, const Vec3f* up);

Mat4f* mat4f_Rotation(Mat4f* dst, const Vec4f* quat);

Mat4f* mat4f_Mul(Mat4f* dst, const Mat4f* a, const Mat4f* b);

/* create rotation, translation, scale */
    
#ifdef __cplusplus
}
#endif