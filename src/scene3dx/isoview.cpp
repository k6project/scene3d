#include "isoview.hpp"

#include <renderer.hpp>

void IsoView::SetView(float aspectRatio, float clipDistance)
{
    AspectRatio = (aspectRatio > 0.f) ? aspectRatio : 1.f;
    ClipDistance = (clipDistance > 0.f) ? clipDistance : 1.f;
}

void IsoView::Commit(Mat4f& projection, Mat4f& viewTransform) const
{
    Vec4f rotA, rotB, rotC;
    Vec3f aAxis = { 0., 0.f, 1.f };
    Vec3f bAxis = { 1., 0.f, 0.f };
    Mat4f_Identity(&viewTransform);
    Vec4f_RQuat(&rotA, &aAxis, MATH_DEG_2_RAD(45.f));
    Vec4f_RQuat(&rotB, &bAxis, MATH_DEG_2_RAD(54.682f));
    Vec4f_RQuatMul(&rotC, &rotB, &rotA);
    Mat4f_Rotation(&viewTransform, &rotC);
    //viewTransform.col[3].x = XOffset;
    //viewTransform.col[3].y = YOffset;
    float zoom = static_cast<float>(Zoom);
    if (RendererAPI::Get()->HasRHClipSpace())
    {
        //Mat4f_OrthographicRH(&projection, Zoom, AspectRatio, 0.f, ClipDistance);
    }
    else
    {
        Mat4f_OrthographicLH(&projection, zoom, AspectRatio, 0.f, ClipDistance);
    }
}
