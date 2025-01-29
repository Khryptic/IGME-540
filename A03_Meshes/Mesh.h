#pragma once

#include <d3d11.h>
#include <wrl/client.h>

class Mesh
{
public:
	// Basic OOP Setup
	Mesh(const char* name, unsigned int vertexCount, unsigned int indexCount,
		struct Vertex vertices[], unsigned int indices[]);	// Constructor
	~Mesh();								// Destructor
	Mesh(const Mesh&) = delete;				// Remove copy constructor
	Mesh& operator=(const Mesh&) = delete;	// Remove copy-assignment operator


	// Getters
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer() { return vertexBuffer; }
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer() { return indexBuffer; }

	unsigned int GetIndexCount() { return indexCount; }
	unsigned int GetVertexCount() { return vertexCount; }
	const char* GetName() { return name; }

	void Draw();

private:
	// Buffers
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	// Mesh data
	unsigned int indexCount;	// Used when drawing
	unsigned int vertexCount;	// Good for the UI
	const char* name;			// Name of Mesh
};

