#include <math.h>

#include "vecmath.h"

#include <string.h>

#define M4ROW(m, v, i) \
do {v.x=m->ptr[0][i];v.y=m->ptr[1][i];v.z=m->ptr[2][i];v.w=m->ptr[3][i];} while (0)
#define M4SET(m, _11, _12, _13, _14, _21, _22, _23, _24, _31, _32, _33, _34, _41, _42, _43, _44) \
do {m->ptr[0][0]=_11; m->ptr[0][1]=_21; m->ptr[0][2]=_31; m->ptr[0][3]=_41; \
    m->ptr[1][0]=_12; m->ptr[0][1]=_22; m->ptr[0][2]=_32; m->ptr[0][3]=_42; \
    m->ptr[2][0]=_13; m->ptr[0][1]=_23; m->ptr[0][2]=_33; m->ptr[0][3]=_43; \
    m->ptr[3][0]=_14; m->ptr[0][1]=_24; m->ptr[0][2]=_34; m->ptr[0][3]=_44; } while (0)

const Mat4f MAT4F_IDENTITY = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};


float Vec3f_Dot(const Vec3f* a, const Vec3f* b)
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
    float divisor = 1.f / sqrtf(Vec3f_Dot(src, src));
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

Vec4f* Vec4f_RQuat(Vec4f* dst, const Vec3f* axis, float angle)
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

Mat4f* Mat4f_Identity(Mat4f* dst)
{
	memset(dst, 0, sizeof(*dst));
	dst->ptr[0][0] = 1.f;
	dst->ptr[1][1] = 1.f;
	dst->ptr[2][2] = 1.f;
	dst->ptr[3][3] = 1.f;
	return dst;
}

Mat4f* Mat4_From3DBasis(Mat4f* dst, const Vec3f* x, const Vec3f* y, const Vec3f* z)
{
	dst->ptr[0][0] = x->x;
	dst->ptr[0][1] = y->x;
	dst->ptr[0][2] = z->x;
	dst->ptr[0][3] = 0.f;
	dst->ptr[1][0] = x->y;
	dst->ptr[1][1] = y->y;
	dst->ptr[1][2] = z->y;
	dst->ptr[1][3] = 0.f;
	dst->ptr[2][0] = x->z;
	dst->ptr[2][1] = y->z;
	dst->ptr[2][2] = z->z;
	dst->ptr[2][3] = 0.f;
	dst->ptr[3][0] = 0.f;
	dst->ptr[3][1] = 0.f;
	dst->ptr[3][2] = 0.f;
	dst->ptr[3][3] = 1.f;
	return dst;
}

Mat4f* Mat4f_PerspectiveRH(Mat4f* dst, float fov, float aspect, float near, float far)
{
    float ar = 1.f / aspect;
    float angle = 0.5f * fov;
    float divisor = 1.f / (far - near);
    float ctg = cosf(angle) / sinf(angle);
	memset(dst, 0, sizeof(*dst));
    dst->ptr[0][0] = ar * ctg;
    dst->ptr[1][1] = ctg;
    dst->ptr[2][2] = -(far + near) * divisor; // first "-" for right, "+" for left
    dst->ptr[3][2] = -2.f * far * near * divisor;
    dst->ptr[2][3] = -1.f; // -1 right, 1 left
    return dst;
}

Mat4f* Mat4f_PerspectiveLH(Mat4f* dst, float fov, float aspect, float near, float far)
{
	float ar = 1.f / aspect;
	float angle = 0.5f * fov;
	float divisor = 1.f / (far - near);
	float ctg = cosf(angle) / sinf(angle);
	memset(dst, 0, sizeof(*dst));
	dst->ptr[0][0] = ar * ctg;
	dst->ptr[1][1] = ctg;
	dst->ptr[2][2] = (far + near) * divisor;
	dst->ptr[3][2] = -2.f * far * near * divisor;
	dst->ptr[2][3] = 1.f;
	return dst;
}

Mat4f* Mat4f_LookAt(Mat4f* dst, const Vec3f* eye, const Vec3f* target, const Vec3f* up)
{
	Vec3f dir;
	vec3f_Normalize(&dir, vec3f_Sub(&dir, target, eye));
	return Mat4f_LookDir(dst, eye, &dir, up);
}

Mat4f* Mat4f_LookDir(Mat4f* dst, const Vec3f* eye, const Vec3f* dir, const Vec3f* up)
{
    Vec3f newX, newY;
    vec3f_Cross(&newX, up, dir);
    vec3f_Cross(&newY, dir, &newX);
    dst->ptr[0][0] = newX.x;
    dst->ptr[0][1] = newY.x;
    dst->ptr[0][2] = dir->x;
    dst->ptr[0][3] = 0.f;
    dst->ptr[1][0] = newX.y;
    dst->ptr[1][1] = newY.y;
    dst->ptr[1][2] = dir->y;
    dst->ptr[1][3] = 0.f;
    dst->ptr[2][0] = newX.z;
    dst->ptr[2][1] = newY.z;
    dst->ptr[2][2] = dir->z;
    dst->ptr[2][3] = 0.f;
    dst->ptr[3][0] = -Vec3f_Dot(eye, &newX);
    dst->ptr[3][1] = -Vec3f_Dot(eye, &newY);
    dst->ptr[3][2] = -Vec3f_Dot(eye, dir);
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

Mat4f* Mat4f_Translate(Mat4f* dst, const Vec3f* delta)
{
	dst->col[3].x = delta->x;
	dst->col[3].y = delta->y;
	dst->col[3].z = delta->z;
	return dst;
}

Mat4fRow* Mat4f_GetRow(Mat4f* src, unsigned int r)
{
	return (Mat4fRow*)(&src->ptr[0][(r & 3u)]);
}
