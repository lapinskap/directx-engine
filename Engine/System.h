#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "Common.h"
#include "Input.h"
#include "Application.h"

class System
{
public:
	
	System();
	~System();

	void Run();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

	bool Frame();
	void InitializeWindows(uint& screenWidth, uint& screenHeight);
	void ShutdownWindows();

private:

	LPCWSTR _applicationName = nullptr;
	HINSTANCE _hinstance = nullptr;
	HWND _hwnd = nullptr;

	Input _input; // This object will be used to handle reading the keyboard input from the user.
	std::unique_ptr<Application> _application;
};
