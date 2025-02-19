#include "Camera.h"
#include <DirectXMath.h>
#include "Input.h"

// Updates the View Matrix
void Camera::UpdateViewMatrix() {
	DirectX::XMStoreFloat4x4(&viewMatrix ,DirectX::XMMatrixLookToLH({ transform.GetPosition().x, transform.GetPosition().y, transform.GetPosition().z },
							  { transform.GetForward().x, transform.GetForward().y, transform.GetForward().z },
							  { 0, 1, 0}));
}

// Updates the Projection Matrix
void Camera::UpdateProjectionMatrix(float aspectRatio) {
	// Perspective Projection
	if (currectProjection == Projection::perpective) {
		DirectX::XMStoreFloat4x4(&projectionMatrix, DirectX::XMMatrixPerspectiveFovLH(fovRad, aspectRatio, nearPlane, farPlane));
	}

	// Orthographic Projection
	else if (currectProjection == Projection::orthograpic) {
		DirectX::XMStoreFloat4x4(&projectionMatrix, DirectX::XMMatrixOrthographicLH(-1, 1, nearPlane, farPlane));
	}
}

// Updates per frame
void Camera::Update(float deltaTime) {
	// Keyboard Inputs
	if (Input::KeyDown('W')) {
		transform.MoveRelative(0.0f, 0.0f, deltaTime * moveSpeed);
	}

	if (Input::KeyDown('S')) {
		transform.MoveRelative(0.0f, 0.0f, -deltaTime * moveSpeed);
	}

	if (Input::KeyDown('A')) {
		transform.MoveRelative(-deltaTime * moveSpeed, 0.0f, 0.0f);
	}

	if (Input::KeyDown('D')) {
		transform.MoveRelative(deltaTime * moveSpeed, 0.0f, 0.0f);
	}

	if (Input::KeyDown(' ')) {
		transform.MoveAbsolute(0.0f, deltaTime * moveSpeed, 0.0f);
	}

	if (Input::KeyDown('X')) {
		transform.MoveAbsolute(0.0f, -deltaTime * moveSpeed, 0.0f);
	}

	// Mouse Inputs
	if (Input::MouseLeftDown()) {
		float cursorMovementX = (float)Input::GetMouseXDelta() * mouseSpeed;
		float cursorMovementY = (float)Input::GetMouseYDelta() * mouseSpeed;

		transform.Rotate((float)cursorMovementY, (float)cursorMovementX, 0.0f);

		// Clamping X rotation
		if (transform.GetRotation().x >= DirectX::g_XMHalfPi[0] ) {
			transform.SetRotation(DirectX::g_XMHalfPi[0] - FLT_EPSILON, 0, 0 );
		}

		else if (transform.GetRotation().x <= -DirectX::g_XMHalfPi[0]) {
			transform.SetRotation(-DirectX::g_XMHalfPi[0] + FLT_EPSILON, 0, 0);
		}

		// Clamping Y rotation
		if (transform.GetRotation().y >= DirectX::g_XMHalfPi[0]) {
			transform.SetRotation(0, DirectX::g_XMHalfPi[0] - FLT_EPSILON, 0);
		}

		else if (transform.GetRotation().y <= -DirectX::g_XMHalfPi[0]) {
			transform.SetRotation(0, -DirectX::g_XMHalfPi[0] + FLT_EPSILON, 0);
		}
	}

	UpdateViewMatrix();
}