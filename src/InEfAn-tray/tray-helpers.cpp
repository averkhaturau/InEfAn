#include "tray-helpers.h"

#include "Version.inl"
#include "resource-App.h"

#include <stdlib.h>
#include <tchar.h>

NOTIFYICONDATAW traydata = {
    NOTIFYICONDATA_V2_SIZE,                                // cbSize for WinXP compatibility
    NULL,                                                  // hWnd
    0,                                                     // uID
    NIF_ICON | NIF_TIP | NIF_MESSAGE,                      // uFlags
    TRAY_ICON_MESSAGE,                                     // uCallbackMessage
    NULL,                          // hIcon
    _T(VER_SZ_PRODUCTNAME),                                // szTip
    0,                                                     // dwState
    0,                                                     // dwStateMask
    {},                                                    // szInfo
    0,                                                     // uTimeout
    _T(VER_SZ_PRODUCTNAME),                                // szInfoTitle
    NIIF_INFO                                              // dwInfoFlags
    // must omit them for XP compatibility
    // {},                                                 // GUID
    // LoadIconW(hInstance, MAKEINTRESOURCE(IDI_MAINICON)) // hBalloonIcon
};
UINT traydata_id = 0;


void trayNotify(UINT resStr)
{
    traydata_id = resStr;
    LoadStringW(GetModuleHandle(NULL), resStr, traydata.szInfo, _countof(traydata.szInfo));
    traydata.uFlags |= NIF_INFO;
    Shell_NotifyIconW(NIM_MODIFY, &traydata);
}


void trayIconUpdate(UINT resIco, UINT resStr)
{
    HICON icon = LoadIconW(GetModuleHandle(NULL), MAKEINTRESOURCE(resIco));
    traydata.hIcon = icon;
    const int displ = _countof(_T(VER_SZ_PRODUCTNAME));
    if (LoadStringW(GetModuleHandleW(NULL), resStr, traydata.szTip + displ, _countof(traydata.szTip) - displ)) {
        traydata.szTip[displ - 1] = L'\n';
    }

    traydata.uFlags = NIF_TIP | NIF_ICON;
    Shell_NotifyIconW(NIM_MODIFY, &traydata);
}

std::wstring loadStdWStringFromRC(int resId)
{
    wchar_t text[200] = {};
    LoadStringW(GetModuleHandleW(NULL), resId, text, _countof(text));
    return text;
}

