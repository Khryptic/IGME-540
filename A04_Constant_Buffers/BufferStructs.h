#pragma once

#include <DirectXMath.h>

// Custom Buffer struct for constant buffers
struct BufferStructs
{
	DirectX::XMFLOAT4 colorTint;	    // The tint of the object to be drawn
	DirectX::XMFLOAT3 offset;        // The offset of the geometry
};