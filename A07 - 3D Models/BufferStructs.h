#pragma once
#include <DirectXMath.h>
#include "Camera.h"

// Custom Buffer struct for constant buffers
struct BufferStructs
{
	DirectX::XMFLOAT4 colorTint;	    // The tint of the object to be drawn
	DirectX::XMFLOAT4X4 world;			// The world matrix
	DirectX::XMFLOAT4X4 view;			// Camera view matrix
	DirectX::XMFLOAT4X4 projection;		// Camera projection matrix
};