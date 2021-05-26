#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "Msimg32.lib")
#pragma comment(lib, "Comctl32.lib")

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' " \
	"version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <Uxtheme.h>

#include "UserInterface.h"
#include "Drawing.h"
#include "Button.h"
#include "resource.h"

#define CLASS_NAME L"MainWindow"
#define TOGGLE_HOTKEY_ID 150

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static SIZE GetAdjustedWindowSize(int width, int height)
{
	RECT rcWindow;
	SetRect(&rcWindow, 0, 0, width, height);
	AdjustWindowRectExForDpi(&rcWindow, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, TRUE, NULL, GetDpiForSystem());

	SIZE szWin;
	szWin.cx = rcWindow.right - rcWindow.left;
	szWin.cy = rcWindow.bottom - rcWindow.top;
	return szWin;
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	SetProcessDPIAware();
	BufferedPaintInit();

	WNDCLASS wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.hInstance     = hInstance;
	wc.lpfnWndProc   = WindowProcedure;
	wc.lpszClassName = CLASS_NAME;
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));

	if (!RegisterClass(&wc))
	{
		MessageBox(NULL, L"Failed to register window class", L"Error", MB_OK | MB_ICONERROR);
		return EXIT_FAILURE;
	}

	double dpiScale = GetDpiForSystem() / 96.0;
	SIZE szWin = GetAdjustedWindowSize(
		 (INT)(250 * dpiScale),
		 (INT)(45 * dpiScale)
	);

	HWND hWnd = CreateWindow(
		CLASS_NAME,
		L"AutoClicker",
		WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT,
		szWin.cx, 
		szWin.cy,
		NULL, NULL, hInstance, NULL
	);

	SetMenu(hWnd, LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU1)));

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	BufferedPaintUnInit();

	return (int)msg.wParam;
}

static void PaintBackground(HWND hWnd)
{
	PAINTSTRUCT ps;
	HDC hDC = BeginPaint(hWnd, &ps);

	RECT clientRect;
	GetClientRect(hWnd, &clientRect);
	PaintVerticalGradient(hDC, clientRect, RGB(237, 244, 252), RGB(219, 236, 255));
	EndPaint(hWnd, &ps);
}

static LONG GetTitlebarHeight(HWND hWnd)
{
	RECT rcWin, rcClient;
	GetWindowRect(hWnd, &rcWin);
	GetClientRect(hWnd, &rcClient);
	return (rcWin.bottom - rcWin.top) - rcClient.bottom;
}

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{	
	case WM_CREATE:
		RegisterHotKey(hWnd, TOGGLE_HOTKEY_ID, MOD_NOREPEAT, VK_F7);
		InitializeUserInterface(hWnd);
		return 0;

	case WM_PAINT:
		PaintBackground(hWnd);
		return 0;

	case WM_COMMAND:
		HandleCommandMessage(hWnd, wParam);
		return 0;

	case WM_HOTKEY:
		if (wParam == TOGGLE_HOTKEY_ID)
			ToggleClicker(hWnd);
		return 0;

	case WM_CLOSE:
		PostQuitMessage(EXIT_SUCCESS);
		return 0;
		
	case WM_DRAWITEM:
		return HandleDrawItemMessage(lParam);

	case WM_SIZE:
		ResizeControls(lParam);
		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}