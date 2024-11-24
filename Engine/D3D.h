#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#pragma warning(push, 0)
#include <d3d11.h>
#include <directxmath.h>
#pragma warning(pop)

#include "Common.h"
#include "ReleasePtr.h"

class D3DError : public std::runtime_error
{
public:
    D3DError(const std::string& message) : std::runtime_error(message) {}
	D3DError(const char* message) : std::runtime_error(message) {}
};

class D3D
{
public:

	struct InitParams
	{
        HWND hwnd;
        uint screenWidth;
		uint screenHeight;
        float screenNear;
        float screenFar;
        bool vsync;
		bool fullscreen;
	};

	D3D(const InitParams& initParams);
    ~D3D();

    void BeginScene(float red, float green, float blue, float alpha);
    void EndScene();

    ID3D11Device* GetDevice();
    ID3D11DeviceContext* GetDeviceContext();

    void GetProjectionMatrix(DirectX::XMMATRIX&);
    void GetWorldMatrix(DirectX::XMMATRIX&);
    void GetOrthoMatrix(DirectX::XMMATRIX&);

    const std::string& GetVideoCardInfo() const;

    void SetBackBufferRenderTarget();
    void ResetViewport();

private:

	void InitVideoCardInfo(const InitParams& initParams, uint& numerator, uint& denominator);
	void InitDeviceAndSwapChain(const InitParams& initParams, uint numerator, uint denominator);
    void InitRenderTargetView();
    void InitDepthStencilBuffer(const InitParams& initParams);
    void InitDepthStencilState();
	void InitDepthStencilView();
    void InitRasterState();
    void InitViewport(const InitParams& initParams);

    bool _vsyncEnabled = false;
    int _videoCardMemory = 0;
    std::string _videoCardDescription;
    ReleasePtr<IDXGISwapChain> _swapChain;
    ReleasePtr<ID3D11Device> _device;
    ReleasePtr<ID3D11DeviceContext> _deviceContext;
    ReleasePtr<ID3D11RenderTargetView> _renderTargetView;
    ReleasePtr<ID3D11Texture2D> _depthStencilBuffer;
    ReleasePtr<ID3D11DepthStencilState> _depthStencilState;
    ReleasePtr<ID3D11DepthStencilView> _depthStencilView;
    ReleasePtr<ID3D11RasterizerState> _rasterState;
    DirectX::XMMATRIX _projectionMatrix;
    DirectX::XMMATRIX _worldMatrix;
    DirectX::XMMATRIX _orthoMatrix;
    D3D11_VIEWPORT _viewport;
};

