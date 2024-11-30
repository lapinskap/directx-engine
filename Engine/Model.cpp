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

	const int GRID_SIZE = 10;
	const int VERTICES_PER_ROW = GRID_SIZE + 1;
	const int VERTICES_PER_COLUMN = GRID_SIZE + 1;

	// Set the number of vertices in the vertex array.
	_vertexCount = VERTICES_PER_ROW * VERTICES_PER_COLUMN;

	// Set the number of indices in the index array.
	_indexCount = GRID_SIZE * GRID_SIZE * 6;

	// Create the vertex array.
	std::vector<VertexType> vertices(_vertexCount);

	// Create the index array.
	std::vector<uint> indices(_indexCount);

	// Initialize vertex array
	for (int row = 0; row < VERTICES_PER_COLUMN; ++row)
	{
		for (int col = 0; col < VERTICES_PER_ROW; ++col)
		{
			int index = row * VERTICES_PER_ROW + col;
			
			vertices[index].position = DirectX::XMFLOAT3((float)col, (float)row, 0.0f);
			vertices[index].texture = DirectX::XMFLOAT2((float)col / GRID_SIZE, (float)row / GRID_SIZE);
		}
	}

	// Initialize index array
	int index = 0;
	for (int row = 0; row < GRID_SIZE; ++row)
	{
		for (int col = 0; col < GRID_SIZE; ++col)
		{
			int topLeft = row * VERTICES_PER_ROW + col;
			int topRight = topLeft + 1;
			int bottomLeft = topLeft + VERTICES_PER_ROW;
			int bottomRight = bottomLeft + 1;

			indices[index++] = topLeft;
			indices[index++] = bottomLeft;
			indices[index++] = topRight;
			indices[index++] = topRight;
			indices[index++] = bottomLeft;
			indices[index++] = bottomRight;
		}
	}

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
