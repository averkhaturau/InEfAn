#include "WindowInfo.h"
#include "ActiveWindowTracker.h"
#include <iostream>

int main(){
	ActiveWindowTracker awt;
	awt.setCallback([](HWND hwnd){
		if(!hwnd)
			std::wcout << L"No foreground window detected\n";
		else{
			WindowInfo wi(hwnd);
            std::cout << "Foreground window title is \"" //<< wi.getTitle()
				<< "\" from process name \"" //<< wi.getProcessName()
				<< "\" running from file \"" //<< wi.getProcessFilename()
                << "\"\n";
		}
	});

	MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)){DispatchMessage(&msg);}
	return 0;
}
