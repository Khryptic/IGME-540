#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>
#include "Mesh.h"
#include "Transform.h"
#include "Camera.h"
#include "Material.h"

class GameEntity {
public:
	// Constructor
	GameEntity(std::shared_ptr<Mesh> entityMesh, std::shared_ptr<Material> material) {
		mesh = entityMesh;
		transform = std::make_shared<Transform>();
		this->material = material;
	}

	// Getters
	std::shared_ptr<Mesh> GetMesh() { return mesh; }
	std::shared_ptr<Transform> GetTransform() { return transform; }
	std::shared_ptr<Material> GetMaterial() { return material; }

	// Methods
	void Draw(std::shared_ptr<Camera> camera);

private:
	// Entity Data
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;
};
