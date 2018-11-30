#include "global.h"

#include "math_lib.h"

#include <math.h>

#define M4ROW(m, v, i) \
do {v.x=m->ptr[0][i];v.y=m->ptr[1][i];v.z=m->ptr[2][i];v.w=m->ptr[3][i];} while (0)
#define M4SET(m, _11, _12, _13, _14, _21, _22, _23, _24, _31, _32, _33, _34, _41, _42, _43, _44) \
do {m->ptr[0][0]=_11; m->ptr[0][1]=_21; m->ptr[0][2]=_31; m->ptr[0][3]=_41; \
    m->ptr[1][0]=_12; m->ptr[0][1]=_22; m->ptr[0][2]=_32; m->ptr[0][3]=_42; \
    m->ptr[2][0]=_13; m->ptr[0][1]=_23; m->ptr[0][2]=_33; m->ptr[0][3]=_43; \
    m->ptr[3][0]=_14; m->ptr[0][1]=_24; m->ptr[0][2]=_34; m->ptr[0][3]=_44; } while (0)

const Mat4f MAT4F_IDENTITY = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};


float vec3f_Dot(const Vec3f* a, const Vec3f* b)
{
    float result = a->x * b->x + a->y * b->y + a->z * b->z;
    return result;
}

float vec4f_Dot(const Vec4f* a, const Vec4f* b)
{
    float result = a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w;
    return result;
}

Vec3f* vec3f_Normalize(Vec3f* dst, const Vec3f* src)
{
    float divisor = 1.f / sqrtf(vec3f_Dot(src, src));
    dst->x *= divisor;
    dst->y *= divisor;
    dst->z *= divisor;
    return dst;
}

Vec3f* vec3f_Sub(Vec3f* dst, const Vec3f* a, const Vec3f* b)
{
    dst->x = a->x - b->x;
    dst->y = a->y - b->y;
    dst->z = a->z - b->z;
    return dst;
}

Vec3f* vec3f_Cross(Vec3f* dst, const Vec3f* a, const Vec3f* b)
{
    dst->x = a->y * b->z - a->z * b->y;
    dst->y = a->z * b->x - a->x * b->z;
    dst->z = a->x * b->y - a->y * b->x;
    return dst;
}

Vec4f* vec4f_RQuat(Vec4f* dst, const Vec3f* axis, float angle)
{
    float half = 0.5f * angle;
    float cosA = cosf(half);
    float sinA = sinf(half);
    dst->x = axis->x * sinA;
    dst->y = axis->y * sinA;
    dst->z = axis->z * sinA;
    dst->w = cosA;
    return dst;
}

Mat4f* mat4f_Perspective(Mat4f* dst, float fov, float aspect, float near, float far)
{
    float ar = 1.f / aspect;
    float angle = 0.5f * fov;
    float divisor = 1.f / (far - near);
    float ctg = cosf(angle) / sinf(angle);
    dst->ptr[0][0] = ar * ctg;
    dst->ptr[1][1] = ctg;
    dst->ptr[2][2] = -(far + near) * divisor;
    dst->ptr[3][2] = -2.f * far * near * divisor;
    dst->ptr[2][3] = -1.f;
    return dst;
}

Mat4f* mat4f_LookAt(Mat4f* dst, const Vec3f* eye, const Vec3f* dir, const Vec3f* up)
{
    Vec3f newZ, newX, newY;
    vec3f_Normalize(&newZ, vec3f_Sub(&newZ, eye, dir));
    vec3f_Cross(&newX, up, &newZ);
    vec3f_Cross(&newY, &newZ, &newX);
    dst->ptr[0][0] = newX.x;
    dst->ptr[0][1] = newY.x;
    dst->ptr[0][2] = newZ.x;
    dst->ptr[0][3] = 0.f;
    dst->ptr[1][0] = newX.y;
    dst->ptr[1][1] = newY.y;
    dst->ptr[1][2] = newZ.y;
    dst->ptr[1][3] = 0.f;
    dst->ptr[2][0] = newX.z;
    dst->ptr[2][1] = newY.z;
    dst->ptr[2][2] = newZ.z;
    dst->ptr[2][3] = 0.f;
    dst->ptr[3][0] = -vec3f_Dot(eye, &newX);
    dst->ptr[3][1] = -vec3f_Dot(eye, &newY);
    dst->ptr[3][2] = -vec3f_Dot(eye, &newZ);
    dst->ptr[3][3] = 1.f;
    return dst;
}

Mat4f* mat4f_Rotation(Mat4f* dst, const Vec4f* quat)
{
    float qxSq2 = 2.f * quat->x * quat->x;
    float qySq2 = 2.f * quat->y * quat->y;
    float qzSq2 = 2.f * quat->z * quat->z;
    float qxqy2 = 2.f * quat->x * quat->y;
    float qxqz2 = 2.f * quat->x * quat->z;
    float qxqw2 = 2.f * quat->x * quat->w;
    float qzqw2 = 2.f * quat->z * quat->w;
    float qyqw2 = 2.f * quat->y * quat->w;
    float qyqz2 = 2.f * quat->y * quat->z;
    M4SET(dst,
          (1.f - qySq2 - qzSq2), (qxqy2 - qzqw2),       (qxqz2 + qyqw2),        0.f,
          (qxqy2 + qzqw2),       (1.f - qxSq2 - qzSq2), (qyqz2 - qxqw2),        0.f,
          (qxqz2 - qyqw2),       (qyqz2 + qxqw2),       (1.f - qxSq2 - qySq2),  0.f,
          0.f,                   0.f,                   0.f,                    1.f
    );
    return dst;
}

Mat4f* mat4f_Mul(Mat4f* dst, const Mat4f* a, const Mat4f* b)
{
    Vec4f r0, r1, r2, r3;
    M4ROW(a, r0, 0);
    M4ROW(a, r1, 1);
    M4ROW(a, r2, 2);
    M4ROW(a, r3, 3);
    M4SET(dst,
        vec4f_Dot(&r0,&(b->col[0])),vec4f_Dot(&r0,&(b->col[1])),vec4f_Dot(&r0,&(b->col[2])),vec4f_Dot(&r0,&(b->col[3])),
        vec4f_Dot(&r1,&(b->col[0])),vec4f_Dot(&r1,&(b->col[1])),vec4f_Dot(&r1,&(b->col[2])),vec4f_Dot(&r1,&(b->col[3])),
        vec4f_Dot(&r2,&(b->col[0])),vec4f_Dot(&r2,&(b->col[1])),vec4f_Dot(&r2,&(b->col[2])),vec4f_Dot(&r2,&(b->col[3])),
        vec4f_Dot(&r3,&(b->col[0])),vec4f_Dot(&r3,&(b->col[1])),vec4f_Dot(&r3,&(b->col[2])),vec4f_Dot(&r3,&(b->col[3]))
    );
    return dst;
}
