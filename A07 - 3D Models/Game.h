#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include "Mesh.h"
#include <vector>
#include "GameEntity.h"

class Game
{
public:
	// Basic OOP setup
	Game() = default;
	~Game();
	Game(const Game&) = delete; // Remove copy constructor
	Game& operator=(const Game&) = delete; // Remove copy-assignment operator

	// Primary functions
	void Initialize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void OnResize();

	// Helper Function
	void ImGuiRefresh(float deltaTime);
	void BuildUI();

private:

	// Initialization helper methods
	void CreateGeometry();

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	// Shaders and shader-related constructs

	// Shared Pointers
	std::shared_ptr<Mesh> triangle;
	std::shared_ptr<Mesh> box;
	std::shared_ptr<Mesh> crown;

	// Vector to hold pointers
	std::vector<GameEntity> meshes;
};

