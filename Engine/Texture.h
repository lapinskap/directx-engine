#pragma once

#include <d3d11.h>
#include "Common.h"
#include "ReleasePtr.h"

class Texture
{
public:

	Texture(ID3D11Device* device, ID3D11DeviceContext* context, const char* filename);

	bool IsValid() const;
	ID3D11ShaderResourceView* GetTexture();
	ushort GetWidth() const;
	ushort GetHeight() const;

private:

	std::vector<uchar> LoadTarga32Bit(const char* filename);

	ReleasePtr<ID3D11Texture2D> _texture;
	ReleasePtr<ID3D11ShaderResourceView> _textureView;
	ushort _width = 0u;
	ushort _height = 0u;
	bool _isValid = false;
};