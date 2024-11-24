#pragma once

#include <d3d11.h>
#include <directxmath.h>
#include "ReleasePtr.h"
#include "Texture.h"

class Model
{
public:

	Model(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* textureFilename);

	void Render(ID3D11DeviceContext* deviceContext);

	int GetIndexCount();

	ID3D11ShaderResourceView* GetTexture();

private:

	void InitializeBuffers(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* textureFilename);
	void RenderBuffers(ID3D11DeviceContext* deviceContext);

	bool LoadTexture(ID3D11Device*, ID3D11DeviceContext*, const char* filename);

	ReleasePtr<ID3D11Buffer> _vertexBuffer;
	ReleasePtr<ID3D11Buffer> _indexBuffer;
	int _vertexCount = 0;
	int _indexCount = 0;
	std::unique_ptr<Texture> _texture;
};