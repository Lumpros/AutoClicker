#include "Clicker.h"
#include "resource.h"
#include "UserInterface.h"

#include <thread>

#define CB_CHOICE_LEFT 0

bool isThreadRunning = false;

void Clicker::Disable(void)
{
	isClicking = false;
}

bool Clicker::IsClicking(void) const
{
	return isClicking;
}

static void InitializeClickerSettings(INT* iDelayMilliseconds, INT* iCurSel)
{
	*iDelayMilliseconds = GetMillisecondsDelay();
	*iCurSel = GetCBCurSel();
}

static void InitializeClickInput(LPINPUT pInput, INT iCurSel)
{
	ZeroMemory(pInput, sizeof(INPUT) * 2);

	pInput[0].type       = INPUT_MOUSE;
	pInput[1].type       = INPUT_MOUSE;
	pInput[0].mi.dwFlags = (iCurSel == CB_CHOICE_LEFT) ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN;
	pInput[1].mi.dwFlags = (iCurSel == CB_CHOICE_LEFT) ? MOUSEEVENTF_LEFTUP   : MOUSEEVENTF_RIGHTUP;
}

void AutoClickerLoop(bool* isClicking)
{
	/* Default settings */
	INT iDelayMilliseconds = 100, iCurSel = 0;
	InitializeClickerSettings(&iDelayMilliseconds, &iCurSel);

	if (iDelayMilliseconds == 0)
	{
		MessageBox(NULL, L"Click delay cannot be 0!", L"Error", MB_OK | MB_ICONERROR);
	}

	else
	{
		INPUT input[2];
		InitializeClickInput(input, iCurSel);
		while (*isClicking)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(iDelayMilliseconds));
			
			/* Might have been changed in a different thread during sleep_for*/
			if (*isClicking)
			{
				SendInput(2, input, sizeof(INPUT));
			}
		}
	}

	isThreadRunning = false;
}

void Clicker::Toggle(HWND hWnd)
{
	isClicking = !isClicking;
	if (isClicking)
		LaunchThread(hWnd);
}

void Clicker::LaunchThread(HWND hWnd)
{
	if (!isThreadRunning)
	{
		isThreadRunning = true;
		std::thread thrClick(AutoClickerLoop, &isClicking);
		thrClick.detach();
	}
}