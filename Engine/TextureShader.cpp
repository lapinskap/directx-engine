#define _CRT_SECURE_NO_WARNINGS
#include "TextureShader.h"
#include "Common.h"
#include "D3D.h"

struct MatrixBufferType
{
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;
};

TextureShader::TextureShader(ID3D11Device* device, HWND hwnd)
{
	// Initialize the vertex and pixel shaders.
	InitializeShader(device, hwnd, "../Engine/texture.vs", "../Engine/texture.ps");
}

bool TextureShader::Render(ID3D11DeviceContext* deviceContext, int indexCount, DirectX::XMMATRIX worldMatrix, DirectX::XMMATRIX viewMatrix,
	DirectX::XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture)
{
	// Set the shader parameters that it will use for rendering.
	bool result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture);
	if (!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderShader(deviceContext, indexCount);

	return true;
}

void TextureShader::InitializeShader(ID3D11Device* device, HWND hwnd, const char* vsFilename, const char* psFilename)
{
	HRESULT result;
	ReleasePtr<ID3D10Blob> errorMessage;
	ReleasePtr<ID3D10Blob> vertexShaderBuffer;
	ReleasePtr<ID3D10Blob> pixelShaderBuffer;

	WCHAR vsFilenameW[128];
	WCHAR psFilenameW[128];

	// Set the filename of the vertex shader.
	std::mbstowcs(vsFilenameW, vsFilename, 128);

	// Set the filename of the pixel shader.
	std::mbstowcs(psFilenameW, psFilename, 128);

	// Compile the vertex shader code.
	result = D3DCompileFromFile(vsFilenameW, nullptr, nullptr, "TextureVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&vertexShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage)
			throw D3DError(std::format("Error compiling shader\n{}", errorMessage->GetBufferPointer()));
		// If there was nothing in the error message then it simply could not find the shader file itself.
		else
			throw D3DError("Missing shader file");
	}

	// Compile the pixel shader code.
	result = D3DCompileFromFile(psFilenameW, nullptr, nullptr, "TexturePixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&pixelShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage)
			throw D3DError(std::format("Error compiling shader\n{}", errorMessage->GetBufferPointer()));
		// If there was nothing in the error message then it simply could not find the shader file itself.
		else
			throw D3DError("Missing shader file");
	}

	// Create the vertex shader from the buffer.
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), nullptr, &_vertexShader);
	if (FAILED(result))
		throw D3DError("Failed to create a vertex shader");

	// Create the pixel shader from the buffer.
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), nullptr, &_pixelShader);
	if (FAILED(result))
		throw D3DError("Failed to create a pixel shader");

	const uint numElements = 2;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[numElements];

	// Create the vertex input layout description.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	// Create the vertex input layout.
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), &_layout);
	if (FAILED(result))
		throw D3DError("Failed to create an input layout");

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&matrixBufferDesc, nullptr, &_matrixBuffer);
	if (FAILED(result))
		throw D3DError("Failed to create a matrix buffer");

	// Create a texture sampler state description.
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	result = device->CreateSamplerState(&samplerDesc, &_sampleState);
	if (FAILED(result))
		throw D3DError("Failed to create a texture sampler");
}

bool TextureShader::SetShaderParameters(ID3D11DeviceContext* deviceContext, DirectX::XMMATRIX worldMatrix, DirectX::XMMATRIX viewMatrix,
	DirectX::XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture)
{
	// Transpose the matrices to prepare them for the shader.
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	// Lock the constant buffer so it can be written to.
	HRESULT result = deviceContext->Map(_matrixBuffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		throw D3DError("Failed to map the matrix buffer");

	// Get a pointer to the data in the constant buffer.
	MatrixBufferType* dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	// Unlock the constant buffer.
	deviceContext->Unmap(_matrixBuffer.get(), 0);

	// Set the position of the constant buffer in the vertex shader.
	const uint bufferNumber = 0u;

	// Finanly set the constant buffer in the vertex shader with the updated values.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &_matrixBuffer);

	// Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(bufferNumber, 1, &texture);

	return true;
}

void TextureShader::RenderShader(ID3D11DeviceContext* deviceContext, uint indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(_layout.get());

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(_vertexShader.get(), nullptr, 0);
	deviceContext->PSSetShader(_pixelShader.get(), nullptr, 0);

	// Set the sampler state in the pixel shader.
	deviceContext->PSSetSamplers(0, 1, &_sampleState);

	// Render the triangle.
	deviceContext->DrawIndexed(indexCount, 0, 0);
}