#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <fstream>
#include <stdexcept>
#include <vector>
#include "Vertex.h"

class Mesh
{
public:
	// Basic OOP Setup
	// Mesh Constructor for primitives
	Mesh(const char* name, unsigned int vertexCount, unsigned int indexCount,
		struct Vertex vertices[], unsigned int indices[]);

	// Mesh Constructor for Obj Imports
	Mesh(const char* name, const char* fileName);

	~Mesh();								// Destructor
	Mesh& operator=(const Mesh&) = delete;	// Remove copy-assignment operator


	// Getters
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer() const { return vertexBuffer; }
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer() const { return indexBuffer; }

	unsigned int GetIndexCount() const { return indexCount; }
	unsigned int GetVertexCount() const { return vertexCount; }
	const char* GetName() const { return name; }

	void Draw();

	// Calculate Tangents
	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);

	// Creates A Vertex and An Index Buffer
	void CreateBuffers(const char* name, unsigned int vertexCount, unsigned int indexCount,
		struct Vertex vertices[], unsigned int indices[]);

private:
	// Buffers
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	// Mesh data
	unsigned int indexCount;	// Used when drawing
	unsigned int vertexCount;	// Good for the UI
	const char* name;			// Name of Mesh

};
