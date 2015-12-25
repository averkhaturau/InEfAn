#include "tray-helpers.h"

#include "Version.inl"

#include "single-instance.h"
#include "Version.inl"
#include "tray-helpers.h"

#include <windows.h>
#include <filesystem>
#include "../logger/logger.h"
#include <tchar.h>

#include "../input-hooker/input-hooker.h"
#include "../input-hooker/InputEvent.h"
#include "../active-app-tracker/ActiveWindowTracker.h"
#include "../active-app-tracker/WindowInfo.h"


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

void logEvent(InputDeviceEvent const& ie)
{
    try {
        Logger::instance() << "\t" << ie.inputDevice() << "\t" << ie.description();
    } catch (std::exception& ee) {
        Logger::instance() << ee.what();
    } catch (...) {
        Logger::instance() << "Unknown exception raised";
    }
};

template<class Ev_t>
class EventPreanalyser
{
    EventPreanalyser() = delete;
    EventPreanalyser(EventPreanalyser const&) = delete;
    EventPreanalyser(EventPreanalyser&&) = delete;
public:
    explicit EventPreanalyser(Ev_t&& ev) : ie(ev) {}
    void operator()() { logEvent(ie); }
private:
    Ev_t ie;
};

void onAppStart()
{
    // Start listen to input devices
    InputHooker::instance().setHooks(
    [](WPARAM wparam, KBDLLHOOKSTRUCT kbsrtuct) {EventPreanalyser<KeyboardEvent>(KeyboardEvent(wparam, kbsrtuct))(); },
    [](WPARAM wparam, MSLLHOOKSTRUCT mstruct) {EventPreanalyser<MouseAnyEvent>(MouseAnyEvent(wparam, mstruct))(); });
    InputHooker::instance().startHook();

    // Start tracking foreground window
    static ActiveWindowTracker awt;
    awt.setCallback([](HWND hwnd) {
        try {
            if (!hwnd)
                Logger::instance() << L"No foreground window detected";
            else {
                WindowInfo wi(hwnd);
                Logger::instance() << L"Foreground window title is \"" << wi.getTitle()
                                   << L"\" from process name \"" << wi.getProcessName()
                                   << L"\" running from file \"" << wi.getProcessFilename()
                                   << L"\"";
            }
        } catch (std::exception& ee) {
            Logger::instance() << ee.what();
        } catch (...) {
            Logger::instance() << "Unknown exception raised";
        }
    });
}

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
        onAppStart();

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
                        break;
                    case ID_TRAYMENU_RESUME:
                        Logger::instance().enable(true);
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
        MessageBoxA(hWnd, ee.what(), BRAND_FULLNAME, MB_ICONERROR);
    }
    return 0;
}
