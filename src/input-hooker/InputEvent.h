#pragma once

#include <Windows.h>
#include "keycodes.h"

class InputDeviceEvent
{
public:
    virtual DWORD time()const = 0;
    virtual std::string description()const = 0;
    virtual std::string inputDevice()const = 0;
};


class MouseAnyEvent : public InputDeviceEvent
{
public:
	MouseAnyEvent(WPARAM wp, MSLLHOOKSTRUCT const& me) : wparam(wp) { memcpy(&eventData, &me, sizeof(me)); }
    virtual DWORD time()const override { return eventData.time; };
    virtual std::string description()const override
    {
        switch (wparam) {
            case WM_LBUTTONDOWN: return "left down"; break;
            case WM_LBUTTONUP:   return "left up"; break;
            case WM_MOUSEMOVE:   return "moved"; break;
            case WM_MOUSEWHEEL:  return "wheeled"; break;
            case WM_MOUSEHWHEEL: return "h-wheeled"; break;
            case WM_RBUTTONDOWN: return "right down"; break;
            case WM_RBUTTONUP:   return "right up"; break;
            default:             return "undefined action"; break;
        }
    };
    virtual std::string inputDevice()const override { return "mouse"; };

private:
    WPARAM wparam;
    MSLLHOOKSTRUCT eventData;
};

class KeyboardEvent : public InputDeviceEvent
{
public:
    KeyboardEvent(WPARAM wp, KBDLLHOOKSTRUCT const& me) : wparam(wp) { memcpy(&eventData, &me, sizeof(me)); }
    virtual DWORD time()const override { return eventData.time; };
    virtual std::string description()const override { return std::string(wparam == WM_KEYDOWN || wparam == WM_SYSKEYDOWN ? "down " : "up ") + vKeyCodes[eventData.vkCode % sizeof(vKeyCodes)]; };
    virtual std::string inputDevice()const override { return "keyboard"; };

private:
    WPARAM wparam;
    KBDLLHOOKSTRUCT eventData;
};


// alternative structure
class InputEvent
{
    /*std::chrono::system_clock::time_point*/ DWORD time_;
public:
    InputEvent() = delete;
    InputEvent(InputEvent&&) = delete;
    InputEvent(InputEvent const& ie) : time_(ie.time_) {}
    explicit InputEvent(DWORD t) : time_(t) {}

    DWORD time()const { return time_; }
};

class KeyEvent : public InputEvent
{
    DWORD vkCode_;
public:
    explicit KeyEvent(KBDLLHOOKSTRUCT const& eventData) : InputEvent(eventData.time), vkCode_(eventData.vkCode) {}
    KeyEvent(KeyEvent const& ke) : InputEvent(ke), vkCode_(ke.vkCode_) {}
    KeyEvent() = delete;
    KeyEvent(KeyEvent&&) = delete;

    DWORD keyCode()const { return vkCode_; }
};

class KeyDownEvent : public KeyEvent
{
public:
    KeyDownEvent(KeyDownEvent const& kde) : KeyEvent(kde) {}
    explicit KeyDownEvent(KBDLLHOOKSTRUCT const& eventData) : KeyEvent(eventData) {}
    KeyDownEvent(KeyDownEvent&&) = delete;
    KeyDownEvent() = delete;
};

class KeyUpEvent : public KeyEvent
{
public:
    KeyUpEvent(KeyUpEvent const& kde) : KeyEvent(kde) {}
    explicit KeyUpEvent(KBDLLHOOKSTRUCT const& eventData) : KeyEvent(eventData) {}
    KeyUpEvent(KeyUpEvent&&) = delete;
    KeyUpEvent() = delete;
};

class MouseEvent : public InputEvent
{
    long x_, y_;
public:
    MouseEvent(MouseEvent const& me) : InputEvent(me), x_(me.x_), y_(me.y_) {}
    explicit MouseEvent(MSLLHOOKSTRUCT const& eventData) : InputEvent(eventData.time), x_(eventData.pt.x), y_(eventData.pt.y) {}
    MouseEvent(MouseEvent&&) = delete;
    MouseEvent() = delete;

	long x()const{ return x_; }
	long y()const{ return y_; }
};

class MouseMoveEvent : public MouseEvent
{
public:
    MouseMoveEvent() = delete;
    MouseMoveEvent(MouseMoveEvent&&) = delete;
    MouseMoveEvent(MouseMoveEvent const& e) : MouseEvent(e) {}
    explicit MouseMoveEvent(MSLLHOOKSTRUCT const& eventData) : MouseEvent(eventData) {}
};

class MouseWheelEvent : public MouseEvent
{
public:
    MouseWheelEvent() = delete;
    MouseWheelEvent(MouseWheelEvent&&) = delete;
    MouseWheelEvent(MouseWheelEvent const& e) : MouseEvent(e) {}
    explicit MouseWheelEvent(MSLLHOOKSTRUCT const& eventData) : MouseEvent(eventData) {}
};

class MouseHorWheelEvent : public MouseEvent
{
public:
    MouseHorWheelEvent() = delete;
    MouseHorWheelEvent(MouseHorWheelEvent&&) = delete;
    MouseHorWheelEvent(MouseHorWheelEvent const& e) : MouseEvent(e) {}
    explicit MouseHorWheelEvent(MSLLHOOKSTRUCT const& eventData) : MouseEvent(eventData) {}
};

class MouseLeftUpEvent : public MouseEvent
{
public:
    MouseLeftUpEvent() = delete;
    MouseLeftUpEvent(MouseLeftUpEvent&&) = delete;
    MouseLeftUpEvent(MouseLeftUpEvent const& e) : MouseEvent(e) {}
    explicit MouseLeftUpEvent(MSLLHOOKSTRUCT const& eventData) : MouseEvent(eventData) {}
};

class MouseLeftDownEvent : public MouseEvent
{
public:
    MouseLeftDownEvent() = delete;
    MouseLeftDownEvent(MouseLeftDownEvent&&) = delete;
    MouseLeftDownEvent(MouseLeftDownEvent const& e) : MouseEvent(e) {}
    explicit MouseLeftDownEvent(MSLLHOOKSTRUCT const& eventData) : MouseEvent(eventData) {}
};

class MouseRightUpEvent : public MouseEvent
{
public:
    MouseRightUpEvent() = delete;
    MouseRightUpEvent(MouseRightUpEvent&&) = delete;
    MouseRightUpEvent(MouseRightUpEvent const& e) : MouseEvent(e) {}
    explicit MouseRightUpEvent(MSLLHOOKSTRUCT const& eventData) : MouseEvent(eventData) {}
};

class MouseRightDownEvent : public MouseEvent
{
public:
    MouseRightDownEvent() = delete;
    MouseRightDownEvent(MouseRightDownEvent&&) = delete;
    MouseRightDownEvent(MouseRightDownEvent const& e) : MouseEvent(e) {}
    explicit MouseRightDownEvent(MSLLHOOKSTRUCT const& eventData) : MouseEvent(eventData) {}
};

