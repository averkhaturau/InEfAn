#pragma once

#include "Version.inl"
#include "resource-App.h"

#include <Windows.h>
#include <stdlib.h>
#include <string>
#include <shellapi.h>

extern NOTIFYICONDATAW traydata;
extern UINT traydata_id;

//void trayNotify(UINT resStr);
template<class ... T>
void trayNotify(UINT resStr, T... args)
{
    traydata_id = resStr;
    std::wstring fmt(MAX_PATH, wchar_t());
    LoadStringW(GetModuleHandleW(NULL), resStr, &*fmt.begin(), static_cast<int>(fmt.size()));
    if (sizeof...(args) != 0)
        swprintf_s(traydata.szInfo, _countof(traydata.szInfo), fmt.c_str(), args...);
    traydata.uFlags |= NIF_INFO;
    Shell_NotifyIconW(NIM_MODIFY, &traydata);
}
void trayNotify(UINT resStr);
void trayIconUpdate(UINT resIco, UINT resStr);
std::wstring loadStdWStringFromRC(int resId);

