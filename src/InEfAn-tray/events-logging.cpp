#include "events-logging.h"

#include "input-hooker/input-hooker.h"
#include "active-app-tracker/ActiveWindowTracker.h"
#include "active-app-tracker/WindowInfo.h"


Logger::LogRecord logEvent(InputDeviceEvent const& ie)
{
    try {
        return Logger::instance() << std::setw(9) << ie.inputDevice() << " " << ie.description();
    } catch (std::exception& ee) {
        return Logger::instance() << ee.what();
    } catch (...) {
        return Logger::instance() << "Unknown exception raised";
    }
}

void initEventsListening()
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

void EventPreanalyser<MouseAnyEvent>::operator()()
{
    if (ie.isRepeatable()) {
        const bool isTheSameEvent = ie.eventType() == lastEvent.eventType();

        if (!isTheSameEvent) {
            if (lastEvent.eventType() != 0)
                logEvent(lastEvent) << " finished";
            logEvent(ie) << " started";
        }

        if (timerHandle)
            KillTimer(NULL, timerHandle);

        lastEvent = ie;

        timerHandle = SetTimer(NULL, 0, 200/*ms*/, static_cast<TIMERPROC>([](HWND, UINT, UINT_PTR, DWORD) {
            KillTimer(NULL, timerHandle);
            logEvent(lastEvent) << " finished";
            lastEvent = MouseAnyEvent(0, {});
        }));


    } else
        logEvent(ie);
}

MouseAnyEvent EventPreanalyser<MouseAnyEvent>::lastEvent(0, {});
UINT_PTR EventPreanalyser<MouseAnyEvent>::timerHandle(0);