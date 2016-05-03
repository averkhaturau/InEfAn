#include "events-logging.h"

#include "input-hooker/input-hooker.h"
#include "active-app-tracker/ActiveWindowTracker.h"
#include "active-app-tracker/WindowInfo.h"
#include "language-change-listener.h"
#include "keycode-deanonimizer.h"

#include <future>
#include <atomic>

// helper function to print mouse position from the input event
inline std::string mousePosToString(InputDeviceEvent const& ie)
{
    std::string mousePosStr;
    try {
        if (ie.eventType() == WM_MOUSEMOVE) {
            const POINT& mousePos = dynamic_cast<MouseAnyEvent const&>(ie).mousePos();
            mousePosStr = std::string("(") + std::to_string(mousePos.x) + ", " + std::to_string(mousePos.y) + ")";
        }
    } catch(std::bad_cast const&) {/*skip*/}
    return mousePosStr;
}

Logger::LogRecord logEvent(InputDeviceEvent const& ie)
{
    try {
        return Logger::instance() << std::setw(9) << ie.inputDevice() << " " << ie.description() << mousePosToString(ie);
    } catch (std::exception& ee) {
        return Logger::instance() << ee.what();
    } catch (...) {
        return Logger::instance() << "Unknown exception raised";
    }
}


void initEventsListening()
{
    static LanguageChangeListener lnHooker;
    lnHooker.setCallback([](std::wstring const & lang) {
        Logger::instance() << "Language changed to " << lang;
    });

    static KeycodeDeanonimizer kcDeanonimizer;

    // Start listen to input devices
    InputHooker::instance().setHooks(
    [](WPARAM wparam, KBDLLHOOKSTRUCT kbstruct) {
        std::future<void> parallelSections[] = {
            std::async(std::launch::async, [&]() {EventPreanalyser<KeyboardEvent>(KeyboardEvent(wparam, kbstruct).deanonimized(kcDeanonimizer.shouldDeanonimize()))(); }),
            std::async(std::launch::async, [&]() {if (LanguageChangeListener::langChangePossible(wparam, kbstruct)) { std::this_thread::yield(); lnHooker.checkChange(); }}),
            std::async(std::launch::async, [&]() {kcDeanonimizer.updateState(wparam, kbstruct); }),
        };
    },
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
                lnHooker.checkChange();
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
