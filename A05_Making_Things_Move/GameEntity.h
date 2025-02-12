#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>
#include "Mesh.h"
#include "Transform.h"
#include "BufferStructs.h"

class GameEntity {
public:
	// Constructor
	GameEntity(Mesh entityMesh) {
		mesh = std::make_shared<Mesh>(entityMesh);
		transform = std::make_shared<Transform>();
	}

	// There is no need for a destructor

	// Getters
	std::shared_ptr<Mesh> GetMesh() { return mesh; }
	std::shared_ptr<Transform> GetTransform() { return transform; }

	// Methods
	void Draw(Microsoft::WRL::ComPtr<ID3D11Buffer> constBuffer, BufferStructs constBufferStruct);

private:
	// Entity Data
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Mesh> mesh;
};
