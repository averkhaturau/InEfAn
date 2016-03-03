#pragma once

#include <Windows.h>
#include <functional>
#include <stdexcept>
#include <string>
#include <iostream>
#include "input-hooker-dll.h"


// Please create this class in dll only
class InputHooker
{
public:
    void startHook()
    {
        stopHook();

        std::cout << "starting hook" << std::endl;

        hookDll = LoadLibraryA(dllName);
        if (!hookDll)
            throw std::runtime_error(std::string("Error loading ") + dllName + "; error " + std::to_string(GetLastError()));

        std::cout << "DLL loaded: " << hookDll << std::endl;

        keyboardCallback = GetProcAddress<HOOKPROC>(hookDll, "keyboardCallback");
        mouseCallback = GetProcAddress<HOOKPROC>(hookDll, "mouseCallback");
        setCallbacks_t* setCallbacks = (setCallbacks_t*)GetProcAddress(hookDll, "setCallbacks");

        std::cout << "ProcAddr acquired: " << keyboardCallback << ", " << mouseCallback << ", " << setCallbacks << std::endl;


        if (!keyboardCallback || !mouseCallback || !setCallbacks)
            throw std::runtime_error(std::string("Unable to find DLL methods in ") + dllName + " error: " + std::to_string(GetLastError()));

        keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboardCallback, hookDll, 0);
        mouseHook    = SetWindowsHookEx(WH_MOUSE_LL,    mouseCallback, hookDll, 0);

        if (!(keyboardHook && mouseHook))
            throw std::runtime_error(std::string("SetWindowsHookEx filed w/e: ") + std::to_string(GetLastError()));

        (*setCallbacks)(onKeypressed, onMouse, keyboardHook, mouseHook);

        std::cout << "Hooks set: " << keyboardHook << ", " << mouseHook  << std::endl;
    }
    void stopHook()
    {
        if(keyboardHook) {
            UnhookWindowsHookEx(keyboardHook);
            keyboardHook = NULL;
        }
        if(mouseHook) {
            UnhookWindowsHookEx(mouseHook);
            mouseHook = NULL;
        }
        if (hookDll) {
            FreeLibrary(hookDll);
            hookDll = NULL;
        }
    }

    ~InputHooker() {stopHook();}

    void setHooks(Keypressed_t keypressFn, Mouse_t mouseFn)
    {
        onKeypressed = keypressFn;
        onMouse = mouseFn;
    }

    static InputHooker& instance()
    {
        static InputHooker _this;
        return _this;
    }

private:
    InputHooker() {}

    HOOKPROC keyboardCallback = NULL;
    HOOKPROC mouseCallback = NULL;

    HHOOK keyboardHook = NULL;
    HHOOK mouseHook = NULL;

    HINSTANCE hookDll = NULL;

    Keypressed_t onKeypressed;
    Mouse_t onMouse;

    static const char* InputHooker::dllName;
};
