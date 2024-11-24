#include "Application.h"


Application::Application(uint screenWidth, uint screenHeight, HWND hwnd)
	: _direct3D(D3D::InitParams{ hwnd, screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH, VSYNC_ENABLED, FULL_SCREEN })
{
	// Set the initial position of the camera.
	_camera.SetPosition(0.0f, 0.0f, -5.0f);

	const char* textureFilename = "../Engine/data/sidewalk.tga";

	// Create and initialize the model object.
	_model = std::make_unique<Model>(_direct3D.GetDevice(), _direct3D.GetDeviceContext(), "../Engine/data/sidewalk.tga");

	// Create and initialize the color shader object.
	_colorShader = std::make_unique<ColorShader>(_direct3D.GetDevice(), hwnd);

	_textureShader = std::make_unique<TextureShader>(_direct3D.GetDevice(), hwnd);
}

bool Application::Render()
{
	DirectX::XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

	// Clear the buffers to begin the scene.
	_direct3D.BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	_camera.Render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	_direct3D.GetWorldMatrix(worldMatrix);
	_camera.GetViewMatrix(viewMatrix);
	_direct3D.GetProjectionMatrix(projectionMatrix);

	// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
	_model->Render(_direct3D.GetDeviceContext());

	// Render the model using the color shader.
	result = _textureShader->Render(_direct3D.GetDeviceContext(), _model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, _model->GetTexture());
	if (!result)
		return false;

	// Present the rendered scene to the screen.
	_direct3D.EndScene();

	return true;
}

bool Application::Frame()
{
	return Render();
}

