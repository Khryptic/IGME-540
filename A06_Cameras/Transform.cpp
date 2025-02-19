#include "Transform.h"

// Transformers
// Moves the position based on x, y, z values
void Transform::MoveAbsolute(float x, float y, float z) {
	position.x += x;
	position.y += y;
	position.z += z;
}

// Moves the position based on an offset
void Transform::MoveAbsolute(DirectX::XMFLOAT3 offset) {
	position.x += offset.x;
	position.y += offset.y;
	position.z += offset.z;
}

// Moves the position relative to camera with floats
void Transform::MoveRelative(float x, float y, float z) {
	// Using the absolute direction, rotate the transform
	DirectX::XMVECTOR absDirection = DirectX::XMVector3Rotate(GetPositionVector(x, y, z), GetRotationVector());
	position.x += DirectX::XMVectorGetX(absDirection);
	position.y += DirectX::XMVectorGetY(absDirection);
	position.z += DirectX::XMVectorGetZ(absDirection);
}

// Moves the position relative to the camera with a float3
void Transform::MoveRelative(DirectX::XMFLOAT3 offset) {
	// Using the absolute direction, rotate the transform
	DirectX::XMVECTOR absDirection = DirectX::XMVector3Rotate(GetPositionVector(offset.x, offset.y, offset.z), GetRotationVector());
	position.x += DirectX::XMVectorGetX(absDirection);
	position.y += DirectX::XMVectorGetY(absDirection);
	position.z += DirectX::XMVectorGetZ(absDirection);
}

// Rotates based on pitch, yaw, roll values
void Transform::Rotate(float pitch, float yaw, float roll) {
	rotation.x += pitch;
	rotation.y += yaw;
	rotation.z += roll;
}

// Rotates based on an offset
void Transform::Rotate(DirectX::XMFLOAT3 rotation) {
	this->rotation.x += rotation.x;
	this->rotation.y += rotation.y;
	this->rotation.z += rotation.z;
}

// Scales based on x, y, z values
void Transform::Scale(float x, float y, float z) {
	scale.x *= x;
	scale.y *= y;
	scale.z *= z;
}

// Scales the based on an offset
void Transform::Scale(DirectX::XMFLOAT3 scale) {
	this->scale.x *= scale.x;
	this->scale.y *= scale.y;
	this->scale.z *= scale.z;
}

// Make a new World Matrix
void Transform::CreateWorldMatrix() {
	// Create new Matrices
	DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
	DirectX::XMMATRIX newRotation = DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
	DirectX::XMMATRIX newScale = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);

	// Make Matrix
	DirectX::XMMATRIX worldMatrix = newScale * newRotation * translation;

	// Store new Matrices
	DirectX::XMStoreFloat4x4(&world, worldMatrix);
	DirectX::XMStoreFloat4x4(&worldInverseTranspose,
		DirectX::XMMatrixInverse(0, DirectX::XMMatrixTranspose(worldMatrix)));
}