#include "UserInterface.h"
#include "Button.h"
#include "Drawing.h"
#include "Clicker.h"
#include "resource.h"

#include <vector>
#include <thread>

#define IDC_TOGGLE       200
#define IDC_QUIT         201
#define IDC_MOUSE_BUTTON 202

INT     iDlgChoice = 0;
INT     iDelay = 100;
LPWSTR  lpszDelay = NULL;
Button* pToggleButton, * pQuitButton;
Clicker autoClicker;
BOOL    isDialogOpen = false;

/* This should be an atomic but I don't care enough right now*/
volatile BOOL   isCountingDown = false;

INT GetMillisecondsDelay(void)
{
	return iDelay;
}

INT GetCBCurSel(void)
{
	return iDlgChoice;
}

void DeleteUserInterface(void)
{
	/* Doesnt go with the name but idk where to put this */
	autoClicker.Disable();

	delete pToggleButton;
	delete pQuitButton;
	delete[] lpszDelay;
}

LRESULT HandleDrawItemMessage(LPARAM lParam)
{
	LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT)lParam;
	pToggleButton->Draw(lpDIS);
	pQuitButton->Draw(lpDIS);
	return 0;
}

static void CreateToggleButton(HWND hParentWnd, DOUBLE dpiScale, RECT rcClient)
{
	BUTTONINFO bInfo;
	bInfo.lpfnDrawFunc = DrawCustomButton;
	bInfo.lpszText     = L"Toggle (F7)";

	SetRect(&bInfo.rect,
		(INT)(10 * dpiScale),
		(INT)(rcClient.bottom - 25 * dpiScale - 10 * dpiScale),
		(INT)(80 * dpiScale),
		(INT)(25 * dpiScale)
	);

	pToggleButton = new Button(hParentWnd, bInfo, IDC_TOGGLE);
}

static void CreateQuitButton(HWND hParentWnd, DOUBLE dpiScale, RECT rcClient)
{
	BUTTONINFO bInfo;
	bInfo.lpfnDrawFunc = DrawCustomButton;
	bInfo.lpszText     = L"Quit";

	SetRect(&bInfo.rect,
		(INT)(100 * dpiScale),
		(INT)(rcClient.bottom - 35 * dpiScale),
		(INT)(80 * dpiScale),
		(INT)(25 * dpiScale)
	);

	pQuitButton = new Button(hParentWnd, bInfo, IDC_QUIT);
}

static void SetFontsForControls(void)
{
	DOUBLE dpiScale = GetDpiForSystem() / 96.0;
	HFONT hFont = CreateFont(
		(INT)(16 * dpiScale), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, L"Segoe UI"
	);

	SendMessage(*pToggleButton, WM_SETFONT, (WPARAM)hFont, NULL);
	SendMessage(*pQuitButton, WM_SETFONT, (WPARAM)hFont, NULL);
}

void InitializeUserInterface(HWND hWnd)
{
	HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	DOUBLE dpiScale = GetDpiForSystem() / 96.0;
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);

	CreateToggleButton(hWnd, dpiScale, rcClient);
	CreateQuitButton(hWnd, dpiScale, rcClient);
	SetFontsForControls();
}

void ToggleClicker(HWND hWnd)
{
	if (isCountingDown)
		isCountingDown = false;
	else
		autoClicker.Toggle(hWnd);
}

static void HandleDelayString(void)
{
	if (lpszDelay == NULL)
	{
		lpszDelay = new WCHAR[16];
		lstrcpy(lpszDelay, L"100");
	}

	else if (lstrlen(lpszDelay) == 0)
	{
		lstrcpy(lpszDelay, L"100");
	}
}

static void InitializeDialogControlData(HWND hWnd)
{
	HWND hComboWnd = GetDlgItem(hWnd, IDC_COMBO1);
	SendMessage(hComboWnd, CB_ADDSTRING, 0, (LPARAM)L"Left");
	SendMessage(hComboWnd, CB_ADDSTRING, 0, (LPARAM)L"Right");
	SendMessage(hComboWnd, CB_SETCURSEL, iDlgChoice, NULL);

	HWND hEditWnd = GetDlgItem(hWnd, IDC_DELAY_EDIT);
	SendMessage(hEditWnd, EM_SETLIMITTEXT, 8, 0);

	HandleDelayString();
	SendMessage(hEditWnd, WM_SETTEXT, 0, (LPARAM)lpszDelay);
}

static void SaveSettings(HWND hWnd)
{
	WCHAR wchText[16];
	GetWindowText(GetDlgItem(hWnd, IDC_DELAY_EDIT), wchText, 16);
	lstrcpy(lpszDelay, wchText);

	iDlgChoice = SendMessage(GetDlgItem(hWnd, IDC_COMBO1), CB_GETCURSEL, 0, 0);
	iDelay     = _wtoi(wchText);
}

INT_PTR CALLBACK OptionsDlgProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		isDialogOpen = true;
		autoClicker.Disable();
		InitializeDialogControlData(hWnd);
		break;

	case WM_CLOSE:
		EndDialog(hWnd, EXIT_SUCCESS);
		break;

	case WM_COMMAND:
		switch (wParam)
		{
		case IDOK:
			SaveSettings(hWnd);
			isDialogOpen = false;
			EndDialog(hWnd, IDOK);
			break;
			
		case IDCANCEL:
			EndDialog(hWnd, IDCANCEL);
			isDialogOpen = false;
			break;
		}
		break;
	}

	return 0;
}

/* Returns true if the countdown was completed, false if it was disrupted */
static bool DoCountdown(HWND hWnd)
{
	isCountingDown = true;
	WCHAR lpszTitle[32];

	for (int i = 5; i > 0; --i)
	{
		wsprintf(lpszTitle, L"Clicking in %d...", i);
		SetWindowText(hWnd, lpszTitle);

		if (!isCountingDown)
		{
			SetWindowText(hWnd, L"AutoClicker");
			return false;
		}

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	isCountingDown = false;
	return true;
}

void CountdownAndLaunchThread(HWND hWnd)
{
	if (!isCountingDown)
	{
		if (DoCountdown(hWnd))
		{
			autoClicker.Toggle(hWnd);
		}
	}

	SetWindowText(hWnd, L"AutoClicker");
}

static void HandleToggleCommand(HWND hWnd)
{
	if (isCountingDown) 
		isCountingDown = false;

	else if (!autoClicker.IsClicking()) {
		std::thread thr(CountdownAndLaunchThread, hWnd);
		thr.detach();
	}

	else
		autoClicker.Disable();
}

void HandleCommandMessage(HWND hWnd, WPARAM wParam)
{
	WORD wID = LOWORD(wParam);

	switch (wID)
	{
	case ID_EXIT:
	case IDC_QUIT:
		PostQuitMessage(0);
		break;

	case ID_OPTIONS:
		autoClicker.Disable();
		DialogBox(NULL, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, OptionsDlgProcedure);
		break;

	case IDC_TOGGLE:
		HandleToggleCommand(hWnd);
		break;
	}
}

static void AdjustToggleButtonRect(UINT uiWidth, UINT uiHeight, DOUBLE dpiScale)
{
	SetWindowPos(
		*pToggleButton,
		NULL,
		(INT)(dpiScale * 10),
		(INT)(dpiScale * 10),
		(INT)(dpiScale * -15 + uiWidth / 2),
		(INT)(dpiScale * -20 + uiHeight),
		SWP_NOZORDER
	);
}

static void AdjustQuitButtonRect(UINT uiWidth, UINT uiHeight, DOUBLE dpiScale)
{
	SetWindowPos(
		*pQuitButton,
		NULL,
		(INT)(dpiScale * 5 + uiWidth / 2),
		(INT)(dpiScale * 10),
		(INT)(dpiScale * -15 + uiWidth / 2),
		(INT)(dpiScale * -20 + uiHeight),
		SWP_NOZORDER
	);
}

void ResizeControls(LPARAM lParam)
{
	UINT   uiWidth  = LOWORD(lParam);
	UINT   uiHeight = HIWORD(lParam);
	DOUBLE dpiScale = GetDpiForSystem() / 96.0;

	AdjustToggleButtonRect(uiWidth, uiHeight, dpiScale);
	AdjustQuitButtonRect(uiWidth, uiHeight, dpiScale);
}

void HandleHotkeyMessage(HWND hWnd, WPARAM wParam)
{
	if (wParam == TOGGLE_HOTKEY_ID && !isDialogOpen)
	{
		ToggleClicker(hWnd);
	}
}