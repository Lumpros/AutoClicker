#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

void PaintVerticalGradient(HDC hDC, const RECT rect, COLORREF rgbTop, COLORREF rgbBottom);

void DrawCustomButton(LPDRAWITEMSTRUCT lpDIS, LPWSTR lpszText, BOOLEAN isFinal, BOOLEAN isClicked);