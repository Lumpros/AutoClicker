#pragma once

#include <Windows.h>
#include <vector>

void InitializeUserInterface(HWND hWnd);
void DeleteUserInterface(void);

LRESULT HandleDrawItemMessage(LPARAM lParam);
void HandleCommandMessage(HWND hWnd, WPARAM wParam);
void ResizeControls(LPARAM lParam);

void ToggleClicker(HWND hWnd);
INT GetMillisecondsDelay(void);
INT GetCBCurSel(void);