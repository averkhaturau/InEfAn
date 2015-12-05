#pragma once

#include <Windows.h>
#include <string>

class WindowInfo
{
public:
    WindowInfo(HWND hwnd): myHWND(hwnd) {}

    std::wstring getTitle()const;
    std::wstring getProcessName()const;
    std::wstring getProcessFilename()const;
private:
    HWND myHWND;
};
