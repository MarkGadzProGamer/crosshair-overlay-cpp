#include <windows.h>
#include <stdio.h>

#define ID_HOT_QUIT 1
#define ID_HOT_TOGGLE 2
#define ID_HOT_EDIT 3

bool visible = true, editMode = false;
HWND hwnd = NULL, hwndMenu = NULL;
int posX, posY, dotSize = 4, thickness = 2, length = 12, gap = 6, mode = 2, colorIdx = 0; 
COLORREF colors[] = { RGB(0, 255, 255), RGB(255, 0, 0), RGB(0, 255, 0), RGB(255, 255, 0), RGB(255, 255, 255) };
const char* colorNames[] = { "Cyan", "Red", "Green", "Yellow", "White" };
const int winSize = 400; 

void UpdatePos() {
    SetWindowPos(hwnd, HWND_TOPMOST, posX - winSize/2, posY - winSize/2, winSize, winSize, SWP_NOACTIVATE);
    RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
    if (hwndMenu) InvalidateRect(hwndMenu, NULL, TRUE);
}

LRESULT CALLBACK WindowProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_PAINT) {
        PAINTSTRUCT ps; HDC hdc = BeginPaint(h, &ps);
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBM = CreateCompatibleBitmap(hdc, winSize, winSize);
        SelectObject(memDC, memBM);
        
        HBRUSH hBack = CreateSolidBrush(RGB(0, 0, 0));
        RECT r = {0, 0, winSize, winSize};
        FillRect(memDC, &r, hBack); DeleteObject(hBack);

        HBRUSH hBrush = CreateSolidBrush(colors[colorIdx]);
        int c = winSize / 2;
        int t2 = thickness / 2;

        if (mode == 1 || mode == 3) {
            RECT rDot = { c - dotSize/2, c - dotSize/2, c - dotSize/2 + dotSize, c - dotSize/2 + dotSize };
            FillRect(memDC, &rDot, hBrush);
        }
        if (mode == 2 || mode == 3) {
            // Идеально симметричные прямоугольники вместо линий
            RECT vTop = { c - t2, c - gap - length, c - t2 + thickness, c - gap };
            RECT vBot = { c - t2, c + gap, c - t2 + thickness, c + gap + length };
            RECT hLef = { c - gap - length, c - t2, c - gap, c - t2 + thickness };
            RECT hRig = { c + gap, c - t2, c + gap + length, c - t2 + thickness };
            
            FillRect(memDC, &vTop, hBrush); FillRect(memDC, &vBot, hBrush);
            FillRect(memDC, &hLef, hBrush); FillRect(memDC, &hRig, hBrush);
        }

        BitBlt(hdc, 0, 0, winSize, winSize, memDC, 0, 0, SRCCOPY);
        DeleteObject(memBM); DeleteDC(memDC); DeleteObject(hBrush);
        EndPaint(h, &ps); return 0;
    }
    return DefWindowProc(h, m, w, l);
}

LRESULT CALLBACK MenuProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (m == WM_PAINT) {
        PAINTSTRUCT ps; HDC hdc = BeginPaint(h, &ps);
        SetTextColor(hdc, colors[colorIdx]); SetBkColor(hdc, RGB(20, 20, 20));
        char buf[256];
        snprintf(buf, 256, "SETTINGS (F1-Close):\n1-3: Mode (%d)\n[ ]: Len (%d)\n- =: Thick (%d)\n9 0: Gap (%d)\nPgUp/Dn: Dot (%d)\nC: Color (%s)\nArrows: Move", mode, length, thickness, gap, dotSize, colorNames[colorIdx]);
        DrawText(hdc, buf, -1, &ps.rcPaint, DT_LEFT);
        EndPaint(h, &ps); return 0;
    }
    return DefWindowProc(h, m, w, l);
}

int WINAPI WinMain(HINSTANCE hI, HINSTANCE hP, LPSTR lpC, int nS) {
    WNDCLASS wc = {0}, mc = {0};
    wc.lpfnWndProc = WindowProc; wc.hInstance = hI; wc.lpszClassName = "CrosshairV4";
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH); RegisterClass(&wc);
    mc.lpfnWndProc = MenuProc; mc.hInstance = hI; mc.lpszClassName = "MenuClassV4";
    mc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(20, 20, 20)); RegisterClass(&mc);

    posX = GetSystemMetrics(SM_CXSCREEN)/2; posY = GetSystemMetrics(SM_CYSCREEN)/2;
    hwnd = CreateWindowEx(WS_EX_TOPMOST|WS_EX_LAYERED|WS_EX_TRANSPARENT|WS_EX_TOOLWINDOW, "CrosshairV4", "Ch", WS_POPUP, posX-winSize/2, posY-winSize/2, winSize, winSize, NULL, NULL, hI, NULL);
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
    hwndMenu = CreateWindowEx(WS_EX_TOPMOST|WS_EX_TOOLWINDOW, "MenuClassV4", "Menu", WS_POPUP, 10, 10, 200, 160, NULL, NULL, hI, NULL);

    RegisterHotKey(NULL, ID_HOT_QUIT, MOD_CONTROL | MOD_ALT, 0x58);
    RegisterHotKey(NULL, ID_HOT_TOGGLE, 0, VK_F2); RegisterHotKey(NULL, ID_HOT_EDIT, 0, VK_F1);
    ShowWindow(hwnd, SW_SHOW);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_HOTKEY) {
            int k = msg.wParam;
            if (k == ID_HOT_QUIT) break;
            if (k == ID_HOT_TOGGLE) { visible = !visible; ShowWindow(hwnd, visible ? SW_SHOW : SW_HIDE); }
            if (k == ID_HOT_EDIT) {
                editMode = !editMode; ShowWindow(hwndMenu, editMode ? SW_SHOW : SW_HIDE);
                if (editMode) {
                    RegisterHotKey(NULL, 10, 0, VK_UP); RegisterHotKey(NULL, 11, 0, VK_DOWN);
                    RegisterHotKey(NULL, 12, 0, VK_LEFT); RegisterHotKey(NULL, 13, 0, VK_RIGHT);
                    for(int i=0; i<3; i++) RegisterHotKey(NULL, 14+i, 0, '1'+i);
                    RegisterHotKey(NULL, 17, 0, 0xDB); RegisterHotKey(NULL, 18, 0, 0xDD);
                    RegisterHotKey(NULL, 19, 0, 0xBD); RegisterHotKey(NULL, 20, 0, 0xBB);
                    RegisterHotKey(NULL, 21, 0, '9'); RegisterHotKey(NULL, 22, 0, '0');
                    RegisterHotKey(NULL, 23, 0, VK_PRIOR); RegisterHotKey(NULL, 24, 0, VK_NEXT);
                    RegisterHotKey(NULL, 25, 0, 'C');
                } else { for(int i=10; i<=25; i++) UnregisterHotKey(NULL, i); }
            }
            if (editMode) {
                if (k==10) posY--; if (k==11) posY++; if (k==12) posX--; if (k==13) posX++;
                if (k==14) mode=1; if (k==15) mode=2; if (k==16) mode=3;
                if (k==17 && length > 1) length--; if (k==18 && length < 80) length++;
                if (k==19 && thickness > 1) thickness--; if (k==20 && thickness < 15) thickness++;
                if (k==21 && gap > 0) gap--; if (k==22 && gap < 50) gap++;
                if (k==23 && dotSize < 40) dotSize++; if (k==24 && dotSize > 1) dotSize--;
                if (k==25) colorIdx = (colorIdx + 1) % 5;
                UpdatePos();
            }
        }
        TranslateMessage(&msg); DispatchMessage(&msg);
    }
    return 0;
}