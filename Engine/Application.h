#pragma once

#include "Common.h"
#include "D3D.h"

#include "Camera.h"
#include "Model.h"
#include "ColorShader.h"
#include "TextureShader.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.3f;

class Application
{
public:
	
	Application(uint screenWidth, uint screenHeight, HWND hwnd);

	bool Frame();

private:

	bool Render();

	D3D _direct3D;

	Camera _camera;
	std::unique_ptr<Model> _model;
	std::unique_ptr<ColorShader> _colorShader;
	std::unique_ptr<TextureShader> _textureShader;
};
