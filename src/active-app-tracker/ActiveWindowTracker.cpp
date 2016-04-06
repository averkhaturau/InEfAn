#include "ActiveWindowTracker.h"
#include "WindowInfo.h"

#include <assert.h>

ActiveWindowTracker* ActiveWindowTracker::_this = nullptr;

ActiveWindowTracker::ActiveWindowTracker()
{
    assert(!_this && "Allow only one instance of me");
    _this = this;
    timerHandle = SetTimer(NULL, 0, 1000/*1 second*/, timerProc);
}

ActiveWindowTracker::~ActiveWindowTracker()
{
    KillTimer(NULL, timerHandle);
    _this = nullptr;
}

void ActiveWindowTracker::setCallback(OnWindowChanged_t const& callback)
{
    this->callback = callback;
}

void ActiveWindowTracker::checkChanges()
{
    try {
        const HWND currentFW = GetForegroundWindow();
        // Window title is considered private info
        // const std::wstring currentTitle = WindowInfo(currentFW).getTitle();
        // track window changes and title changes
        if (activeHWND != currentFW /*|| lastWindowTitle != currentTitle*/) {
            activeHWND = currentFW;
            // lastWindowTitle = currentTitle;
            if (callback)
                callback(currentFW);
        }
    } catch (...) {/*too late to handle here*/}
}

void CALLBACK ActiveWindowTracker::timerProc(HWND, UINT, UINT_PTR, DWORD dwTime)
{
    if (_this)
        _this->checkChanges();
}
