#include "GameEntity.h"
#include "Graphics.h"

// Draw Entity
void GameEntity::Draw(Microsoft::WRL::ComPtr<ID3D11Buffer> constBuffer, BufferStructs constBufferStruct) {
	// Bind Constant Buffer to Pipeline
	Graphics::Context->VSSetConstantBuffers(0, 1, constBuffer.GetAddressOf());

	// Get World Matrix
	transform->CreateWorldMatrix();
	constBufferStruct.world = transform->GetWorldMatrix();

	// Send new data to Constant Buffer
	D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	Graphics::Context->Map(constBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
	memcpy(mappedBuffer.pData, &constBufferStruct, sizeof(constBufferStruct));
	Graphics::Context->Unmap(constBuffer.Get(), 0);

	// Draw Mesh
	mesh->Draw();
}