#pragma once

#include <vecmath.hpp>

class IsoView
{

public:

    void SetView(float aspectRatio, float clipDistance);

    void Commit(Mat4f& projection, Mat4f& viewTransform) const;

private:

    float XOffset = 0.f;

    float YOffset = 0.f;

    float ClipDistance = 1.f;

    float AspectRatio = 1.f;

    int Zoom = 32;

};
