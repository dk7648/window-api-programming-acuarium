#include <Windows.h>
#include "MyUtil.h"

POINT CenterPoint(RECT& r) {
	POINT p;
	p.x = (r.left + r.right) / 2;
	p.y = (r.top + r.bottom) / 2;
	return p;
}
int CheckStrikeX(RECT& r, RECT& bound) {
	if (r.left <= bound.left || r.right >= bound.right)
		return 1;
	return 0;
}
int CheckStrikeY(RECT& r, RECT& bound) {
	if (r.top <= bound.top || r.bottom >= bound.bottom)
		return 1;
	return 0;
}

void DrawObject(HDC hdc, RECT& r, COLORREF color, int type) {
	DrawObject(hdc, r, color, color, type);
}
void DrawObject(HDC hdc, RECT& r, COLORREF penC, COLORREF brushC, int type) {
	HPEN hPen, hOldPen;
	HBRUSH hBrush, hOldBrush;
	hPen = CreatePen(PS_SOLID, 1, penC);
	hOldPen = (HPEN)SelectObject(hdc, hPen);
	hBrush = CreateSolidBrush(brushC);
	hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

	switch (type) {
	case 0:
		Rectangle(hdc, r.left, r.top, r.right, r.bottom);
		break;
	case 1:
		Ellipse(hdc, r.left, r.top, r.right, r.bottom);
		break;
	}

	SelectObject(hdc, hOldPen);
	SelectObject(hdc, hOldBrush);
	DeleteObject(hPen);
	DeleteObject(hBrush);
}
void flipFlag(int& flag) {
	flag = flag ? 0 : 1;
}