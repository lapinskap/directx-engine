#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include "ReleasePtr.h"

class ColorShader
{
public:
	ColorShader(ID3D11Device* device, HWND hwnd);

	bool Render(ID3D11DeviceContext* deviceContext, int indexCount, DirectX::XMMATRIX worldMatrix, DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix);

private:
	struct MatrixBufferType
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
	};

	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext*, DirectX::XMMATRIX, DirectX::XMMATRIX, DirectX::XMMATRIX);
	void RenderShader(ID3D11DeviceContext*, int);

	ReleasePtr<ID3D11VertexShader> _vertexShader;
	ReleasePtr<ID3D11PixelShader> _pixelShader;
	ReleasePtr<ID3D11InputLayout> _layout;
	ReleasePtr<ID3D11Buffer> _matrixBuffer;
};
