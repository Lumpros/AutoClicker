#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

typedef void (*DRAWFUNC)(LPDRAWITEMSTRUCT, LPWSTR, BOOLEAN, BOOLEAN);

typedef struct tagBUTTONINFO
{
	DRAWFUNC lpfnDrawFunc;
	LPCWSTR  lpszText;
	RECT rect;
} BUTTONINFO;

class Button
{
private:
	LPWSTR lpszText;
	DRAWFUNC lpfnDrawFunc;
	HWND hWnd;
	RECT rect;
	BOOLEAN hasStartedAnimation = FALSE;
	BOOLEAN isClicked = FALSE;

	void DrawButtonStates(LPDRAWITEMSTRUCT pDIS, HDC hdcFrom, HDC hdcTo);

public:
	void Draw(LPDRAWITEMSTRUCT pDIS);
	void StartAnimation(BOOLEAN hasStarted, BOOLEAN isClicked);

	BOOLEAN IsClicked(void) const;
	
	operator HWND() const { return hWnd; }

	Button(HWND hWndParent, BUTTONINFO buttonInfo, WORD wID);
	~Button(void);
};