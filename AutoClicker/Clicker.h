#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

class Clicker
{
private:
	bool isClicking = false;

public:
	void Toggle(HWND hWnd);
	void Disable(void);
	void LaunchThread(HWND hWnd);
	bool IsClicking(void) const;
};

