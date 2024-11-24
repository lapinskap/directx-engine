#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>

#include "Common.h"
#include "ReleasePtr.h"

class TextureShader
{
public:
	TextureShader(ID3D11Device* device, HWND hwnd);

	bool Render(ID3D11DeviceContext* deviceContext, int indexCount, DirectX::XMMATRIX worldMatrix, DirectX::XMMATRIX viewMatrix,
		DirectX::XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture);

private:
	
	void InitializeShader(ID3D11Device* device, HWND hwnd, const char* vsFilename, const char* psFilename);

	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, DirectX::XMMATRIX worldMatrix, DirectX::XMMATRIX viewMatrix,
		DirectX::XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture);
	void RenderShader(ID3D11DeviceContext* deviceContext, uint indexCount);

	ReleasePtr<ID3D11VertexShader> _vertexShader;
	ReleasePtr<ID3D11PixelShader> _pixelShader;
	ReleasePtr<ID3D11InputLayout> _layout;
	ReleasePtr<ID3D11Buffer> _matrixBuffer;
	ReleasePtr<ID3D11SamplerState> _sampleState;
};