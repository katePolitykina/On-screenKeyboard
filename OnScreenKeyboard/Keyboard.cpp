
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <string> 
#include <vector>
#include <iostream>
#include <map>
#include <unordered_map>
#include "KeyboardView.h"


#define WM_INPUTLANGCHANGE 0x0051
//
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
const UINT macKeyboardLayout[] = {
    // Ряд 1
    VK_ESCAPE, VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6, VK_F7, VK_F8, VK_F9, VK_F10, VK_F11, VK_F12,

    // Ряд 2
    VK_OEM_3, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', VK_OEM_MINUS, VK_OEM_PLUS, VK_BACK,

    // Ряд 3
    VK_TAB, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', VK_OEM_4, VK_OEM_6, VK_OEM_5,

    // Ряд 4
    VK_CAPITAL, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', VK_OEM_1, VK_OEM_7, VK_RETURN,

    // Ряд 5
    VK_LSHIFT, 'Z', 'X', 'C', 'V', 'B', 'N', 'M', VK_OEM_COMMA, VK_OEM_PERIOD, VK_OEM_2, VK_RSHIFT,

    // Ряд 6
    VK_LCONTROL, VK_LMENU, VK_LWIN, VK_SPACE, VK_RWIN, VK_RMENU, VK_LEFT, VK_UP, VK_DOWN, VK_RIGHT
};

KeyboardView keyboard;


struct Key {
    RECT rect;
    bool isClicked;

    Key(RECT r) : rect(r), isClicked(false) {}
    Key(LONG left, LONG top, LONG right, LONG bottom)
        : rect{ left, top, right, bottom }, isClicked(false) {}
};

std::wstring GetKeyDisplaySymbol(int keyCode) {
    // Создаем маппинг виртуальных кодов клавиш на их отображаемые символы
    std::unordered_map<int, std::wstring> keyMap = {
        { 0x1b, L"ESC" },          // ESC
        { 0x70, L"F1" },           // F1
        { 0x71, L"F2" },           // F2
        { 0x72, L"F3" },           // F3
        { 0x73, L"F4" },           // F4
        { 0x74, L"F5" },           // F5
        { 0x75, L"F6" },           // F6
        { 0x76, L"F7" },           // F7
        { 0x77, L"F8" },           // F8
        { 0x78, L"F9" },           // F9
        { 0x79, L"F10" },          // F10
        { 0x7a, L"F11" },          // F11
        { 0x7b, L"F12" },          // F12
        { 0x09, L"Tab" },         // Tab
        { 0x08, L"Del" },       // Delete
        { 0x14, L"Caps" },    // Caps Lock
        { 0x0d, L"Return" },       // Return (Enter)
        { 0x25, L"◁" },            // Left Arrow
        { 0x26, L"△" },            // Up Arrow
        { 0x28, L"▽" },            // Down Arrow
        { 0x27, L"▷" },            // Right Arrow

        // Левые и правые клавиши
        { 0x5B, L"Wnd" },  // Left Command (Windows)ПF
        { 0x5C, L"Wnd" }, // Right Command (Windows)
        { 0xA0, L"Shift" },   // Левый Shift
        { 0xA1, L"Shift" },  // Правый Shift
        { 0xA2, L"Ctrl" }, // Левый Control
        { 0xA3, L"Ctrl" },// Правый Control
        { 0xA4, L"Alt" },  // Левый Option (Alt)
        { 0xA5, L"Alt" }  // Правый Option (Alt)
    };

    // Пытаемся найти символ по коду клавиши
    if (keyMap.find(keyCode) != keyMap.end()) {
        return keyMap[keyCode];
    }
    else {
        return L"";
    }
}
std::vector<Key> keyPos;

static HWND previousActiveWindow = NULL;
static HWND hwnd;
HHOOK keyboardHook;
HHOOK languageHook;
bool isKeyPressed = false;
std::map<UINT, bool> modifierState = {
    {VK_RSHIFT, false},
    {VK_LSHIFT, false},
    {VK_LCONTROL, false},
    {VK_LMENU, false},
    {VK_RMENU, false},  // Alt
    {VK_LWIN, false},     // Command (левая)
    {VK_RWIN, false}      // Command (правая)
};

// Преобразование виртуального кода в символ
std::wstring GetCharFromVirtualKey(const UINT vkCode) {
    std::wstring res = GetKeyDisplaySymbol(vkCode);
    if (res.length() != 0)
        return res;
    BYTE keyboardState[256] = { 0 };
    WCHAR buffer[5] = { 0 };

    HKL keyboardLayout = GetKeyboardLayout(0);
    LANGID langId = LOWORD((DWORD_PTR)keyboardLayout);

    // Получаем текущее состояние клавиш
    if (!GetKeyboardState(keyboardState)) {
        return L""; // Ошибка получения состояния клавиатуры
    }
    bool isLeftShiftPressed = keyboardState[VK_LSHIFT] & 0x80;
    bool isRightShiftPressed = keyboardState[VK_RSHIFT] & 0x80;
    bool isLeftShiftPressed1 = (GetKeyState(VK_LSHIFT) & 0x8000) != 0;
    // Устанавливаем состояние клавиш модификаторов
    if (GetKeyState(VK_SHIFT) & 0x8000) {
        keyboardState[VK_SHIFT] = 0x80; // Задаём состояние нажатой клавиши Shift
    }
    if (GetKeyState(VK_LCONTROL) & 0x8000) {
        keyboardState[VK_CONTROL] = 0;
        keyboardState[VK_LCONTROL] = 0; // Задаём состояние нажатой клавиши Ctrl
    }

    switch (langId) {
    case 0x0409: // Английский (США)
        printf("Current language: English (US)\n");
        break;
    case 0x0419: // Русский
        printf("Current language: Russian\n");
        break;
    default:
        printf("Unknown language: 0x%x\n", langId);
    }

    // Преобразуем виртуальный код в символ
    int result = ToUnicodeEx(
        vkCode,                          // Виртуальный код клавиши
        MapVirtualKeyEx(vkCode, 0, keyboardLayout), // Скан-код
        keyboardState,                   // Состояние клавиш
        buffer,                          // Буфер для символа
        4,                               // Размер буфера
        0,                               // Флаги (не используются)
        keyboardLayout                   // Раскладка клавиатуры
    );

    if (result > 0) {
        return std::wstring(1, buffer[0]); // Возвращаем первый символ как строку
    }

    return L""; // Если символа нет
}

void DrawKey(HDC hdc, const Key& rect, const UINT key) {
    // Создаём кисть для фона клавиши
    HBRUSH brush = CreateSolidBrush(rect.isClicked ? RGB(192, 192, 192) : RGB(255, 255, 255));
    FillRect(hdc, &rect.rect, brush);
    DeleteObject(brush);

    // Рисуем рамку клавиши
    FrameRect(hdc, &rect.rect, (HBRUSH)GetStockObject(BLACK_BRUSH));

    SetBkMode(hdc, TRANSPARENT); // Прозрачный фон текста
    SetTextColor(hdc, rect.isClicked ? RGB(255, 255, 255) : RGB(0, 0, 0)); // Устанавливаем цвет текста

    // Получаем символ клавиши
    std::wstring keyText = GetCharFromVirtualKey(key);

    // Создаём копию RECT для корректного отображения текста
    RECT textRect = rect.rect;

    // Рисуем текст
    DrawText(hdc, keyText.c_str(), -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}


void DrawKeyboard(HDC hdc) {
    for (int i = 0; i < keyPos.size(); i++) {
        DrawKey(hdc, keyPos[i], macKeyboardLayout[i]);
    }
}

void DebugActiveWindow()
{
    HWND hForegroundWindow = GetForegroundWindow(); // Получаем активное окноa

    if (hForegroundWindow != NULL)
    {
        wchar_t windowTitle[256];
        GetWindowText(hForegroundWindow, windowTitle, sizeof(windowTitle) / sizeof(wchar_t)); // Получаем заголовок окна

        wchar_t debugMessage[512];
        StringCchPrintf(debugMessage, 512, L"Активное окно: %s (HWND: 0x%X)\n", windowTitle, (unsigned int)hForegroundWindow);

        OutputDebugString(debugMessage); // Выводим сообщение в отладчик
    }
    else
    {
        OutputDebugString(L"Активное окно не найдено.\n");
    }
}

void GenerateKeys(int width, int height) {
    keyPos.clear();
    keyboard.CalculateRowHeight(height);
    int rowTop = 0;
    int rowBottom = 0;
    for (int i = 0; i < keyboard.numbOfRows; i++) {
        int numberOfKewsInRow = keyboard.keySizes[i].size();

        rowTop = rowBottom;
        if (i == 0) {
            rowBottom += keyboard.padding;
        }
        rowBottom += keyboard.rowHeight * keyboard.keyHeight[i][0];
        int standartKeyWidth = width / keyboard.normalKeyWidth[i];
        int right = 0;
        int left = 0;
        int numberOfBigKeys = 0;
        for (bool value : keyboard.keySizes[i]) {
            if (!value) { // Если значение равно 0 (false)
                numberOfBigKeys++;
            }
        }
        int additionalWidth = 0;
        if (numberOfBigKeys > 0) {
            additionalWidth = (width - standartKeyWidth * numberOfKewsInRow) / numberOfBigKeys;
        }
        for (int j = 0; j < numberOfKewsInRow; j++) {
            left = right;
            right += standartKeyWidth;
            if (!keyboard.keySizes[i][j]) {
                right += additionalWidth;
            }
            RECT escRect = { left, rowTop, right, rowBottom };
            keyPos.push_back(escRect);
        }
    }

}

// Обработчик событий клавиатуры
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
            StringCchPrintf(debugMessage, 512, L"Нажата клавиша: %x\n", (unsigned int)kbdStruct->vkCode);
            OutputDebugString(debugMessage);

        }
        else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            // Обработка отпускания клавиши
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
//        // Отслеживаем сообщения смены языка
//        if (msg->message == WM_INPUTLANGCHANGEREQUEST || msg->message == WM_INPUTLANGCHANGE) {
//            std::cout << "Смена языка произошла!" << std::endl;
//            // Здесь можно добавить дополнительные действия, например, обработку смены языка
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
         std::cerr << "Не удалось установить хук!" << std::endl;
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
            OutputDebugString(L"Окно keyboard деактивировано.\n");
            DebugActiveWindow();
        }
        break;
    case WM_SIZE: {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        GenerateKeys(width, height);
        InvalidateRect(hwnd, nullptr, TRUE); // Перерисовать окно
    } break;
    case WM_KILLFOCUS: {
        OutputDebugString(L"WM_KILLFOCUS\n");
        OutputDebugString(L"Окно keyboard потеряло фокус.\n");
        DebugActiveWindow();
    }
                     break;
    case WM_SETFOCUS: {
        OutputDebugString(L"WM_SETFOCUS\n");
        OutputDebugString(L"Окно keyboard получило фокус.\n");
        DebugActiveWindow();
    }
                    break;




    case WM_LBUTTONDOWN: {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        // Проверяем, попал ли клик в один из прямоугольников
        int keyPosSize = keyPos.size();
        for (int i = 0; i < keyPosSize; ++i) {
            if (PtInRect(&keyPos[i].rect, { x, y })) {
                UINT vkCode = macKeyboardLayout[i];
                if (modifierState.find(vkCode) != modifierState.end()) {
                    // Переключаем состояние модификаторной клавиши
                    modifierState[vkCode] = !modifierState[vkCode];
                    // Готовим INPUT структуру
                    INPUT input = { 0 };
                    input.type = INPUT_KEYBOARD;
                    input.ki.wVk = vkCode;
                    if (!modifierState[vkCode]) {
                        // Если клавиша отпущена, добавляем флаг KEYEVENTF_KEYUP
                        input.ki.dwFlags = KEYEVENTF_KEYUP;
                        
                    }
                    SendInput(1, &input, sizeof(INPUT));
                    if (!modifierState[vkCode]) {

                        for (auto it = modifierState.begin(); it != modifierState.end(); ++it) {
                            UINT modKey = it->first;
                            bool isActive = it->second;

                            if (isActive) {
                                INPUT modInput = { 0 };
                                modInput.type = INPUT_KEYBOARD;
                                modInput.ki.wVk = modKey;
                                modInput.ki.dwFlags = KEYEVENTF_KEYUP;
                                SendInput(1, &modInput, sizeof(INPUT));
                                modifierState[modKey] = false;
                            }
                        }
                    }
                }
                else {
                    // Обычная клавиша, отправляем нажатие
                    INPUT input = { 0 };
                    input.type = INPUT_KEYBOARD;
                    input.ki.wVk = vkCode;
                    // Симулируем нажатие
                    SendInput(1, &input, sizeof(INPUT));
                }
                break;
            }
        }
    } break;
    case WM_LBUTTONUP: {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        // Проверяем, попал ли клик в один из прямоугольников
        int keyPosSize = keyPos.size();
        for (int i = 0; i < keyPosSize; ++i) {
            if (PtInRect(&keyPos[i].rect, { x, y })) {
                UINT vkCode = macKeyboardLayout[i];
                // Проверяем, не является ли клавиша модификаторной
                if (modifierState.find(vkCode) == modifierState.end()) {
                    // Для обычных клавиш отправляем отпускание
                    INPUT input = { 0 };
                    input.type = INPUT_KEYBOARD;
                    input.ki.wVk = vkCode;
                    input.ki.dwFlags = KEYEVENTF_KEYUP;
                    // Симулируем отпусканиеАЧ
                    SendInput(1, &input, sizeof(INPUT));
                    for (auto it = modifierState.begin(); it != modifierState.end(); ++it) {
                        UINT modKey = it->first;
                        bool isActive = it->second;

                        if (isActive) {
                            INPUT modInput = { 0 };
                            modInput.type = INPUT_KEYBOARD;
                            modInput.ki.wVk = modKey;
                            modInput.ki.dwFlags = KEYEVENTF_KEYUP;
                            SendInput(1, &modInput, sizeof(INPUT));
                            modifierState[modKey] = false;
                        }
                    }
                }
                break;
            }
        }
    } break;
    case WM_DESTROY:
        PostQuitMessage(0);

        break;

        //case WM_PAINT: {
        //    PAINTSTRUCT ps;
        //    HDC hdc = BeginPaint(hwnd, &ps);
        //    // Очистка окна: закрашиваем фон
        //    RECT rect;
        //    GetClientRect(hwnd, &rect);
        //  //  HBRUSH hBrush = CreateSolidBrush(isKeyPressed ? RGB(200, 200, 200) : RGB(255, 255, 255)); // Серый или белый
        //    HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255)); // Белый фонa
        //    FillRect(hdc, &rect, hBrush);
        //    DeleteObject(hBrush);
        //    // Отрисовка клавиатуры
        //    DrawKeyboard(hdc);
        //    EndPaint(hwnd, &ps);
        //    return 0; 
        //} break;


    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        // Получаем размеры окна
        RECT rect;
        GetClientRect(hwnd, &rect);
        // Создаем совместимый контекст устройства памяти
        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hBitmap = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
        HGDIOBJ hOldBitmap = SelectObject(hdcMem, hBitmap);
        // Заливаем фон в буфере памяти
        HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255)); // Белый фон
        FillRect(hdcMem, &rect, hBrush);
        DeleteObject(hBrush);
        // Отрисовка клавиатуры в буфере памяти
        DrawKeyboard(hdcMem);
        // Копируем содержимое из памяти на экран
        BitBlt(hdc, 0, 0, rect.right, rect.bottom, hdcMem, 0, 0, SRCCOPY);
        // Освобождаем ресурсы 
        SelectObject(hdcMem, hOldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(hdcMem);
        EndPaint(hwnd, &ps);
        return 0;
    } break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}