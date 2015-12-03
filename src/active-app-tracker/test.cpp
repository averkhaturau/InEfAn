#include "WindowInfo.h"
#include "ActiveWindowTracker.h"
#include <iostream>

int main()
{
    SetConsoleOutputCP(65001);

    ActiveWindowTracker awt;
    awt.setCallback([](HWND hwnd){
        try
        {
            if (!hwnd)
                std::wcout << L"No foreground window detected\n";
            else
            {
                WindowInfo wi(hwnd);
                std::wcout << L"Foreground window title is \"" << wi.getTitle()
                    << L"\" from process name \"" << wi.getProcessName()
                    << L"\" running from file \"" << wi.getProcessFilename()
                    << L"\"\n";
            }
        } catch (std::exception& ee)
        {
            std::cerr << ee.what() << "\n";
        } catch (...)
        {
            std::cerr << "Unknown exception raised\n";
        }
    });

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        DispatchMessage(&msg);
    }
    return 0;
}
