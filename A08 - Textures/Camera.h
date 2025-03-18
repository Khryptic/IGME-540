#pragma once
#include "Transform.h"
#include <d3d11.h>
#include <DirectXMath.h>

class Camera {
public:
	// Enum for camera projection
	enum Projection {
		perpective = 0,
		orthograpic = 1
	};

	// Constructor
	Camera(DirectX::XMFLOAT3 initialPos, float aspectRatio, float fovDeg) {
		// Initialize 
		fovRad = DirectX::XMConvertToRadians(fovDeg);
		nearPlane = 0.1f;
		farPlane = 1000.0f;
		moveSpeed = 6.0f;
		mouseSpeed = 0.003f;
		currectProjection = Projection::perpective;

		// Initialize Position
		transform.SetPosition(initialPos);

		// Initialize Matrices
		UpdateViewMatrix();
		UpdateProjectionMatrix(aspectRatio);
	}

	// Getters
	DirectX::XMFLOAT4X4 GetViewMatrix() const {
		return viewMatrix;
	}

	DirectX::XMFLOAT4X4 GetProjectionMatrix() const {
		return projectionMatrix;
	}

	float GetFOV() const {
		return DirectX::XMConvertToDegrees(fovRad);
	}

	float GetNearPlane() const {
		return nearPlane;
	}

	float GetFarPlane() const {
		return farPlane;
	}

	float GetMovementSpeed() const{
		return moveSpeed;
	}

	float GetCameraSpeed() const {
		return mouseSpeed;
	}

	Camera::Projection GetProjectionType() const {
		return currectProjection;
	}

	DirectX::XMFLOAT3 GetPosition() const {
		return transform.GetPosition();
	}

	DirectX::XMFLOAT3 GetRotation() const {
		return transform.GetRotation();
	}

	DirectX::XMFLOAT3 GetScale() const {
		return transform.GetScale();
	}

	// Functions
	void UpdateViewMatrix();
	void UpdateProjectionMatrix(float aspectRatio);
	void Update(float deltaTime);

private:
	// Camera requirements
	Transform transform = Transform();
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