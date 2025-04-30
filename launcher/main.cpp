#define UNICODE
#define _UNICODE

#include <windows.h>
#include <string>

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND widthBox, heightBox, vsyncCheckbox;

    switch (msg) {
    case WM_CREATE:
        CreateWindowW(L"STATIC", L"Width:", WS_VISIBLE | WS_CHILD,
            10, 10, 50, 20, hwnd, NULL,
            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        widthBox = CreateWindowW(L"EDIT", L"1920", WS_VISIBLE | WS_CHILD | WS_BORDER,
            70, 10, 100, 20, hwnd, NULL,
            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        CreateWindowW(L"STATIC", L"Height:", WS_VISIBLE | WS_CHILD,
            10, 40, 50, 20, hwnd, NULL,
            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        heightBox = CreateWindowW(L"EDIT", L"1080", WS_VISIBLE | WS_CHILD | WS_BORDER,
            70, 40, 100, 20, hwnd, NULL,
            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        vsyncCheckbox = CreateWindowW(L"BUTTON", L"VSync", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
            10, 70, 100, 20, hwnd, NULL,
            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        CreateWindowW(L"BUTTON", L"Launch Game", WS_VISIBLE | WS_CHILD,
            10, 100, 160, 30, hwnd, (HMENU)1,
            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
        break;

case WM_COMMAND:
    if (LOWORD(wParam) == 1) {
        wchar_t width[10], height[10];
        GetWindowTextW(widthBox, width, 10);
        GetWindowTextW(heightBox, height, 10);
        BOOL vsyncEnabled = (SendMessageW(vsyncCheckbox, BM_GETCHECK, 0, 0) == BST_CHECKED);

        std::wstring arguments = L" ";
        arguments += width;
        arguments += L" ";
        arguments += height;
        arguments += L" ";
        arguments += vsyncEnabled ? L"1" : L"0";

        std::wstring gamePath = L"..\\PKGPro3D.exe";
        std::wstring fullCommandLine = L"\"" + gamePath + L"\"" + arguments;

        std::wstring workingDirectory = L"..\\";

        STARTUPINFOW si = { sizeof(si) };
        PROCESS_INFORMATION pi;

        wchar_t* cmdLineBuffer = _wcsdup(fullCommandLine.c_str());

        if (CreateProcessW(
            NULL, cmdLineBuffer,
            NULL, NULL, FALSE, 0,
            NULL,
            workingDirectory.c_str(),
            &si, &pi))
        {
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        else {
            DWORD errorCode = GetLastError();
            wchar_t errorMsg[256];
            wsprintfW(errorMsg, L"cosik nie poszlo.\nerror code: %lu", errorCode);
            MessageBoxW(hwnd, errorMsg, L"error", MB_OK | MB_ICONERROR);
        }

        free(cmdLineBuffer);
    }
    break;


    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"assblast";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProcedure;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        0, CLASS_NAME, L"Game Launcher",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 300, 200,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL)
        return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return 0;
}
