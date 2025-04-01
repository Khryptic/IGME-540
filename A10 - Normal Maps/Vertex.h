#pragma once

#include <DirectXMath.h>

// --------------------------------------------------------
// A custom vertex definition
//
// You will eventually ADD TO this, and/or make more of these!
// --------------------------------------------------------
struct Vertex
{
	DirectX::XMFLOAT3 Position;	    // The local position of the vertex
	DirectX::XMFLOAT2 UV;			// UV for the model
	DirectX::XMFLOAT3 Normal;		// Normal for the model
	DirectX::XMFLOAT3 Tangent;		// Tangent for the model
};