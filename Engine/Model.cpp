#include "Model.h"
#include "D3D.h"
#include "Common.h"

struct VertexType
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 texture;
};

Model::Model(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* textureFilename)
{
	InitializeBuffers(device, deviceContext, textureFilename);
	LoadTexture(device, deviceContext, textureFilename);
}

void Model::Render(ID3D11DeviceContext* deviceContext)
{
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(deviceContext);
}

int Model::GetIndexCount()
{
	return _indexCount;
}

void Model::InitializeBuffers(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* textureFilename)
{
	HRESULT result;

	// Set the number of vertices in the vertex array.
	_vertexCount = 4;

	// Set the number of indices in the index array.
	_indexCount = 6;

	// Create the vertex array.
	std::vector<VertexType> vertices(_vertexCount);

	// Create the index array.
	std::vector<uint> indices(_indexCount);

	// Load the vertex array with data.
	vertices[0].position = DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f);  // Bottom left
	vertices[0].texture = DirectX::XMFLOAT2(0.0f, 0.0f);

	vertices[1].position = DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f);  // Top left
	vertices[1].texture = DirectX::XMFLOAT2(0.0f, 1.0f);

	vertices[2].position = DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f);  // Bottom right
	vertices[2].texture = DirectX::XMFLOAT2(1.0f, 0.0f);

	vertices[3].position = DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f); // Top right
	vertices[3].texture = DirectX::XMFLOAT2(1.0f, 1.0f);

	// Load the index array with data.
	indices[0] = 0;  // Bottom left.
	indices[1] = 1;  // Top left.
	indices[2] = 2;  // Bottom right.
	indices[3] = 1;  // Top left.
	indices[4] = 3;  // Top right.
	indices[5] = 2;  // Bottom right.

	// Set up the description of the static vertex buffer.
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * _vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices.data();
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &_vertexBuffer);
	if (FAILED(result))
		throw D3DError("Failed to create a vertex buffer");

	// Set up the description of the static index buffer.
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(uint) * _indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices.data();
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &_indexBuffer);
	if (FAILED(result))
		throw D3DError("Failed to create an index buffer");
}

ID3D11ShaderResourceView* Model::GetTexture()
{
	return _texture->GetTexture();
}

bool Model::LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* filename)
{
	// Create and initialize the texture object.
	_texture = std::make_unique<Texture>(device, deviceContext, filename);

	return _texture->IsValid();
}

void Model::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	uint stride;
	uint offset;

	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType);
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(_indexBuffer.get(), DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
