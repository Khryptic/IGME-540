#include "GameEntity.h"
#include "Graphics.h"

// Draw Entity
void GameEntity::Draw(std::shared_ptr<Camera> camera) {

	// Fill Vertex Shader
	std::shared_ptr<SimpleVertexShader> vertexShader = material->getVS();
	transform->CreateWorldMatrix();

	vertexShader->SetFloat4("colorTint", material->getTint());
	vertexShader->SetMatrix4x4("world", transform->GetWorldMatrix());
	vertexShader->SetMatrix4x4("view", camera->GetViewMatrix());
	vertexShader->SetMatrix4x4("projection", camera->GetProjectionMatrix());

	// Copy Buffer Data
	vertexShader->CopyAllBufferData();

	// Activate Shaders
	material->getVS()->SetShader();
	material->getPS()->SetShader();

	// Draw Mesh
	mesh->Draw();
}