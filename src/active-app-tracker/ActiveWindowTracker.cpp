#include "ActiveWindowTracker.h"

ActiveWindowTracker::ActiveWindowTracker()
{
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

ActiveWindowTracker* ActiveWindowTracker::_this = nullptr;

void ActiveWindowTracker::checkChanges()
{
    const HWND currentFW = GetForegroundWindow();
    if (activeHWND != currentFW)
    {
        activeHWND = currentFW;
        if (callback)
            callback(currentFW);
    }
}

void CALLBACK ActiveWindowTracker::timerProc(HWND, UINT, UINT_PTR, DWORD dwTime)
{
    if (_this)
        _this->checkChanges();
}
