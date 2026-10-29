#pragma once
#include "Math/Vector.h"
#include <numbers>

class Camera {
public:
    Camera() {
        transform.scale = { 1.0f, 1.0f,  1.0f };
        transform.rotate = { 0.3f, 0.0f,  0.0f };
        transform.translate = { 0.0f, 5.0f, -20.0f };
        mNearPlane = 0.1f;
        mFarPlane = 10000.0f;
    }

    Matrix4x4 GetViewMatrix() {
         cameraMatrix = MakeAffineMatrix(
            transform.scale, transform.rotate, transform.translate);
        return Inverse(cameraMatrix);
    }

    Matrix4x4 GetProjectionMatrix(float width, float height) {
        return MakePerspectiveFovMatrix(0.45f, width / height, mNearPlane, mFarPlane);
    }

    Matrix4x4 GetViewProjectionMatrix(float width, float height) {
         cameraMatrix = MakeAffineMatrix(
            transform.scale, transform.rotate, transform.translate);
        Matrix4x4 viewMatrix = Inverse(cameraMatrix);
        Matrix4x4 projMatrix = MakePerspectiveFovMatrix(
            0.45f, width / height, 0.1f, 100.0f);
        return Multiply(viewMatrix, projMatrix);
    }

    Matrix4x4 GetBillboardMatrix() {

        billboardMatrix = Multiply(backToFrontMatrix, cameraMatrix);

        billboardMatrix.m[3][0] = 0.0f;
        billboardMatrix.m[3][1] = 0.0f;
        billboardMatrix.m[3][2] = 0.0f;

        return billboardMatrix;
    }

    float GetNearPlane() const { return mNearPlane; }
    float GetFarPlane() const { return mFarPlane; }

    void SetNearPlane(float n) { mNearPlane = n; }
    void SetFarPlane(float f) { mFarPlane = f; }

    TransformData transform;

private:

    //Y軸に回転をさせる
    Matrix4x4 backToFrontMatrix = MakeRotateYMatrix(std::numbers::pi_v<float>);
    Matrix4x4 billboardMatrix{};

    Matrix4x4 cameraMatrix{};
    float mNearPlane;
    float mFarPlane;
};