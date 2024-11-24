#pragma once

#include <directxmath.h>

class Camera
{
public:

	void SetPosition(float, float, float);
	void SetRotation(float, float, float);

	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetRotation();

	void Render();
	void GetViewMatrix(DirectX::XMMATRIX&);

private:
	float _positionX = 0.0f;
	float _positionY = 0.0f;
	float _positionZ = 0.0f; 	
	float _rotationX = 0.0f;
	float _rotationY = 0.0f;
	float _rotationZ = 0.0f;

	DirectX::XMMATRIX _viewMatrix;
};
