#pragma once
#include <DirectXMath.h>
using namespace DirectX;
typedef XMVECTOR XMQUATERNION;

class Camera
{
private:
    XMQUATERNION orientation;
    XMVECTOR upVector;
    XMVECTOR position;
    XMVECTOR lookAt; //position-vector
public:

    // Mutators
    void Translate(XMVECTOR& translation);
    void TranslateLocal(XMVECTOR& translation);
    void Rotate(XMVECTOR axis, float angleDegrees);

    // Accessors
    XMMATRIX GetViewMatrix();
    const XMVECTOR& GetPositionVector() { return position; }
    XMFLOAT4 GetPositionFloat();
    XMFLOAT4 GetForwardDirectionFloat();
    Camera();
    ~Camera();
};