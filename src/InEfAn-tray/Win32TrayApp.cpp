#include "tray-helpers.h"

#include "Version.inl"

#include "single-instance.h"
#include "Version.inl"
#include "tray-helpers.h"

#include <windows.h>
#include <filesystem>
#include "../logger/log2file.h"
#include <tchar.h>


// handle with care
#define SECONDS *1000
#define MINUTES *(60 SECONDS)
#define HOURS   *(60 MINUTES)
#define DAYS    *(24 HOURS)

namespace
{
    // Global Variables:
    HMENU hContext = NULL;
    const WCHAR szWindowClass[] = L"InEfAnTrayApp";
}
// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);


int APIENTRY _tWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPTSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);

    char logfileDir[MAX_PATH] = {};
    GetTempPathA(MAX_PATH, logfileDir);

    MSG msg = {};
    LimitSingleInstance siLock(L"Local\\" _T(VER_SZ_PRODUCTNAME));

    try {
        Log2File logfile(std::tr2::sys::path(logfileDir) / std::tr2::sys::path("logfile.txt"));

        if (siLock.IsAnotherInstanceRunning()) {

            return 1;
        }

        MyRegisterClass(hInstance);

        HWND hWnd = CreateWindowW(
                        szWindowClass, NULL, WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

        if (!hWnd) {
            return FALSE;
        }

        UpdateWindow(hWnd);

        traydata.hWnd = hWnd;
        traydata.hIcon = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_MAINICON));

        BOOL bSuccess = Shell_NotifyIconW(NIM_ADD, &traydata);

        // Main message loop:
        while (GetMessage(&msg, NULL, 0, 0)) {
            //if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            //{
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            //}
        }

        if (hContext) {
            DestroyMenu(hContext);
            hContext = NULL;
        }

        Shell_NotifyIconW(NIM_DELETE, &traydata);

    } catch (...) {}
    siLock.Release();

    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = {
        sizeof(WNDCLASSEXW),                                    //cbSize
        CS_HREDRAW | CS_VREDRAW,                                //style
        WndProc,                                                //lpfnWndProc
        0,                                                      //cbClsExtra
        0,                                                      //cbWndExtra
        hInstance,                                              //hInstance
        LoadIconW(hInstance, MAKEINTRESOURCE(IDI_MAINICON)),    //hIcon
        LoadCursorW(NULL, IDC_ARROW),                           //hCursor
        (HBRUSH)(COLOR_WINDOW + 1),                             //hbrBackground
        MAKEINTRESOURCE(IDC_WIN32TRAYAPP),                      //lpszMenuName
        szWindowClass,                                          //lpszClassName
        LoadIconW(hInstance, MAKEINTRESOURCE(IDI_MAINICON))     //hIconSm
    };

    return RegisterClassExW(&wcex);
}

void PopulateMenu(HWND hWnd)
{
    SetForegroundWindow(hWnd);

    if (!hContext) {
        hContext = LoadMenuW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDR_MENU1));
    }
    HMENU contextMenu = GetSubMenu(hContext, 0);

    if (true/*logger.enabled()*/) {
        RemoveMenu(contextMenu, ID_TRAYMENU_RESUME, MF_BYCOMMAND);
    } else {
        RemoveMenu(contextMenu, ID_TRAYMENU_PAUSE, MF_BYCOMMAND);
    }

    POINT CursorPos;
    GetCursorPos(&CursorPos);

    TrackPopupMenu(
        contextMenu,
        TPM_LEFTALIGN | TPM_LEFTBUTTON,
        CursorPos.x,
        CursorPos.y,
        0,
        hWnd,
        NULL
    );
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    try {

        switch (message) {
            case WM_COMMAND: {
                const int wmId = LOWORD(wParam);
                // Parse the menu selections:
                switch (wmId) {
                    case ID_TRAYMENU_EXIT:
                        PostQuitMessage(0);
                        break;
                    case ID_TRAYMENU_ABOUT:
                        ShellExecuteW(NULL, L"open", L"http://" _T(VER_SZ_DOMAIN) L"/", NULL, NULL, SW_SHOWNORMAL);
                        break;
                    case ID_TRAYMENU_PAUSE:
                        break;
                    case ID_TRAYMENU_RESUME:
                        break;
                    case ID_TRAYMENU_SENDLOGFILES: {
                        //                        bool result = postLogfiles();
                        //                      MessageBoxW(hWnd, loadStdWStringFromRC(result ? IDC_LOGFILES_SENT : IDC_LOGFILES_NOTSENT).c_str(), _T(VER_SZ_PRODUCTNAME), MB_OK);
                    }
                    break;
                    default:
                        return DefWindowProc(hWnd, message, wParam, lParam);
                        break;
                }
            }
            break;
            case TRAY_ICON_MESSAGE: {
                switch (lParam) {
                    case WM_RBUTTONDOWN:
                    case WM_LBUTTONDOWN:
                        PopulateMenu(hWnd);
                        break;
                    case 0x405: // click on notification bubble
                        break;
                };
            }
            break;
            case WM_DESTROY:
                PostQuitMessage(0);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
        }

    } catch (std::exception& ee) {
        MessageBoxA(hWnd, ee.what(), PROJECT_FULLNAME, MB_ICONERROR);
    }
    return 0;
}
