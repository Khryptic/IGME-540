#include "Material.h"

// Add SRV to textureSRVs map
void Material::AddTextureSRV(std::string resName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv) {
	textureSRVs.insert({ resName, srv });
}

// Add sampler to samplers map
void Material::AddSampler(std::string resName, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler) {
	samplers.insert({ resName, sampler });
}

// Binds the Textures and Samplers
void Material::PrepareMaterial() {

	for (auto& t : textureSRVs) {
		pixelShader->SetShaderResourceView(t.first.c_str(), t.second); 
	}

	for (auto& s : samplers) {
		pixelShader->SetSamplerState(s.first.c_str(), s.second); 
	}
}