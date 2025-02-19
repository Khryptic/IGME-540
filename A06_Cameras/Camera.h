#pragma once
#include "Transform.h"
#include <d3d11.h>
#include <DirectXMath.h>

class Camera {
public:
	// Constructor
	Camera(DirectX::XMFLOAT3 initialPos, float aspectRatio) {
		// Initialize Values
		fovRad = DirectX::XMConvertToRadians(45.0f);
		nearPlane = 0.1f;
		farPlane = 1.0f;
		moveSpeed = 0.5f;
		mouseSpeed = 0.3f;
		currectProjection = Projection::perpective;

		// Initialize Matrices
		DirectX::XMStoreFloat4x4(&viewMatrix, DirectX::XMMatrixIdentity());
		DirectX::XMStoreFloat4x4(&projectionMatrix, DirectX::XMMatrixIdentity());

		UpdateViewMatrix();
		UpdateProjectionMatrix(aspectRatio);
	}

	// Enum for camera projection
	enum Projection {
		perpective = 0,
		orthograpic = 1
	};

	// Getters
	DirectX::XMFLOAT4X4 GetViewMatrix() const {
		return viewMatrix;
	}

	DirectX::XMFLOAT4X4 GetProjectionMatrix() const {
		return projectionMatrix;
	}

	// Functions
	void UpdateViewMatrix();
	void UpdateProjectionMatrix(float aspectRatio);

private:
	// Camera requirements
	Transform transform;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	// Helper variables
	float fovRad;
	float nearPlane;
	float farPlane;
	float moveSpeed;
	float mouseSpeed;
	Projection currectProjection;
};