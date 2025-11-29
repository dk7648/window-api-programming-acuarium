#include <windows.h>
#include <tchar.h>
#include "resource.h"
#include "MyUtil.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HINSTANCE g_hInst;
LPCTSTR lpszClass = _T("FishGame");

#define EMPTY_MAX 10

typedef struct {
    RECT r;
    int active;
    int size;
    int speed;
} EmptyObj;

static int playerX, playerY;

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmd) {
    g_hInst = hInst;

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = lpszClass;
    RegisterClass(&wc);

    HWND hWnd = CreateWindow(lpszClass, lpszClass,
        WS_OVERLAPPEDWINDOW, 300, 200, 800, 600,
        NULL, NULL, hInst, NULL);

    ShowWindow(hWnd, nCmd);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {


    static EmptyObj empty[EMPTY_MAX];
    static HBITMAP hMap, hPlayer, hEmpty, hBonus;
    static RECT mapR, bonusR, playerR;

    static int bonusW, bonusH;
    static int playerW, playerH;
    static int emptyW, emptyH;

    static int bonusAlphaX = 2, bonusAlphaY = 1;
    static int playerAlphaX = 0, playerAlphaY = 0;
    static int grow = 0;

    // 점수/시간
    static int playTime = 0;

    BITMAP bm;
    HDC hdc, mem;
    PAINTSTRUCT ps;

    switch (msg) {

    case WM_CREATE: {
        GetClientRect(hWnd, &mapR);
        POINT p = CenterPoint(mapR);
        playerX = p.x;
        playerY = mapR.bottom - 120;
        grow = 0;
        bonusAlphaX = 2, bonusAlphaY = 1;
        playerAlphaX = 0, playerAlphaY = 0;

        hMap = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BACKGROUND));

        SetRect(&playerR, p.x - 20, p.y - 20, p.x + 20, p.y + 20);
        hPlayer = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_PLAYER));
        GetObject(hPlayer, sizeof(bm), &bm);
        playerW = bm.bmWidth;
        playerH = bm.bmHeight;

        hEmpty = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_EMPTY1));
        GetObject(hEmpty, sizeof(bm), &bm);
        emptyW = bm.bmWidth;
        emptyH = bm.bmHeight;

        SetRect(&bonusR, p.x - 10, p.y - 10, p.x + 10, p.y + 10);
        hBonus = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BONUS1));
        GetObject(hBonus, sizeof(bm), &bm);
        bonusW = bm.bmWidth;
        bonusH = bm.bmHeight;

        for (int i = 0; i < EMPTY_MAX; i++) empty[i].active = 0;

        SetTimer(hWnd, 1, 40, NULL);
        SetTimer(hWnd, 2, 50, NULL);

        // 타이머
        playTime = 0;
        SetTimer(hWnd, 3, 1000, NULL);
        
        return 0;
    }

    case WM_TIMER:
        switch (wParam) {

        // 보너스 및 플레이어 이동
        case 1: {
            if (CheckStrikeX(bonusR, mapR)) bonusAlphaX *= -1;
            if (CheckStrikeY(bonusR, mapR)) bonusAlphaY *= -1;
            OffsetRect(&bonusR, bonusAlphaX, bonusAlphaY);

            if (CheckStrikeX(playerR, mapR)) playerAlphaX *= -1;
            if (CheckStrikeY(playerR, mapR)) playerAlphaY *= -1;
            OffsetRect(&playerR, playerAlphaX, playerAlphaY);

            int spawnCount = 4 + (rand() % 5);

            for (int k = 0; k < spawnCount; k++) {
                int idx = -1;
                for (int i = 0; i < EMPTY_MAX; i++) {
                    if (!empty[i].active) { idx = i; break; }
                }
                if (idx == -1) break;

                EmptyObj& E = empty[idx];

                // 플레이어 크기 기반 랜덤
                int sc = 100 + grow * 10;
                int pw = playerW * sc / 100;
                int ph = playerH * sc / 100;
                int pSize = (pw + ph) / 2;

                double ratio = 0.7 + (rand() % 71) / 100.0;
                E.size = (int)(pSize * ratio);

                if (E.size > 150)
                    E.size = 150;

                int speed = 3 + rand() % 10;
                int side = rand() % 2;

                if (side == 0) { // 생성 방향에 따른 처리
                    E.r.left = -E.size;
                    E.r.right = 0;
                    E.r.top = rand() % (mapR.bottom - E.size);
                    E.r.bottom = E.r.top + E.size;
                    E.speed = speed;
                }
                else {
                    E.r.right = mapR.right + E.size;
                    E.r.left = E.r.right - E.size;
                    E.r.top = rand() % (mapR.bottom - E.size);
                    E.r.bottom = E.r.top + E.size;
                    E.speed = -speed;
                }

                E.active = 1;
            }
            break;


            InvalidateRect(hWnd, NULL, FALSE);
            break;
        }
        case 2: {
            for (int i = 0; i < EMPTY_MAX; i++) {
                if (!empty[i].active) continue;

                EmptyObj& E = empty[i];
                OffsetRect(&E.r, E.speed, 0);
                // 맵 끝에 도달 시 삭제
                if (E.r.right < 0 || E.r.left > mapR.right) {
                    E.active = 0;
                    continue;
                }

                // 충돌 체크
                RECT inter;

                if (IntersectRect(&inter, &E.r, &playerR)) {

                    int pw = playerR.right - playerR.left;
                    int ph = playerR.bottom - playerR.top;

                    int playerArea = pw * ph;
                    int enemyArea = E.size * E.size;

                    
                    if (enemyArea < playerArea) {
                        grow++;
                        E.active = 0;
                    }
                    else {
                        KillTimer(hWnd, 1);
                        KillTimer(hWnd, 2);
                        KillTimer(hWnd, 3);
                        KillTimer(hWnd, 4);
                        KillTimer(hWnd, 10);

                        // 패배 메시지
                        TCHAR msg[256];
                        wsprintf(msg,
                            _T("더 큰 물고기에게 잡아먹혔습니다.\n\n")
                            _T("버틴 시간: %d초\n")
                            _T("최종 점수: %d점\n\n")
                            _T("다시 시작하시겠습니까?"),
                            (int)playTime, grow
                        );

                        // 패배
                        int ret = MessageBox(hWnd, msg, _T("GAME OVER"), MB_YESNO);

                        if (ret == IDYES) {
                            grow = 0;
                            playerAlphaX = playerAlphaY = 0;

                            playerX = mapR.right / 2;
                            playerY = mapR.bottom - 120;

                            POINT p = CenterPoint(mapR);
                            SetRect(&bonusR, p.x - 50, p.y - 50, p.x + 50, p.y + 50);

                            for (int j = 0; j < EMPTY_MAX; j++) empty[j].active = 0;

                            playTime = 0;

                            SetTimer(hWnd, 1, 30, NULL);
                            SetTimer(hWnd, 2, 200, NULL);
                            SetTimer(hWnd, 3, 2000, NULL);
                            SetTimer(hWnd, 4, 40, NULL);
                            SetTimer(hWnd, 10, 100, NULL);
                        }
                        else PostQuitMessage(0);

                        break;
                    }
                    if (grow >= 100) {
                        KillTimer(hWnd, 1);
                        KillTimer(hWnd, 2);
                        KillTimer(hWnd, 3);
                        KillTimer(hWnd, 4);
                        KillTimer(hWnd, 10);

                        // 승리 메시지
                        TCHAR msg[256];
                        wsprintf(msg,
                            _T("바다의 제왕이 되었습니다.\n\n")
                            _T("클리어 시간: %d초\n")
                            _T("다시 시작하시겠습니까?"),
                            (int)playTime
                        );

                        // 승리
                        int ret = MessageBox(hWnd, msg, _T("GAME CLEAR !!"), MB_YESNO);

                        if (ret == IDYES) {
                            grow = 0;
                            playerAlphaX = playerAlphaY = 0;

                            playerX = mapR.right / 2;
                            playerY = mapR.bottom - 120;

                            POINT p = CenterPoint(mapR);
                            SetRect(&bonusR, p.x - 50, p.y - 50, p.x + 50, p.y + 50);

                            for (int j = 0; j < EMPTY_MAX; j++) empty[j].active = 0;

                            playTime = 0;

                            SetTimer(hWnd, 1, 30, NULL);
                            SetTimer(hWnd, 2, 200, NULL);
                            SetTimer(hWnd, 3, 2000, NULL);
                            SetTimer(hWnd, 4, 40, NULL);
                            SetTimer(hWnd, 10, 100, NULL);
                        }
                        else PostQuitMessage(0);

                        break;
                    }
                }
                // 보너스 물고기 충돌
                else if (IntersectRect(&inter, &bonusR, &playerR)) {
                    grow++;
                    bonusR.left = rand() % (mapR.right - bonusW);
                    bonusR.top = rand() % (mapR.bottom - bonusH);
                    bonusR.right = bonusR.left + bonusW;
                    bonusR.bottom = bonusR.top + bonusH;
                }
            }

            InvalidateRect(hWnd, NULL, FALSE);
            break;
        }

        case 3: {
            playTime++;
            break;
        }
        }
        return 0;

    case WM_KEYDOWN:
        switch (wParam) {
        case VK_LEFT:  playerAlphaX = -4; playerAlphaY = 0; break;
        case VK_RIGHT: playerAlphaX = 4; playerAlphaY = 0;  break;
        case VK_UP:    playerAlphaX = 0; playerAlphaY = -4; break;
        case VK_DOWN:  playerAlphaX = 0; playerAlphaY = 4;  break;
        }
        return 0;

    case WM_PAINT: {
        hdc = BeginPaint(hWnd, &ps);
        mem = CreateCompatibleDC(hdc);

        // 배경
        SelectObject(mem, hMap);
        GetObject(hMap, sizeof(bm), &bm);
        StretchBlt(hdc, 0, 0, mapR.right, mapR.bottom,
            mem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

        // 먹이
        SelectObject(mem, hBonus);
        StretchBlt(hdc,
            (bonusAlphaX > 0 ? bonusR.left : bonusR.right),
            bonusR.top,
            (bonusAlphaX > 0 ? bonusW : -bonusW),
            bonusH,
            mem, 0, 0, bonusW, bonusH, SRCCOPY);

        // ------ player ------

        SelectObject(mem, hPlayer);

        //사이즈 계산
        int base = 40;
        int size = base + grow * 5;

        playerR.left = (playerR.left + playerR.right) / 2 - size / 2;
        playerR.top = (playerR.top + playerR.bottom) / 2 - size / 2;
        playerR.right = playerR.left + size;
        playerR.bottom = playerR.top + size;

        //방향 계산
        int drawX = playerR.left;
        int drawW = playerR.right - playerR.left;

        if (playerAlphaX < 0) {
            drawX = playerR.right;
            drawW = -drawW;
        }

        StretchBlt(hdc,
            drawX, playerR.top,
            drawW, playerR.bottom - playerR.top,
            mem, 0, 0, playerW, playerH,
            SRCCOPY
        );

        // ---------- Empty ----------
        SelectObject(mem, hEmpty);
        for (int i = 0; i < EMPTY_MAX; i++) {
            if (!empty[i].active) continue;

            int ex = empty[i].r.left;
            int ey = empty[i].r.top;
            int es = empty[i].size;

            int drawX = ex;
            int drawW = es;

            // 이동 방향에 따른 뒤집기
            if (empty[i].speed < 0) {
                drawX = empty[i].r.right;
                drawW = -es;
            }

            empty[i].r.right = ex + es;
            empty[i].r.bottom = ey + es;

            StretchBlt(hdc,
                drawX, ey,
                drawW, es,
                mem,
                0, 0, emptyW, emptyH,
                SRCCOPY
            );
        }

        // ------- UI -------
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 255, 255));

        TCHAR buf[64];
        wsprintf(buf, _T("Score: %d"), grow);
        TextOut(hdc, 10, 10, buf, lstrlen(buf));

        TCHAR buf2[64];
        wsprintf(buf2, _T("Time: %ds"), (int)playTime);

        TextOut(hdc, mapR.right - 150, 10, buf2, lstrlen(buf2));

        DeleteDC(mem);
        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        DeleteObject(hBonus);
        DeleteObject(hMap);
        DeleteObject(hPlayer);
        DeleteObject(hEmpty);

        KillTimer(hWnd, 1);
        KillTimer(hWnd, 2);
        KillTimer(hWnd, 3);
        KillTimer(hWnd, 4);
        KillTimer(hWnd, 10);

        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}
