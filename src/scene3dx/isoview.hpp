#pragma once

#include <vecmath.hpp>

class IsoView
{

public:

    float ClipDistance = 1000.f;

    float AspectRatio = 1.f;

    float PanningSpeed = 1.f;

    float PanningAccel = 1.f;

    void Commit(Mat4f& projection, Mat4f& viewTransform) const;

    void Update(float deltaT);

private:

    Vec2f CurrentPanSpeed;

    Vec2f TargetPanSpeed;

    float XOffset = 0.f;

    float YOffset = 0.f;

    int Zoom = 32;

};
