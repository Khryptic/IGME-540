#include "GameEntity.h"
#include "Graphics.h"

// Draw Entity
void GameEntity::Draw(std::shared_ptr<Camera> camera) {
	// Prepare Material
	material->PrepareMaterial();
	
	// Fill Shaders
	std::shared_ptr<SimpleVertexShader> vertexShader = material->GetVS();
	std::shared_ptr<SimplePixelShader> pixelShader = material->GetPS();
	transform->CreateWorldMatrix();

	pixelShader->SetFloat4("colorTint", material->GetTint());
	pixelShader->SetFloat2("scale", {material->GetUVScale().at(0), material->GetUVScale().at(1)});
	pixelShader->SetFloat2("offset", {material->GetUVOffset().at(0), material->GetUVOffset().at(1)});
	vertexShader->SetMatrix4x4("world", transform->GetWorldMatrix());
	vertexShader->SetMatrix4x4("view", camera->GetViewMatrix());
	vertexShader->SetMatrix4x4("projection", camera->GetProjectionMatrix());

	// Copy Buffer Data
	vertexShader->CopyAllBufferData();
	pixelShader->CopyAllBufferData();

	// Activate Shaders
	material->GetVS()->SetShader();
	material->GetPS()->SetShader();

	// Draw Mesh
	mesh->Draw();
}