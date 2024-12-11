
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <string> 
#include <vector>
#include <iostream>
#include <map>

#define WM_INPUTLANGCHANGE 0x0051
//
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct Key {
    RECT rect;
    bool isClicked;

    Key(RECT r) : rect(r), isClicked(false) {}
    Key(LONG left, LONG top, LONG right, LONG bottom)
        : rect{ left, top, right, bottom }, isClicked(false) {}
};


std::vector<Key> keyPos;
static HWND previousActiveWindow = NULL;
static HWND hwnd;
HHOOK keyboardHook;
HHOOK languageHook;
bool isKeyPressed = false;
std::map<UINT, bool> modifierState = {
    {VK_RSHIFT, false},
    {VK_LSHIFT, false},
    {VK_CONTROL, false},
    {VK_MENU, false},     // Alt
    {VK_LWIN, false},     // Command (�����)
    {VK_RWIN, false}      // Command (������)G
};
const UINT macKeyboardLayout[] = {
    // ��� 1
    VK_ESCAPE, VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6, VK_F7, VK_F8, VK_F9, VK_F10, VK_F11, VK_F12,

    // ��� 2
    VK_OEM_3, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', VK_OEM_MINUS, VK_OEM_PLUS, VK_BACK,

    // ��� 3
    VK_TAB, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', VK_OEM_4, VK_OEM_6, VK_OEM_5,

    // ��� 4
    VK_CAPITAL, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', VK_OEM_1, VK_OEM_7, VK_RETURN,

    // ��� 5
    VK_LSHIFT, 'Z', 'X', 'C', 'V', 'B', 'N', 'M', VK_OEM_COMMA, VK_OEM_PERIOD, VK_OEM_2, VK_RSHIFT,

    // ��� 6
    VK_CONTROL, VK_LMENU, VK_LWIN, VK_SPACE, VK_RWIN, VK_RMENU, VK_LEFT, VK_UP, VK_DOWN, VK_RIGHT
};

// �������������� ������������ ���� � ������
std::wstring GetCharFromVirtualKey(const UINT vkCode) {
    BYTE keyboardState[256] = { 0 };
    WCHAR buffer[5] = { 0 };

    /*HKL keyboardLayout;
    LANGID langId;
    int count = GetKeyboardLayoutList(1, &keyboardLayout);
    if (count > 0) {
        langId = LOWORD((DWORD_PTR)keyboardLayout);
        printf("System input language: 0x%x\n", langId);
    }
    else {
        printf("Failed to get keyboard layouts.\n");
    }*/

    //�������� ������� ��������� �����������
    HKL keyboardLayout = GetKeyboardLayout(0);
    LANGID langId = LOWORD((DWORD_PTR)keyboardLayout);

    // �������� ������� ��������� ������
    if (!GetKeyboardState(keyboardState)) {
        return L""; // ������ ��������� ��������� ����������
    }
    bool isLeftShiftPressed = keyboardState[VK_LSHIFT] & 0x80;
    bool isRightShiftPressed = keyboardState[VK_RSHIFT] & 0x80;
    bool isLeftShiftPressed1 = (GetKeyState(VK_LSHIFT) & 0x8000) != 0;
    // ������������� ��������� ������ �������������
    if (GetKeyState(VK_SHIFT) & 0x8000) {
        keyboardState[VK_SHIFT] = 0x80; // ����� ��������� ������� ������� Shift
    }
    if (GetKeyState(VK_CONTROL) & 0x8000) {
        keyboardState[VK_CONTROL] = 0x80; // ����� ��������� ������� ������� Ctrl
    }
    if (GetKeyState(VK_MENU) & 0x8000) {
        keyboardState[VK_MENU] = 0x80; // ����� ��������� ������� ������� Alt
    }
    switch (langId) {
    case 0x0409: // ���������� (���)
        printf("Current language: English (US)\n");
        break;
    case 0x0419: // �������
        printf("Current language: Russian\n");
        break;
    default:
        printf("Unknown language: 0x%x\n", langId);
    }

    // ����������� ����������� ��� � ������
    int result = ToUnicodeEx(
        vkCode,                          // ����������� ��� �������
        MapVirtualKeyEx(vkCode, 0, keyboardLayout), // ����-���
        keyboardState,                   // ��������� ������
        buffer,                          // ����� ��� �������
        4,                               // ������ ������
        0,                               // ����� (�� ������������)
        keyboardLayout                   // ��������� ����������
    );

    if (result > 0) {
        return std::wstring(1, buffer[0]); // ���������� ������ ������ ��� ������
    }

    return L""; // ���� ������� ���
}

void DrawKey(HDC hdc, const Key& rect, const UINT key) {
    // ������ ����� ��� ���� �������
    HBRUSH brush = CreateSolidBrush(rect.isClicked ? RGB(192, 192, 192) : RGB(255, 255, 255));
    FillRect(hdc, &rect.rect, brush);
    DeleteObject(brush);

    // ������ ����� �������
    FrameRect(hdc, &rect.rect, (HBRUSH)GetStockObject(BLACK_BRUSH));

    // ���������� �����
    int width = rect.rect.right - rect.rect.left;
    int height = rect.rect.bottom - rect.rect.top;

    SetBkMode(hdc, TRANSPARENT); // ���������� ��� ������
    SetTextColor(hdc, rect.isClicked ? RGB(255, 255, 255) : RGB(0, 0, 0)); // ������������� ���� ������

    // �������� ������ �������
    std::wstring keyText = GetCharFromVirtualKey(key);

    // ������ ����� RECT ��� ����������� ����������� ������
    RECT textRect = rect.rect;

    // ������ �����
    DrawText(hdc, keyText.c_str(), -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}


void DrawKeyboard(HDC hdc) {
    for (int i = 0; i < keyPos.size(); i++) {
        DrawKey(hdc, keyPos[i], macKeyboardLayout[i]);
    }
}

void DebugActiveWindow()
{
    HWND hForegroundWindow = GetForegroundWindow(); // �������� �������� ����a

    if (hForegroundWindow != NULL)
    {
        wchar_t windowTitle[256];
        GetWindowText(hForegroundWindow, windowTitle, sizeof(windowTitle) / sizeof(wchar_t)); // �������� ��������� ����

        wchar_t debugMessage[512];
        StringCchPrintf(debugMessage, 512, L"�������� ����: %s (HWND: 0x%X)\n", windowTitle, (unsigned int)hForegroundWindow);

        OutputDebugString(debugMessage); // ������� ��������� � ��������
    }
    else
    {
        OutputDebugString(L"�������� ���� �� �������.\n");
    }
}

void GenerateKeys(int width, int height) {
    keyPos.clear();
    int rowHeight = height / 11; // 5 ����� ������� � 1 � ��� ���� ������

    int remainingWidth;        // ��� ������ �������� �� ����
    int keyWidth;              // ������� ������ �������

    // --- 1. ������� ��� ---
    int rowTop = 0;
    int rowBottom = height % 11 + rowHeight;
    rowHeight *= 2;

    keyWidth = width / 13;

    RECT escRect = { 0, 0, keyWidth + width % 13, rowBottom };
    keyPos.push_back(escRect);

    for (int i = 1; i <= 12; ++i) {
        RECT keyRect = { escRect.right + keyWidth * (i - 1), 0,
                        escRect.right + keyWidth * i, rowBottom };
        keyPos.push_back(keyRect);
    }


    // --- 2. ������ ��� ---
    keyWidth = width / 14;
    rowTop = rowBottom;
    rowBottom += rowHeight;

    RECT tildaRect = { 0, rowTop, keyWidth,rowBottom };
    keyPos.push_back(tildaRect);
    for (int i = 1; i < 13; ++i) {
        RECT keyRect = { keyWidth * i, rowTop, keyWidth * (i + 1),rowBottom };
        keyPos.push_back(keyRect);
    }

    RECT deleteRect = { keyWidth * 13, rowTop, width, rowBottom };
    keyPos.push_back(deleteRect);


    // --- 3. ������ ��� ---
    rowTop = rowBottom;
    rowBottom += rowHeight;

    remainingWidth = width % 14;

    RECT tabRect = { 0, rowTop, keyWidth + remainingWidth, rowBottom };
    keyPos.push_back(tabRect);

    for (int i = 1; i < 14; ++i) {
        RECT keyRect = { tabRect.right + keyWidth * (i - 1), rowTop,
                        tabRect.right + keyWidth * i, rowBottom };
        keyPos.push_back(keyRect);
    }

    // --- 4. ��������� ��� ---
    rowTop = rowBottom;
    rowBottom += rowHeight;

    remainingWidth = width - keyWidth * 11;

    RECT capsRect = { 0, rowTop, remainingWidth / 2,rowBottom };
    keyPos.push_back(capsRect);

    for (int i = 1; i < 12; ++i) {
        RECT keyRect = { capsRect.right + keyWidth * (i - 1), rowTop,
                        capsRect.right + keyWidth * i, rowBottom };
        keyPos.push_back(keyRect);
    }

    RECT returnRect = { capsRect.right + keyWidth * 11, rowTop,
                       width, rowBottom };
    keyPos.push_back(returnRect);

    // --- 5. ����� ��� ---
    rowTop = rowBottom;
    rowBottom += rowHeight;

    remainingWidth = width - keyWidth * 10;

    RECT leftShiftRect = { 0, rowTop,remainingWidth / 2, rowBottom };
    keyPos.push_back(leftShiftRect);

    for (int i = 1; i < 11; ++i) {
        RECT keyRect = { leftShiftRect.right + keyWidth * (i - 1),rowTop,
                        leftShiftRect.right + keyWidth * i, rowBottom };
        keyPos.push_back(keyRect);
    }

    RECT rightShiftRect = { leftShiftRect.right + keyWidth * 10, rowTop,
                           width, rowBottom };
    keyPos.push_back(rightShiftRect);

    // --- 6. ������ ��� --- 
    rowTop = rowBottom;
    rowBottom += rowHeight;
    remainingWidth = width - keyWidth * 8;
    for (int i = 0; i < 3; ++i) {
        RECT keyRect = { keyWidth * i, rowTop,
                        keyWidth * (i + 1), rowBottom };
        keyPos.push_back(keyRect);
    }

    RECT spaceRect = { keyWidth * 3, rowTop, keyWidth * 3 + remainingWidth, rowBottom };
    keyPos.push_back(spaceRect);

    for (int i = 1; i < 3; ++i) {
        RECT keyRect = { spaceRect.right + keyWidth * (i - 1),rowTop,
                        spaceRect.right + keyWidth * i, rowBottom };
        keyPos.push_back(keyRect);
    }

    // C���������
    RECT LeftRect = { spaceRect.right + keyWidth * 2, rowTop + rowHeight / 2, spaceRect.right + keyWidth * 3, rowBottom };
    keyPos.push_back(LeftRect);

    keyPos.push_back(Key(LeftRect.right, rowTop, LeftRect.right + keyWidth, rowTop + rowHeight / 2));
    keyPos.push_back(Key(LeftRect.right, rowTop + rowHeight / 2, LeftRect.right + keyWidth, rowBottom));
    keyPos.push_back(Key(LeftRect.right + keyWidth, rowTop + rowHeight / 2, width, rowBottom));


}
// ���������� ������� ����������
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* kbdStruct = (KBDLLHOOKSTRUCT*)lParam;
        int keyPosSize = keyPos.size();
        if (keyPosSize == 0) {
            return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
        }
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {

            int arraySize = sizeof(macKeyboardLayout) / sizeof(macKeyboardLayout[0]);
            for (int i = 0; i < arraySize; ++i) {
                if (macKeyboardLayout[i] == kbdStruct->vkCode) {
                    keyPos[i].isClicked = true;
                    InvalidateRect(hwnd, 0, TRUE);
                    break;
                }
            }

            wchar_t debugMessage[512];
            StringCchPrintf(debugMessage, 512, L"������ �������: %x\n", (unsigned int)kbdStruct->vkCode);
            OutputDebugString(debugMessage);

        }
        else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            // ��������� ���������� �������
            int arraySize = sizeof(macKeyboardLayout) / sizeof(macKeyboardLayout[0]);
            for (int i = 0; i < arraySize; ++i) {
                if (macKeyboardLayout[i] == kbdStruct->vkCode) {
                    keyPos[i].isClicked = false;
                    InvalidateRect(hwnd, 0, TRUE);
                    break;
                }
            }


        }
    }
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}
//LRESULT CALLBACK MsgProc(int nCode, WPARAM wParam, LPARAM lParam) {
//    if (nCode == HC_ACTION) {
//        MSG* msg = (MSG*)lParam;
//
//        // ����������� ��������� ����� �����
//        if (msg->message == WM_INPUTLANGCHANGEREQUEST || msg->message == WM_INPUTLANGCHANGE) {
//            std::cout << "����� ����� ���������!" << std::endl;
//            // ����� ����� �������� �������������� ��������, ��������, ��������� ����� �����
//        }
//    }
//    return CallNextHookEx(languageHook, nCode, wParam, lParam);
//}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    // Register the window class.            
    const wchar_t CLASS_NAME[] = L"OnScreenKeyboard";
    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);
    // Create the window.A

    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST
        | WS_EX_NOACTIVATE,                  // Optional window styles.
        CLASS_NAME,                     // Window class
        L"On-screen keyboard",          // Window text
        WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 200,
        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );
    if (hwnd == NULL)
    {
        return -1;
    }

    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    // languageHook = SetWindowsHookEx(WH_CALLWNDPROC, MsgProc, NULL, GetCurrentThreadId());
    /* if (languageHook == NULL) {
         std::cerr << "�� ������� ���������� ���!" << std::endl;
         return 1;
     }*/




    ShowWindow(hwnd, nCmdShow);

    // Run the message loop.

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnhookWindowsHookEx(keyboardHook);
    //    UnhookWindowsHookEx(languageHook);
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {


    case WM_ACTIVATE:
        OutputDebugString(L"WM_ACTIVATE\n");
        DebugActiveWindow();
        if (LOWORD(wParam) == WA_INACTIVE) {
            OutputDebugString(L"���� keyboard ��������������.\n");
            DebugActiveWindow();
        }
        break;
    case WM_SIZE: {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        GenerateKeys(width, height);
        InvalidateRect(hwnd, nullptr, TRUE); // ������������ ����
    } break;
    case WM_KILLFOCUS: {
        OutputDebugString(L"WM_KILLFOCUS\n");
        OutputDebugString(L"���� keyboard �������� �����.\n");
        DebugActiveWindow();
    }
                     break;
    case WM_SETFOCUS: {
        OutputDebugString(L"WM_SETFOCUS\n");
        OutputDebugString(L"���� keyboard �������� �����.\n");
        DebugActiveWindow();
    }
                    break;




    case WM_LBUTTONDOWN: {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        // ���������, ����� �� ���� � ���� �� ���������������
        int keyPosSize = keyPos.size();
        for (int i = 0; i < keyPosSize; ++i) {
            if (PtInRect(&keyPos[i].rect, { x, y })) {
                UINT vkCode = macKeyboardLayout[i];
                if (modifierState.find(vkCode) != modifierState.end()) {
                    // ����������� ��������� �������������� �������
                    modifierState[vkCode] = !modifierState[vkCode];
                    // ������� INPUT ���������
                    INPUT input = { 0 };
                    input.type = INPUT_KEYBOARD;
                    input.ki.wVk = vkCode;
                    if (!modifierState[vkCode]) {
                        // ���� ������� ��������, ��������� ���� KEYEVENTF_KEYUP
                        input.ki.dwFlags = KEYEVENTF_KEYUP;
                    }
                    SendInput(1, &input, sizeof(INPUT));
                }
                else {
                    // ������� �������, ���������� �������
                    INPUT input = { 0 };
                    input.type = INPUT_KEYBOARD;
                    input.ki.wVk = vkCode;
                    // ���������� �������
                    SendInput(1, &input, sizeof(INPUT));
                }
                //INPUT input = { 0 };
                //// Set up the INPUT structure
                //input.type = INPUT_KEYBOARD;
                //input.ki.wVk = macKeyboardLayout[i]; // Virtual-Key Code for 'A'
                //// Simulate the key press ��������
                //SendInput(1, &input, sizeof(INPUT));
                break;
            }
        }
        /*
        previousActiveWindow = GetForegroundWindow();
        OutputDebugString(L"WM_LBUTTONDOWN\n");
        DebugActiveWindow();
        wchar_t debugMessage[512];
        StringCchPrintf(debugMessage, 512, L"�������� ����:(HWND: 0x%X)\n", (unsigned int)previousActiveWindow);
        OutputDebugString(debugMessage);
        StringCchPrintf(debugMessage, 512, L"���� � getfocus:(HWND: 0x%X)\n", (unsigned int)GetFocus());
        OutputDebugString(debugMessage);
        if (previousActiveWindow != hwnd && previousActiveWindow != NULL) {
            // ���������� ���������� ����
            SetForegroundWindow(previousActiveWindow);
            // ���������� � ��� ���� ��������� � ������� ������� 'A'A
           // SendMessage(previousActiveWindow, WM_CHAR, 'A', 0);
            INPUT input = { 0 };
            // Set up the INPUT structure
            input.type = INPUT_KEYBOARD;
            input.ki.wVk = 0x41; // Virtual-Key Code for 'A'
            // Simulate the key press
            SendInput(1, &input, sizeof(INPUT));
            //// Simulate the key release
            //input.ki.dwFlags = KEYEVENTF_KEYUP;
            //SendInput(1, &input, sizeof(INPUT));
        }
        */
    } break;


        //case WM_LBUTTONUP: {
        //    int x = LOWORD(lParam);
        //    int y = HIWORD(lParam);
        //    int keyPosSize = keyPos.size();
        //    for (int i = 0; i < keyPosSize; ++i) {
        //        if (PtInRect(&keyPos[i].rect, { x, y })) {
        //            INPUT input = { 0 };
        //            // Set up the INPUT structure    
        //            input.type = INPUT_KEYBOARD;
        //            input.ki.wVk = macKeyboardLayout[i]; // Virtual-Key Code for 'A'
        //            input.ki.dwFlags = KEYEVENTF_KEYUP;
        //            // Simulate the key press
        //            SendInput(1, &input, sizeof(INPUT));
        //            break;
        //        }
        //    }
        //}
        //break;


    case WM_LBUTTONUP: {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        // ���������, ����� �� ���� � ���� �� ���������������
        int keyPosSize = keyPos.size();
        for (int i = 0; i < keyPosSize; ++i) {
            if (PtInRect(&keyPos[i].rect, { x, y })) {
                UINT vkCode = macKeyboardLayout[i];
                // ���������, �� �������� �� ������� ��������������
                if (modifierState.find(vkCode) == modifierState.end()) {
                    // ��� ������� ������ ���������� ����������
                    INPUT input = { 0 };
                    input.type = INPUT_KEYBOARD;
                    input.ki.wVk = vkCode;
                    input.ki.dwFlags = KEYEVENTF_KEYUP;
                    // ���������� ����������
                    SendInput(1, &input, sizeof(INPUT));
                }
                break;
            }
        }
    } break;
    case WM_DESTROY:
        PostQuitMessage(0);

        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        // ������� ����: ����������� ���
        RECT rect;
        GetClientRect(hwnd, &rect);
        //  HBRUSH hBrush = CreateSolidBrush(isKeyPressed ? RGB(200, 200, 200) : RGB(255, 255, 255)); // ����� ��� �����
        HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255)); // ����� ���a
        FillRect(hdc, &rect, hBrush);
        DeleteObject(hBrush);
        // ��������� ����������
        DrawKeyboard(hdc);
        EndPaint(hwnd, &ps);
        return 0;
    } break;


        //case WM_PAINT: {
        //    PAINTSTRUCT ps;
        //    HDC hdc = BeginPaint(hwnd, &ps);
        //    // �������� ������� ����
        //    RECT rect;
        //    GetClientRect(hwnd, &rect);
        //    // ������� ����������� �������� ���������� ������
        //    HDC hdcMem = CreateCompatibleDC(hdc);
        //    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
        //    HGDIOBJ hOldBitmap = SelectObject(hdcMem, hBitmap);
        //    // �������� ��� � ������ ������
        //    HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255)); // ����� ���
        //    FillRect(hdcMem, &rect, hBrush);
        //    DeleteObject(hBrush);
        //    // ��������� ���������� � ������ ������
        //    DrawKeyboard(hdcMem);
        //    // �������� ���������� �� ������ �� �����
        //    BitBlt(hdc, 0, 0, rect.right, rect.bottom, hdcMem, 0, 0, SRCCOPY);
        //    // ����������� �������
        //    SelectObject(hdcMem, hOldBitmap);
        //    DeleteObject(hBitmap);
        //    DeleteDC(hdcMem);
        //    EndPaint(hwnd, &ps);
        //    return 0;
        //} break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}