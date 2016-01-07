#pragma once

#include <Windows.h>
#include "keycodes.h"
#include "InputEventTypes.h"

class InputDeviceEvent
{
public:
    virtual DWORD time()const = 0;
    virtual std::string description()const = 0;
    virtual std::string inputDevice()const = 0;

    virtual WPARAM eventType()const { return wparam; }
    virtual bool isRepeatable()const = 0;
protected:
    WPARAM wparam;
};


class MouseAnyEvent : public InputDeviceEvent
{
public:
    MouseAnyEvent(WPARAM wp, MSLLHOOKSTRUCT const& me) { wparam = wp; memcpy(&eventData, &me, sizeof(me)); }
    MouseAnyEvent(MouseAnyEvent const& a) { wparam = a.wparam; memcpy(&eventData, &a.eventData, sizeof(eventData)); }
    MouseAnyEvent& operator =(MouseAnyEvent const& a) { wparam = a.wparam; memcpy(&eventData, &a.eventData, sizeof(eventData)); return *this; }
    virtual DWORD time()const override { return eventData.time; };
    virtual std::string description()const override
    {
        switch (wparam) {
            case WM_LBUTTONDOWN:   return "left down"; break;
            case WM_LBUTTONUP:     return "left up"; break;
            case WM_LBUTTONDBLCLK: return "left dblclick"; break;
            case WM_MOUSEMOVE:     return "move"; break;
            case WM_MOUSEWHEEL:    return "wheel"; break;
            case WM_MOUSEHWHEEL:   return "h-wheel"; break;
            case WM_RBUTTONDOWN:   return "right down"; break;
            case WM_RBUTTONUP:     return "right up"; break;
            case WM_RBUTTONDBLCLK: return "right dblclick"; break;
            case WM_MBUTTONDOWN:   return "middle down"; break;
            case WM_MBUTTONUP:     return "middle up"; break;
            case WM_MBUTTONDBLCLK: return "middle dblclick"; break;
            case WM_XBUTTONDOWN:   return "X down"; break;
            case WM_XBUTTONUP:     return "X up"; break;
            case WM_XBUTTONDBLCLK: return "X dblclick"; break;
            default:               return "undefined action"; break;
        }
    };
    virtual std::string inputDevice()const override { return "mouse"; };

    bool isRepeatable()const override { return wparam == WM_MOUSEMOVE || wparam == WM_MOUSEWHEEL || wparam == WM_MOUSEHWHEEL; }

private:
    MSLLHOOKSTRUCT eventData;
};

class KeyboardEvent : public InputDeviceEvent
{
public:
    KeyboardEvent(WPARAM wp, KBDLLHOOKSTRUCT const& me) { wparam = wp; memcpy(&eventData, &me, sizeof(me)); }
    virtual DWORD time()const override { return eventData.time; };
    virtual std::string description()const override { return std::string(wparam == WM_KEYDOWN || wparam == WM_SYSKEYDOWN ? "down " : "up ") + vKeyCodes[eventData.vkCode % sizeof(vKeyCodes)]; };
    virtual std::string inputDevice()const override { return "keyboard"; };

    bool isRepeatable()const override { return false; }
private:
    KBDLLHOOKSTRUCT eventData;
};
