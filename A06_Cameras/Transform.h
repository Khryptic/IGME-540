#pragma once
#include <d3d11.h>
#include <DirectXMath.h>

class Transform
{
public:
	//Constructor
	Transform() {
		// Set Default Values
		position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		rotation = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		scale = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);

		DirectX::XMStoreFloat4x4(&world, DirectX::XMMatrixIdentity());
		DirectX::XMStoreFloat4x4(&worldInverseTranspose, DirectX::XMMatrixIdentity());
	}

	// Setters
	// Set the position using x, y, z coordinates
	void SetPosition(float x, float y, float z) {
		position = DirectX::XMFLOAT3(x, y, z);
	}

	// Set the position using a FLOAT3
	void SetPosition(DirectX::XMFLOAT3 position) {
		this->position = position;
	}

	// Set rotation using pitch, yaw, roll values
	void SetRotation(float pitch, float yaw, float roll) {
		rotation = DirectX::XMFLOAT3(pitch, yaw, roll);
	}

	// Set rotation using a FLOAT3
	void SetRotation(DirectX::XMFLOAT3 rotation) {
		this->rotation = rotation;
	}

	// Set scale using x, y, z values
	void SetScale(float x, float y, float z) {
		scale = DirectX::XMFLOAT3(x, y, z);
	}

	// Set scale using a FLOAT3
	void SetScale(DirectX::XMFLOAT3 scale) {
		this->scale = scale;
	}

	// Getters
	DirectX::XMFLOAT3 GetPosition() const {
		return position;
	}

	DirectX::XMFLOAT3 GetRotation() const {
		return rotation;
	}

	DirectX::XMFLOAT3 GetScale() const {
		return scale;
	}

	DirectX::XMFLOAT4X4 GetWorldMatrix() const {
		return world;
	}

	DirectX::XMFLOAT4X4 GetWorldInverseTransposeMatrix() const {
		return worldInverseTranspose;
	}

	DirectX::XMVECTOR GetPositionVector(float x, float y, float z) const {
		return { x, y , z };
	}

	DirectX::XMVECTOR GetRotationVector() const {
		return DirectX::XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
	}

	DirectX::XMFLOAT3 GetRight() {
		DirectX::XMVECTOR rotatedRight = DirectX::XMVector3Rotate({ 1, 0, 0 }, GetRotationVector());
		return { DirectX::XMVectorGetX(rotatedRight), DirectX::XMVectorGetY(rotatedRight), DirectX::XMVectorGetZ(rotatedRight) };
	}

	DirectX::XMFLOAT3 GetUp() {
		DirectX::XMVECTOR rotatedUp = DirectX::XMVector3Rotate({ 0, 1, 0 }, GetRotationVector());
		return { DirectX::XMVectorGetX(rotatedUp), DirectX::XMVectorGetY(rotatedUp), DirectX::XMVectorGetZ(rotatedUp) };
	}																			   

	DirectX::XMFLOAT3 GetForward() {
		DirectX::XMVECTOR rotatedForward = DirectX::XMVector3Rotate({ 0, 0, 1 }, GetRotationVector());
		return { DirectX::XMVectorGetX(rotatedForward), DirectX::XMVectorGetY(rotatedForward), DirectX::XMVectorGetZ(rotatedForward) };
	}

	// Transformer Methods
	void MoveAbsolute(float x, float y, float z);
	void MoveAbsolute(DirectX::XMFLOAT3 offset);
	void MoveRelative(float x, float y, float z);
	void MoveRelative(DirectX::XMFLOAT3 offset);
	void Rotate(float pitch, float yaw, float roll);
	void Rotate(DirectX::XMFLOAT3 rotation);
	void Scale(float x, float y, float z);
	void Scale(DirectX::XMFLOAT3 scale);

	// Methods
	void CreateWorldMatrix();

private:
	// Matrix transformation data
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 rotation;
	DirectX::XMFLOAT3 scale;

	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 worldInverseTranspose;
};