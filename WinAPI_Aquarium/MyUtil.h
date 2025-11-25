#pragma once
#ifndef _MYUTIL
#define _MYUTIL
#include <windows.h>

POINT CenterPoint(RECT& r);
int CheckStrikeX(RECT& r, RECT& bound);
int CheckStrikeY(RECT& r, RECT& bound);
void DrawObject(HDC hdc, RECT& r, COLORREF color, int type);
void DrawObject(HDC hdc, RECT& r, COLORREF penC, COLORREF brushC, int type);
void flipFlag(int& flag);
#endif