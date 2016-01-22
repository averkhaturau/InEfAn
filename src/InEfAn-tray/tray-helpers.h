#pragma once

#include "Version.inl"
#include "resource-App.h"

#include <Windows.h>
#include <stdlib.h>
#include <string>
#include <shellapi.h>

extern NOTIFYICONDATAW traydata;
extern UINT traydata_id;

void trayNotify(UINT resStr);
void trayIconUpdate(UINT resIco, UINT resStr);
std::wstring loadStdWStringFromRC(int resId);

