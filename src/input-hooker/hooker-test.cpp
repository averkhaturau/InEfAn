#include "input-hooker.h"
#include <iostream>

#include "EventLogger.h"



int main()
{
    try {
        EventLogger el(std::cout);

        InputHooker::instance().setHooks(
        [&el](WPARAM wparam, KBDLLHOOKSTRUCT kbsrtuct) {
            el.log(KeyboardEvent(wparam, kbsrtuct));
            //std::cout << vKeyCodes[kbsrtuct.vkCode] << (wparam == WM_KEYDOWN || wparam == WM_SYSKEYDOWN ? " down" : " up") << " at " << kbsrtuct.time << "\n";
        },
        [&el](WPARAM wparam, MSLLHOOKSTRUCT mstruct) {
            el.log(MouseAnyEvent(wparam, mstruct));
            //std::cout << "Mouse ";
            //switch (wparam){
            //case WM_LBUTTONDOWN: std::cout << "left down"; break;
            //case WM_LBUTTONUP:   std::cout << "left up"; break;
            //case WM_MOUSEMOVE:   std::cout << "moved"; break;
            //case WM_MOUSEWHEEL:  std::cout << "wheeled"; break;
            //case WM_MOUSEHWHEEL: std::cout << "h-wheeled"; break;
            //case WM_RBUTTONDOWN: std::cout << "right down"; break;
            //case WM_RBUTTONUP:   std::cout << "right up"; break;
            //default:             std::cout << "undefined action"; break;
            //}
            //std::cout << " at " << mstruct.time << "\n";
        }
        );
        InputHooker::instance().startHook();

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
        }

    } catch (std::exception& ee) {
        std::cerr << "Exception thrown: " << ee.what();
    } catch (...) {
        std::cerr << "Exited due to unknown echeption";
    }
    return 0;
}
