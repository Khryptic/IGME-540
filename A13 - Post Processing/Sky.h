#pragma once

#include <d3d11.h>
#include <wrl/module.h>
#include <memory>
#include "Mesh.h"
#include "SimpleShader.h"
#include "WICTextureLoader.h"
#include "Graphics.h"
#include "Camera.h"

class Sky {
public:
	Sky(std::shared_ptr<SimpleVertexShader> vertexShader, std::shared_ptr<SimplePixelShader> pixelShader, 
		std::shared_ptr<Mesh> geometry, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler,
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back);

	void Draw(Camera camera);

	// Helper for creating a cubemap from 6 individual textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateCubemap(
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back);

private:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthOptions;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerOptions;

	std::shared_ptr<Mesh> skyMesh;
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimpleVertexShader> vertexShader;
};