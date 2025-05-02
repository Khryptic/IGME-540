#pragma once
#include <d3d11.h>
#include <DirectXMath.h>

#define LIGHT_TYPE_DIRECTION 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

using namespace DirectX;

struct Light {
	int Type;				// Which kind of light? Direction, Point, Spot?
	XMFLOAT3 Direction;		// Directional and Spot lights need a direction
	float Range;			// Point and Spot lights have a max range for attenuation
	XMFLOAT3 Position;		// Point and Spot lights have a position in space
	float Intensity;		// All lights need an intensity
	XMFLOAT3 Color;			// All lights need a color
	float SpotInnerAngle;	// Inner cone angle (in radians) – Inside this, full light!
	float SpotOuterAngle;	// Outer cone angle (radians) – Outside this, no light!
	XMFLOAT2 Padding;		// Purposefully padding to hit the 16-byte boundary
};