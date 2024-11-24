#include "Camera.h"

void Camera::SetPosition(float x, float y, float z)
{
	_positionX = x;
	_positionY = y;
	_positionZ = z;
}

void Camera::SetRotation(float x, float y, float z)
{
	_rotationX = x;
	_rotationY = y;
	_rotationZ = z;
}

DirectX::XMFLOAT3 Camera::GetPosition()
{
	return DirectX::XMFLOAT3(_positionX, _positionY, _positionZ);
}

DirectX::XMFLOAT3 Camera::GetRotation()
{
	return DirectX::XMFLOAT3(_rotationX, _rotationY, _rotationZ);
}

void Camera::Render() 
{
	DirectX::XMFLOAT3 up, position, lookAt;
	float yaw, pitch, roll;

	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;

	position.x = _positionX;
	position.y = _positionY;
	position.z = _positionZ;

	lookAt.x = 0.0f;
	lookAt.y = 0.0f;
	lookAt.z = 1.0f;

	pitch = _rotationX * 0.0174532925f;
	yaw = _rotationY * 0.0174532925f;
	roll = _rotationZ * 0.0174532925f;

	DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	DirectX::XMVECTOR lookAtVector = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&lookAt), rotationMatrix);
	DirectX::XMVECTOR upVector = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&up), rotationMatrix);
	DirectX::XMVECTOR positionVector = DirectX::XMLoadFloat3(&position);

	lookAtVector = DirectX::XMVectorAdd(positionVector, lookAtVector);

	_viewMatrix = DirectX::XMMatrixLookAtLH(positionVector, lookAtVector, upVector);
}

void Camera::GetViewMatrix(DirectX::XMMATRIX& viewMatrix)
{
	viewMatrix = _viewMatrix;
}
