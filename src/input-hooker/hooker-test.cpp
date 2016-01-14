#include "input-hooker.h"
#include <iostream>

#include "event-logger.h"



int main()
{
    try {
        EventLogger el(std::cout);

        InputHooker::instance().setHooks(
        [&el](WPARAM wparam, KBDLLHOOKSTRUCT kbsrtuct) {
            el.log(KeyboardEvent(wparam, kbsrtuct));
        },
        [&el](WPARAM wparam, MSLLHOOKSTRUCT mstruct) {
            el.log(MouseAnyEvent(wparam, mstruct));
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
