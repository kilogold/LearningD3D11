#pragma once
#include <DirectXMath.h>
using namespace DirectX;

class Camera
{
private:
	XMVECTOR upVector;
	XMVECTOR forwardVector;
	XMVECTOR position;
	XMVECTOR lookAt; //position-vector
public:

	// Mutators
	void Translate(XMVECTOR& translation);
	void SetRotation(XMVECTOR axis, float angleDegrees);

	// Accessors
	XMMATRIX GetViewMatrix();
	const XMVECTOR& GetPositionVector() { return position; }
	XMFLOAT4 GetPositionFloat();
	XMFLOAT4 GetForwardDirectionFloat();
	Camera();
	~Camera();
};