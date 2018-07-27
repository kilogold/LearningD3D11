#include "Camera.h"

XMFLOAT4 VectorToFloat4(const XMVECTOR &V)
{
    XMFLOAT4 float4Position;
    float4Position.x = XMVectorGetX(V);
    float4Position.y = XMVectorGetY(V);
    float4Position.z = XMVectorGetZ(V);
    float4Position.w = XMVectorGetW(V);
    return float4Position;
}

void Camera::Translate(XMVECTOR& translation)
{
    position += translation;
}

void Camera::TranslateLocal(XMVECTOR& translation)
{
    const XMVECTOR worldFwdVector = XMVectorSet(0, 0, 1, 0);
    const XMVECTOR localFwdVector = XMQuaternionMultiply(orientation, worldFwdVector);
    const XMVECTOR localTranslation = XMQuaternionMultiply(orientation, translation);

    position += localTranslation;
}

XMFLOAT4 Camera::GetPositionFloat()
{
    return VectorToFloat4(position);
}

XMFLOAT4 Camera::GetForwardDirectionFloat()
{
    const XMVECTOR worldFwdVector = XMVectorSet(0, 0, 1, 0);
    const XMVECTOR localFwdVector = XMQuaternionMultiply(orientation, worldFwdVector);
    return VectorToFloat4(localFwdVector);
}

void Camera::Rotate(XMVECTOR axis, float angleDegrees)
{
    orientation = XMQuaternionMultiply(XMQuaternionRotationAxis(axis, XMConvertToRadians(angleDegrees)),orientation);
}

XMMATRIX Camera::GetViewMatrix()
{
    const XMVECTOR worldFwdVector = XMVectorSet(0, 0, 1, 0);
    const XMVECTOR localFwdVector = XMQuaternionMultiply(orientation, worldFwdVector);

    XMVECTOR focusPoint = position + XMVectorScale(localFwdVector, 2.0f);
    return XMMatrixLookAtLH(position, focusPoint, upVector);
}

Camera::Camera()
{
    position = XMVectorSet(0, 5, -10, 1);
    upVector = XMVectorSet(0, 1, 0, 0);
    orientation = XMQuaternionIdentity();
}

Camera::~Camera()
{
}
