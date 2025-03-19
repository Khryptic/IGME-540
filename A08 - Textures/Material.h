#pragma once
#include <d3d11.h>
#include "DirectXMath.h"
#include "SimpleShader.h"
#include "PathHelpers.h"
#include <vector>
#include <memory>
#include "Graphics.h"
#include <unordered_map>

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
		scale = { 1, 1 };
		offset = { 0, 0 };
	}

	// Helper Methods
	void AddTextureSRV(std::string resName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);
	void AddSampler(std::string resName, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);
	void PrepareMaterial();

	// Getters
	DirectX::XMFLOAT4 GetTint() const {
		return colorTint;
	}

	std::shared_ptr<SimpleVertexShader> GetVS() const {
		return vertexShader;
	}

	std::shared_ptr<SimplePixelShader> GetPS() const {
		return pixelShader;
	}

	std::vector<float> GetUVScale() const {
		return scale;
	}

	std::vector<float> GetUVOffset() const {
		return offset;
	}

	int GetTextureCount() const {
<<<<<<< HEAD
		return (int) textureSRVs.size();
=======
		return (int)textureSRVs.size();
>>>>>>> 2dd67e75d2a1a323cf1df44cdfc715d12404704f
	}

	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> GetTextureSRVs() const {
		return textureSRVs;
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

	void SetUVScale(std::vector<float> scale) {
		this->scale = scale;
	}

	void SetUVOffset(std::vector<float> offset) {
		this->offset = offset;
	}

private:
	DirectX::XMFLOAT4 colorTint;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;

	// Maps for Material Textures
	std::vector<float> scale;
	std::vector<float> offset;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;
};