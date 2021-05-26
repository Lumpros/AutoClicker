#include "Button.h"
#include "Button.h"

#include <stdlib.h>
#include <Uxtheme.h>

#define TRANSITION_DURATION 200

static BP_ANIMATIONPARAMS CreateAnimationParamsStruct(void)
{
	BP_ANIMATIONPARAMS animParams;
	ZeroMemory(&animParams, sizeof(animParams));

	animParams.cbSize     = sizeof(animParams);
	animParams.style      = BPAS_LINEAR;
	animParams.dwDuration = TRANSITION_DURATION;

	return animParams;
}

static RECT CreateRectangleForBufferedAnimation(HWND hButtonWnd)
{
	RECT rcControl;
	GetWindowRect(hButtonWnd, &rcControl);
	OffsetRect(&rcControl, -rcControl.left, -rcControl.top);

	return rcControl;
}

static void TrackMouse(HWND hWnd)
{
	TRACKMOUSEEVENT tme;
	tme.cbSize      = sizeof(tme);
	tme.dwFlags     = TME_LEAVE;
	tme.hwndTrack   = hWnd;
	tme.dwHoverTime = 1;

	_TrackMouseEvent(&tme);
}

LRESULT CALLBACK CustomButtonSubclass(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	Button* ptrButton = (Button*)dwRefData;
	static BOOLEAN isTracking = FALSE;

	if (ptrButton != NULL)
	{
		switch (message)
		{
		case WM_MOUSEMOVE:
			if (!isTracking)
			{
				/* Start tracking mouse in order to receive a WM_MOUSELEAVE message */
				TrackMouse(hWnd);
				isTracking = TRUE;
			}
			/* Fade to second state */
			ptrButton->StartAnimation(TRUE, ptrButton->IsClicked());
			break;

		case WM_MOUSELEAVE:
			/* Fade out button */
			isTracking = FALSE;
			ptrButton->StartAnimation(FALSE, ptrButton->IsClicked());
			break;

		case WM_LBUTTONDOWN:
			/* Make button darker & draw dotted rectangle */
			ptrButton->StartAnimation(TRUE, TRUE);
			break;

		case WM_LBUTTONUP:
			/* Make colors normal & remove dotted rectangle*/
			ptrButton->StartAnimation(TRUE, FALSE);
			break;
		}
	}

	return DefSubclassProc(hWnd, message, wParam, lParam);
}

Button::Button(HWND hWndParent, BUTTONINFO buttonInfo, WORD wID)
{
	this->lpfnDrawFunc = buttonInfo.lpfnDrawFunc;
	this->lpszText     = NULL;
	this->rect         = buttonInfo.rect;

	if (buttonInfo.lpszText != NULL)
		this->lpszText = _wcsdup(buttonInfo.lpszText);

	hWnd = CreateWindowEx(
		NULL,
		L"Button",
		lpszText,
		WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
		rect.left,
		rect.top,
		rect.right,
		rect.bottom,
		hWndParent,
		(HMENU)wID,
		(HINSTANCE)GetWindowLongPtr(hWndParent, GWLP_HINSTANCE),
		NULL
	);

	SetWindowSubclass(hWnd, CustomButtonSubclass, 0, (DWORD_PTR)this);
}

void Button::Draw(LPDRAWITEMSTRUCT pDIS)
{
	if (pDIS->hwndItem == hWnd)
	{
		HDC                hdcFrom, hdcTo;
		RECT               rcControl    = CreateRectangleForBufferedAnimation(hWnd);
		BP_ANIMATIONPARAMS animParams   = CreateAnimationParamsStruct();

		HANIMATIONBUFFER   hbpAnimation = BeginBufferedAnimation(
			hWnd, pDIS->hDC, 
			&rcControl, 
			BPBF_COMPATIBLEBITMAP,
			NULL, 
			&animParams, 
			&hdcFrom, 
			&hdcTo
		);

		if (hbpAnimation)
		{
			DrawButtonStates(pDIS, hdcFrom, hdcTo);
			EndBufferedAnimation(hbpAnimation, TRUE);
		}

		else
		{
			lpfnDrawFunc(pDIS, lpszText, hasStartedAnimation, isClicked);
		}
	}
}

void Button::DrawButtonStates(LPDRAWITEMSTRUCT pDIS, HDC hdcFrom, HDC hdcTo)
{
	HFONT hFont = (HFONT)GetCurrentObject(pDIS->hDC, OBJ_FONT);

	if (hdcFrom)
	{
		SelectObject(hdcFrom, hFont);
		pDIS->hDC = hdcFrom;
		lpfnDrawFunc(pDIS, lpszText, hasStartedAnimation, isClicked);
	}

	if (hdcTo)
	{
		SelectObject(hdcTo, hFont);
		pDIS->hDC = hdcTo;
		lpfnDrawFunc(pDIS, lpszText, hasStartedAnimation, isClicked);
	}
}

Button::~Button(void)
{
	if (lpszText != NULL)
	{
		free(lpszText);
	}
}

void Button::StartAnimation(BOOLEAN hasStarted, BOOLEAN isClicked)
{
	if (hasStartedAnimation != hasStarted)
	{
		hasStartedAnimation = hasStarted;
		InvalidateRect(hWnd, NULL, FALSE);
	}

	else if (this->isClicked != isClicked)
	{
		this->isClicked = isClicked;
		InvalidateRect(hWnd, NULL, FALSE);
	}
}

BOOLEAN Button::IsClicked(void) const
{
	return isClicked;
}