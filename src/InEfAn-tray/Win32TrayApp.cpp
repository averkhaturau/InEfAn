#include "tray-helpers.h"

#include "Version.inl"

#include "single-instance.h"
#include "Version.inl"
#include "tray-helpers.h"

#include <windows.h>
#include <filesystem>
#include "logger/logger.h"
#include <tchar.h>

#include "input-hooker/input-hooker.h"
#include "events-logging.h"

#include "app-id.h"
#include "backend-bridge.h"

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

    MSG msg = {};
    LimitSingleInstance siLock(L"Local\\" _T(VER_SZ_PRODUCTNAME));

    try {

        if (siLock.IsAnotherInstanceRunning()) {
            return 1;
        }

        Logger::instance() << "Starting InEfAn";
        initEventsListening();

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

        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (hContext) {
            DestroyMenu(hContext);
            hContext = NULL;
        }

        Logger::instance() << "Exiting InEfAn";
    } catch (std::exception& ee) {
        Logger::instance() << ee.what() << " exiting...";
    } catch (...) {}

    Shell_NotifyIconW(NIM_DELETE, &traydata);
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


    if (Logger::instance().isEnabled()) {
        EnableMenuItem(contextMenu, ID_TRAYMENU_PAUSE, MF_BYCOMMAND | MF_ENABLED);
        EnableMenuItem(contextMenu, ID_TRAYMENU_RESUME, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

    } else {
        EnableMenuItem(contextMenu, ID_TRAYMENU_PAUSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        EnableMenuItem(contextMenu, ID_TRAYMENU_RESUME, MF_BYCOMMAND | MF_ENABLED);
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

// saves current logfile with timestamp and create new logfile. return old logfile name.
auto rotateLogfile()
{
    Logger::instance() << "Starting new logfile";
    Logger::instance().enable(false);
    const auto archFilename = Logger::instance().logFilename().replace_extension(timestamp_filename() + ".txt");
    std::tr2::sys::rename(Logger::instance().logFilename(), archFilename);
    Logger::instance().enable(true);
    Logger::instance() << "Continuing the logfile " << archFilename.string();
    return archFilename;
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
                        Logger::instance().enable(false);
                        trayIconUpdate(IDI_MAINICONPAUSED, IDC_LOGGING_PAUSED);
                        break;
                    case ID_TRAYMENU_RESUME:
                        Logger::instance().enable(true);
                        trayIconUpdate(IDI_MAINICON, IDC_LOGGING_RESUMED);
                        break;
                    case ID_TRAYMENU_SENDLOGFILES: {
                        allowFirewallForMe();
                        bool fileSent = postData(_T("https://") _T(BRAND_DOMAIN) _T("/postlogfile.php"), std::make_pair("appId", appId()), std::make_pair("logfile", rotateLogfile()));
                        trayNotify(fileSent ? IDC_LOGFILES_SENT : IDC_LOGFILES_NOTSENT);
                    }
                    break;
                    case ID_TRAYMENU_NEW_LOG: {
                        rotateLogfile();
                        trayNotify(IDC_LOGFILES_NOTSENT);
                    }
                    break;
                    case ID_TRAYMENU_EDIT_LOGFILE: {
                        auto oldLogfile = rotateLogfile();
                        const std::wstring oldLogfileName = oldLogfile.wstring();
                        const std::wstring logfilesDir = oldLogfile.parent_path().wstring();
                        ShellExecuteW(hWnd, L"open", oldLogfileName.c_str(), NULL, logfilesDir.c_str(), SW_RESTORE);
                    }
                    break;
                    case ID_TRAYMENU_OPEN_LOGFILES_DIR: {
                        const std::wstring logfilesDir = Logger::instance().logFilename().parent_path().wstring();
                        ShellExecuteW(hWnd, L"open", logfilesDir.c_str(), NULL, NULL, SW_RESTORE);
                    }
                    break;
                    case ID_TRAYMENU_MYPROFILE: {
                        const std::wstring myProfileUrl = std::wstring(_T("https://") _T(BRAND_DOMAIN) _T("/myprofile.php?appId=")) + appId();
                        ShellExecuteW(hWnd, L"open", myProfileUrl.c_str(), NULL, NULL, SW_RESTORE);
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
        MessageBoxA(hWnd, ee.what(), BRAND_FULLNAME, MB_ICONERROR);
    }
    return 0;
}
