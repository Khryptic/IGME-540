#pragma once
#include <d3d11.h>
#include "DirectXMath.h"
#include "SimpleShader.h"
#include "PathHelpers.h"
#include <vector>
#include <memory>
#include "Graphics.h"

class Material
{
public:
	// Constructor
	Material(DirectX::XMFLOAT4 colorTint, 
		std::shared_ptr<SimpleVertexShader> vertexShader, 
		std::shared_ptr<SimplePixelShader> pixelShader) {
		this->colorTint = colorTint;
		this->vertexShader = vertexShader;
		this->pixelShader = pixelShader;
	}

	// Getters
	DirectX::XMFLOAT4 getTint() const {
		return colorTint;
	}

	std::shared_ptr<SimpleVertexShader> getVS() const {
		return vertexShader;
	}

	std::shared_ptr<SimplePixelShader> getPS() const {
		return pixelShader;
	}

	// Setters
	void SetTint(DirectX::XMFLOAT4 tint) {
		this->colorTint = tint;
	}

	void SetVS(std::shared_ptr<SimpleVertexShader> vs) {
		this->vertexShader = vs;
	}

	void SetPS(std::shared_ptr<SimplePixelShader> ps) {
		this->pixelShader = ps;
	}

private:
	DirectX::XMFLOAT4 colorTint;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;
};