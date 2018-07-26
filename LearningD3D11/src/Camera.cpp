#include "Camera.h"

XMFLOAT4 VectorToFloat4(XMVECTOR &V)
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

XMFLOAT4 Camera::GetPositionFloat()
{
	return VectorToFloat4(position);
}

XMFLOAT4 Camera::GetForwardDirectionFloat()
{
	return VectorToFloat4(forwardVector);
}

void Camera::SetRotation(XMVECTOR axis, float angleDegrees)
{
	XMVECTOR quat = XMQuaternionRotationAxis(axis, XMConvertToRadians(angleDegrees));
	forwardVector = XMVector3Rotate(forwardVector, quat);
}

XMMATRIX Camera::GetViewMatrix()
{
	XMVECTOR focusPoint = position + XMVectorScale(forwardVector, 2.0f);
	return XMMatrixLookAtLH(position, focusPoint, upVector);
}

Camera::Camera()
{
	position = XMVectorSet(0, 5, -10, 1);
	upVector = XMVectorSet(0, 1, 0, 0);
	forwardVector = XMVectorSet(0, 0, 1, 0);
}

Camera::~Camera()
{
}
