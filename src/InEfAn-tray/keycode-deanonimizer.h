#pragma once

#include <atomic>
#include <Windows.h>

class KeycodeDeanonimizer
{
public:
    bool shouldDeanonimize() {return isShortCut;}

    void updateState(WPARAM wparam, KBDLLHOOKSTRUCT kbsrtuct)
    {
        if (kbsrtuct.vkCode == VK_CONTROL || kbsrtuct.vkCode == VK_RCONTROL || kbsrtuct.vkCode == VK_LCONTROL ||
            kbsrtuct.vkCode == VK_MENU    || kbsrtuct.vkCode == VK_RMENU    || kbsrtuct.vkCode == VK_LMENU ||
            kbsrtuct.vkCode == VK_RWIN || kbsrtuct.vkCode == VK_LWIN)

            isShortCut = wparam != WM_KEYUP && wparam != WM_SYSKEYUP;
    }
private:
    std::atomic<bool> isShortCut = false;
};
