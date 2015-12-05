#pragma once

#include <Windows.h>
#include <functional>

class ActiveWindowTracker
{
public:
    ActiveWindowTracker();
    ~ActiveWindowTracker();

    typedef std::function<void(HWND)> OnWindowChanged_t;

    void setCallback(OnWindowChanged_t const& callback);
private:
    HWND activeHWND = NULL;
    std::wstring lastWindowTitle;
    OnWindowChanged_t callback;
    UINT_PTR timerHandle;
    static ActiveWindowTracker* _this; // to pass to timer proc

    // should be called on timer
    void checkChanges();

    static void CALLBACK timerProc(HWND, UINT, UINT_PTR, DWORD dwTime);

};
