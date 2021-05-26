#include "Drawing.h"

void PaintVerticalGradient(HDC hDC, const RECT rect, COLORREF rgbTop, COLORREF rgbBottom)
{
    GRADIENT_RECT gradientRect = { 0, 1 };
    TRIVERTEX triVertext[2] = {
        rect.left - 1,
        rect.top - 1,
        (COLOR16)(GetRValue(rgbTop) << 8),
        (COLOR16)(GetGValue(rgbTop) << 8),
        (COLOR16)(GetBValue(rgbTop) << 8),
        0x0000,
        rect.right,
        rect.bottom,
        (COLOR16)(GetRValue(rgbBottom) << 8),
        (COLOR16)(GetGValue(rgbBottom) << 8),
        (COLOR16)(GetBValue(rgbBottom) << 8),
        0x0000
    };

    GradientFill(hDC, triVertext, 2, &gradientRect, 1, GRADIENT_FILL_RECT_V);
}

static void DrawTopHalf(HDC hDC, RECT rect, COLORREF top, COLORREF bottom)
{
    rect.bottom /= 2;
    PaintVerticalGradient(hDC, rect, top, bottom);
}

static void DrawBotHalf(HDC hDC, RECT rect, COLORREF top, COLORREF bottom)
{
    rect.top = rect.bottom / 2;
    PaintVerticalGradient(hDC, rect, top, bottom);
}

static COLORREF GetPenColor(BOOLEAN isFinal, BOOLEAN isClicked)
{
    COLORREF color = RGB(135, 151, 170);

    if (isFinal)
        color = RGB(255, 220, 7);

    if (isClicked)
        color = RGB(194, 155, 41);

    return color;
}

static void DrawButtonOutline(HDC hDC, RECT rect, BOOLEAN isFinal, BOOLEAN isClicked)
{
    HPEN hPen = CreatePen(PS_SOLID, 1, GetPenColor(isFinal, isClicked));

    SelectObject(hDC, GetStockObject(NULL_BRUSH));
    SelectObject(hDC, hPen);
    Rectangle(hDC, rect.left, rect.top, rect.right, rect.bottom);
    DeleteObject(hPen);

    if (!isClicked)
    {
        SelectObject(hDC, GetStockObject(WHITE_PEN));
        Rectangle(hDC, rect.left + 1, rect.top + 1, rect.right - 1, rect.bottom - 1);
    }
}

static void DrawButtonText(HDC hDC, RECT rect, LPWSTR lpszText)
{
    SIZE sz;
    int len = lstrlen(lpszText);

    HFONT hFont = (HFONT)GetCurrentObject(hDC, OBJ_FONT);

    GetTextExtentPoint32(hDC, lpszText, len, &sz);
    TextOut(
        hDC,
        rect.right - (rect.right - rect.left) / 2 - sz.cx / 2,
        rect.bottom - (rect.bottom - rect.top) / 2 - sz.cy / 2,
        lpszText,
        len
    );
}

void MakeColorsDarker(COLORREF* c1, COLORREF* c2, COLORREF* c3, COLORREF* c4)
{
    COLORREF crDark = RGB(20, 20, 20);

    *c1 -= crDark;
    *c2 -= crDark;
    *c3 -= crDark;
    *c4 -= crDark;
}

static void DrawDottedRectangleOnClick(HDC hDC, RECT rcButton)
{
    LOGBRUSH logBrush;
    logBrush.lbColor = RGB(0, 0, 0);
    logBrush.lbStyle = PS_SOLID;

    HPEN hPen = ExtCreatePen(PS_COSMETIC | PS_ALTERNATE, 1, &logBrush, 0, NULL);

    rcButton.left += 3;
    rcButton.right -= 3;
    rcButton.top += 3;
    rcButton.bottom -= 3;

    HPEN   hOldPen   = (HPEN)SelectObject(hDC, hPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(NULL_BRUSH));

    Rectangle(hDC, rcButton.left, rcButton.top, rcButton.right, rcButton.bottom);
    SelectObject(hDC, hOldPen);
    SelectObject(hDC, hOldBrush);
    DeleteObject(hPen);
}

inline void DecideButtonColor(BOOLEAN isFinal, COLORREF& topBegin, COLORREF& topEnd, COLORREF& botBegin, COLORREF& botEnd)
{
    if (!isFinal)
    {
        topBegin = RGB(255, 255, 255);
        topEnd = RGB(235, 242, 248);
        botBegin = RGB(228, 236, 244);
        botEnd = RGB(215, 226, 240);
    }

    else
    {
        topBegin = RGB(255, 251, 246);
        topEnd = RGB(255, 227, 197);
        botBegin = RGB(254, 217, 154);
        botEnd = RGB(255, 244, 151);
    }
}

void DrawCustomButton(LPDRAWITEMSTRUCT lpDIS, LPWSTR lpszText, BOOLEAN isFinal, BOOLEAN isClicked)
{
    HDC  hDC = lpDIS->hDC;
    RECT rect = lpDIS->rcItem;
    COLORREF topBegin, topEnd, botBegin, botEnd;

    DecideButtonColor(isFinal, topBegin, topEnd, botBegin, botEnd);

    if (isClicked)
        MakeColorsDarker(&topBegin, &topEnd, &botBegin, &botEnd);

    SetBkMode(hDC, TRANSPARENT);
    DrawTopHalf(hDC, rect, topBegin, topEnd);
    DrawBotHalf(hDC, rect, botBegin, botEnd);
    DrawButtonOutline(hDC, rect, isFinal, isClicked);
    DrawButtonText(hDC, rect, lpszText);

    if (isClicked)
        DrawDottedRectangleOnClick(hDC, rect);
}