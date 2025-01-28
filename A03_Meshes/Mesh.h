#pragma once

#include <d3d11.h>
#include <wrl/client.h>

class Mesh
{
public:
	// Basic OOP Setup
	Mesh(unsigned int vertexCount, unsigned int indexCount);	// Constructor
	~Mesh();								// Destructor will not need code due to smart pointers
	Mesh(const Mesh&) = delete;				// Remove copy constructor
	Mesh& operator=(const Mesh&) = delete;	// Remove copy-assignment operator


	// Getters
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	unsigned int GetIndexCount();
	unsigned int GetVertexCount();
	void Draw();

private:
	// Buffers
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	// Mesh data
	unsigned int indexCount;	// Used when drawing
	unsigned int vertexCount;	// Good for the UI
};

