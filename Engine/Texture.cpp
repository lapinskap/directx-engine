#include "Texture.h"
#include <stdio.h>

Texture::Texture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* filename)
{
	std::vector<uchar> targaData = LoadTarga32Bit(filename);
	if (targaData.empty())
		return;

	// Initialize the texture description.
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	// Setup the texture description.
	textureDesc.Width = _width;
	textureDesc.Height = _height;
	textureDesc.MipLevels = 0u;
	textureDesc.ArraySize = 1u;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1u;
	textureDesc.SampleDesc.Quality = 0u;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0u;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	// Create the empty texture.
	HRESULT hresult = device->CreateTexture2D(&textureDesc, nullptr, &_texture);
	if (FAILED(hresult))
		return;

	// Set the row pitch of the targa image data.
	uint rowPitch = (_width * 4) * sizeof(uchar);

	// Copy the targa image data into the texture.
	deviceContext->UpdateSubresource(_texture.get(), 0u, nullptr, targaData.data(), rowPitch, 0u);

	// Setup the shader resource view description.
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0u;
	srvDesc.Texture2D.MipLevels = (uint) - 1;

	// Create the shader resource view for the texture.
	hresult = device->CreateShaderResourceView(_texture.get(), &srvDesc, &_textureView);
	if (FAILED(hresult))
		return;

	// Generate mipmaps for this texture.
	deviceContext->GenerateMips(_textureView.get());

	_isValid = true;
}

ID3D11ShaderResourceView* Texture::GetTexture()
{
	return _textureView.get();
}

std::vector<uchar> Texture::LoadTarga32Bit(const char* filename)
{
	struct TargaHeader
	{
		uint8_t data1[12];
		uint16_t width;
		uint16_t height;
		uint8_t bpp;
		uint8_t data2;
	};


	FILE* filePtr = nullptr;
	size_t countRead = 0u;

	// Open the targa file for reading in binary.
	int error = fopen_s(&filePtr, filename, "rb");
	if (error != 0)
		return std::vector<uchar>();

	// Read in the file header.
	TargaHeader targaFileHeader;
	countRead = fread(&targaFileHeader, sizeof(TargaHeader), 1, filePtr);
	if (countRead != 1u)
	{
		fclose(filePtr);
		return std::vector<uchar>();
	}

	// Get the important information from the header.
	_height = targaFileHeader.height;
	_width = targaFileHeader.width;
	uchar bpp = targaFileHeader.bpp;

	// Check that it is 32 bit and not 24 bit.
	if (bpp != 32u)
	{
		fclose(filePtr);
		return std::vector<uchar>();
	}

	// Calculate the size of the 32 bit image data.
	uint imageSize = _width * _height * 4;

	// Allocate memory for the targa image data.
	std::vector<uchar> targaImage(imageSize);

	// Read in the targa image data.
	countRead = fread(targaImage.data(), 1u, imageSize, filePtr);
	if (countRead != imageSize)
	{
		fclose(filePtr);
		return std::vector<uchar>();
	}

	// Close the file.
	error = fclose(filePtr);
	if (error != 0)
		return std::vector<uchar>();

	// Allocate memory for the targa destination data.
	std::vector<uchar> targaData(imageSize);

	// Initialize the index into the targa destination data array.
	uint index = 0u;

	// Initialize the index into the targa image data.
	uint k = (_width * _height * 4) - (_width * 4);

	// Now copy the targa image data into the targa destination array in the correct order since the targa format is stored upside down and also is not in RGBA order.
	for (uint j = 0; j < _height; j++)
	{
		for (uint i = 0; i < _width; i++)
		{
			targaData[index + 0] = targaImage[k + 2];  // Red
			targaData[index + 1] = targaImage[k + 1];  // Green
			targaData[index + 2] = targaImage[k + 0];  // Blue
			targaData[index + 3] = targaImage[k + 3];  // Alpha

			// Increment the indexes into the targa data.
			k += 4u;
			index += 4u;
		}

		// Set the targa image data index back to the preceding row at the beginning of the column since its reading it in upside down.
		k -= (_width * 8u);
	}

	return targaData;
}

bool Texture::IsValid() const
{
	return _isValid;
}

ushort Texture::GetWidth() const
{
    return _width;
}

ushort Texture::GetHeight() const
{
    return _height;
}
