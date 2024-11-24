#include "D3D.h"

#define _USE_MATH_DEFINES
#include <math.h>

D3D::D3D(const InitParams& initParams)
{
	// Store the vsync setting.
	_vsyncEnabled = initParams.vsync;
	uint numerator = 0u;
	uint denominator = 0u;
	InitVideoCardInfo(initParams, numerator, denominator);
	InitDeviceAndSwapChain(initParams, numerator, denominator);
	InitRenderTargetView();
	InitDepthStencilBuffer(initParams);
	InitDepthStencilState();
	InitDepthStencilView();
	InitRasterState();
	InitViewport(initParams);
}

void D3D::InitVideoCardInfo(const InitParams& initParams, uint& numerator, uint& denominator)
{
	HRESULT result;

	// Create a DirectX graphics interface factory.
	ReleasePtr<IDXGIFactory> factory;
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(result))
		throw D3DError("Failed to create DXGIFactory");

	// Use the factory to create an adapter for the primary graphics interface (video card).
	ReleasePtr<IDXGIAdapter> adapter;
	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result))
		throw D3DError("Failed to enum adapters");

	// Enumerate the primary adapter output (monitor).
	ReleasePtr<IDXGIOutput> adapterOutput;
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result))
		throw D3DError("Failed to enum adapter outputs");

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	uint numModes = 0u;
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr);
	if (FAILED(result))
		throw D3DError("Failed to get num of display modes");

	// Create a list to hold all the possible display modes for this monitor/video card combination.
	std::vector<DXGI_MODE_DESC> displayModes(numModes);

	// Now fill the display mode list structures.
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModes.data());
	if (FAILED(result))
		throw D3DError("Failed to get display modes");

	// Now go through all the display modes and find the one that matches the screen width and height.
	// When a match is found store the numerator and denominator of the refresh rate for that monitor.
	for (uint i = 0; i < numModes; i++)
	{
		if (displayModes[i].Width == initParams.screenWidth)
		{
			if (displayModes[i].Height == initParams.screenHeight)
			{
				numerator = displayModes[i].RefreshRate.Numerator;
				denominator = displayModes[i].RefreshRate.Denominator;
			}
		}
	}

	// Get the adapter (video card) description.
	DXGI_ADAPTER_DESC adapterDesc;
	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
		throw D3DError("Failed to get adapter desc");

	// Store the dedicated video card memory in megabytes.
	_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// Convert the name of the video card to a character array and store it.
	size_t stringLength;
	char videoCardDescription[128];
	int error = wcstombs_s(&stringLength, videoCardDescription, 128, adapterDesc.Description, 128);
	if (!error)
	{
		_videoCardDescription = videoCardDescription;
	}
}

void D3D::InitDeviceAndSwapChain(const InitParams& initParams, uint numerator, uint denominator)
{
	// Set the refresh rate of the back buffer.
	DXGI_SWAP_CHAIN_DESC swapChainDesc;

	// Initialize the swap chain description.
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// Set to a single back buffer.
	swapChainDesc.BufferCount = 1;

	// Set the width and height of the back buffer.
	swapChainDesc.BufferDesc.Width = initParams.screenWidth;
	swapChainDesc.BufferDesc.Height = initParams.screenHeight;

	// Set regular 32-bit surface for the back buffer.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	if (_vsyncEnabled)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	// Set the usage of the back buffer.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// Set the handle for the window to render to.
	swapChainDesc.OutputWindow = initParams.hwnd;

	// Turn multisampling off.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// Set to full screen or windowed mode.
	swapChainDesc.Windowed = !initParams.fullscreen;

	// Set the scan line ordering and scaling to unspecified.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Discard the back buffer contents after presenting.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// Don't set the advanced flags.
	swapChainDesc.Flags = 0;

	// Set the feature level to DirectX 11.
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	// Create the swap chain, Direct3D device, and Direct3D device context.
	 HRESULT result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, &_swapChain, &_device, nullptr, &_deviceContext);

	if (FAILED(result))
		throw D3DError("Failed to create device and swap chain");
}

void D3D::InitRenderTargetView()
{
	HRESULT result;

	// Get the pointer to the back buffer.
	ReleasePtr<ID3D11Texture2D> backBuffer;
	result = _swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
	if (FAILED(result))
		throw D3DError("Failed to get buffer");

	// Create the render target view with the back buffer pointer.
	result = _device->CreateRenderTargetView(backBuffer.get(), NULL, &_renderTargetView);
	if (FAILED(result))
		throw D3DError("Failed to create render target view");
}

void D3D::InitDepthStencilBuffer(const InitParams& initParams)
{
	HRESULT result;

	// Initialize the description of the depth buffer.
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = initParams.screenWidth;
	depthBufferDesc.Height = initParams.screenHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create the texture for the depth buffer using the filled out description.
	result = _device->CreateTexture2D(&depthBufferDesc, NULL, &_depthStencilBuffer);
	if (FAILED(result))
		throw D3DError("Failed to create texture");
}

void D3D::InitDepthStencilState()
{
	HRESULT result;

	// Initialize the description of the stencil state.
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the depth stencil state.
	result = _device->CreateDepthStencilState(&depthStencilDesc, &_depthStencilState);
	if (FAILED(result))
		throw D3DError("Failed to create depth stencil state");

	// Set the depth stencil state.
	_deviceContext->OMSetDepthStencilState(_depthStencilState.get(), 1);
}

void D3D::InitDepthStencilView()
{
	// Initialize the depth stencil view.
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	HRESULT result = _device->CreateDepthStencilView(_depthStencilBuffer.get(), &depthStencilViewDesc, &_depthStencilView);
	if (FAILED(result))
		throw D3DError("Failed to create depth stencil view");

	// Bind the render target view and depth stencil buffer to the output render pipeline.
	ID3D11RenderTargetView* renderTargetViewArray[1]{ _renderTargetView.get() };
	_deviceContext->OMSetRenderTargets(1, renderTargetViewArray, _depthStencilView.get());
}

void D3D::InitRasterState()
{
	HRESULT result;

	// Setup the raster description which will determine how and what polygons will be drawn.
	D3D11_RASTERIZER_DESC rasterDesc;
	ZeroMemory(&rasterDesc, sizeof(rasterDesc));

	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	result = _device->CreateRasterizerState(&rasterDesc, &_rasterState);
	if (FAILED(result))
		throw D3DError("Failed to create rasterizer state");

	// Now set the rasterizer state.
	_deviceContext->RSSetState(_rasterState.get());
}

void D3D::InitViewport(const InitParams& initParams)
{
	float screenWidth = (float)initParams.screenWidth;
	float screenHeight = (float)initParams.screenHeight;

	// Setup the viewport for rendering.
	_viewport.Width = screenWidth;
	_viewport.Height = screenHeight;
	_viewport.MinDepth = 0.0f;
	_viewport.MaxDepth = 1.0f;
	_viewport.TopLeftX = 0.0f;
	_viewport.TopLeftY = 0.0f;

	// Create the viewport.
	_deviceContext->RSSetViewports(1, &_viewport);

	// Setup the projection matrix.
	float fieldOfView = (float)M_PI / 4.0f;
	float screenAspect = screenWidth / screenHeight;

	// Create the projection matrix for 3D rendering.
	_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, initParams.screenNear, initParams.screenFar);

	// Initialize the world matrix to the identity matrix.
	_worldMatrix = DirectX::XMMatrixIdentity();

	// Create an orthographic projection matrix for 2D rendering.
	_orthoMatrix = DirectX::XMMatrixOrthographicLH(screenWidth, screenHeight, initParams.screenNear, initParams.screenFar);
}

D3D::~D3D()
{
	// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	if (_swapChain)
	{
		_swapChain->SetFullscreenState(false, nullptr);
	}
}

void D3D::BeginScene(float red, float green, float blue, float alpha)
{
	// Setup the color to clear the buffer to.
	float color[4] = {red, green, blue, alpha};

	// Clear the back buffer.
	_deviceContext->ClearRenderTargetView(_renderTargetView.get(), color);

	// Clear the depth buffer.
	_deviceContext->ClearDepthStencilView(_depthStencilView.get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}


void D3D::EndScene()
{
	// Present the back buffer to the screen since rendering is complete.
	if (_vsyncEnabled)
	{
		// Lock to screen refresh rate.
		_swapChain->Present(1, 0);
	}
	else
	{
		// Present as fast as possible.
		_swapChain->Present(0, 0);
	}
}

ID3D11Device* D3D::GetDevice()
{
	return _device.get();
}

ID3D11DeviceContext* D3D::GetDeviceContext()
{
	return _deviceContext.get();
}

void D3D::GetProjectionMatrix(DirectX::XMMATRIX& projectionMatrix)
{
	projectionMatrix = _projectionMatrix;
}

void D3D::GetWorldMatrix(DirectX::XMMATRIX& worldMatrix)
{
	worldMatrix = _worldMatrix;
}

void D3D::GetOrthoMatrix(DirectX::XMMATRIX& orthoMatrix)
{
	orthoMatrix = _orthoMatrix;
}

const std::string& D3D::GetVideoCardInfo() const
{
	return _videoCardDescription;
}

void D3D::SetBackBufferRenderTarget()
{
	// Bind the render target view and depth stencil buffer to the output render pipeline.
	ID3D11RenderTargetView* renderTargetViewArray[1]{ _renderTargetView.get() };
	_deviceContext->OMSetRenderTargets(1, renderTargetViewArray, _depthStencilView.get());
}

void D3D::ResetViewport()
{
	// Set the viewport.
	_deviceContext->RSSetViewports(1, &_viewport);
}
