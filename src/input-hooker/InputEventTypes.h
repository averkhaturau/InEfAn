#pragma once

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

    long x()const { return x_; }
    long y()const { return y_; }
};

class MouseMoveEvent : public MouseEvent
{
public:
    MouseMoveEvent() = delete;
    MouseMoveEvent(MouseMoveEvent&&) = delete;
    MouseMoveEvent(MouseMoveEvent const& e) : MouseEvent(e) {}
    explicit MouseMoveEvent(MSLLHOOKSTRUCT const& eventData) : MouseEvent(eventData) {}
};

class MouseMoveStartEvent : public MouseMoveEvent
{
public:
    MouseMoveStartEvent() = delete;
    MouseMoveStartEvent(MouseMoveStartEvent&&) = delete;
    MouseMoveStartEvent(MouseMoveStartEvent const& e) : MouseMoveEvent(e) {}
    explicit MouseMoveStartEvent(MSLLHOOKSTRUCT const& eventData) : MouseMoveEvent(eventData) {}
};

class MouseMoveFinishEvent : public MouseMoveEvent
{
public:
    MouseMoveFinishEvent() = delete;
    MouseMoveFinishEvent(MouseMoveFinishEvent&&) = delete;
    MouseMoveFinishEvent(MouseMoveFinishEvent const& e) : MouseMoveEvent(e) {}
    explicit MouseMoveFinishEvent(MSLLHOOKSTRUCT const& eventData) : MouseMoveEvent(eventData) {}
};

class MouseWheelEvent : public MouseEvent
{
public:
    MouseWheelEvent() = delete;
    MouseWheelEvent(MouseWheelEvent&&) = delete;
    MouseWheelEvent(MouseWheelEvent const& e) : MouseEvent(e) {}
    explicit MouseWheelEvent(MSLLHOOKSTRUCT const& eventData) : MouseEvent(eventData) {}
};

class MouseWheelStartEvent : public MouseWheelEvent
{
public:
    MouseWheelStartEvent() = delete;
    MouseWheelStartEvent(MouseWheelStartEvent&&) = delete;
    MouseWheelStartEvent(MouseWheelStartEvent const& e) : MouseWheelEvent(e) {}
    explicit MouseWheelStartEvent(MSLLHOOKSTRUCT const& eventData) : MouseWheelEvent(eventData) {}
};

class MouseWheelFinishEvent : public MouseWheelEvent
{
public:
    MouseWheelFinishEvent() = delete;
    MouseWheelFinishEvent(MouseWheelFinishEvent&&) = delete;
    MouseWheelFinishEvent(MouseWheelFinishEvent const& e) : MouseWheelEvent(e) {}
    explicit MouseWheelFinishEvent(MSLLHOOKSTRUCT const& eventData) : MouseWheelEvent(eventData) {}
};

class MouseHorWheelEvent : public MouseEvent
{
public:
    MouseHorWheelEvent() = delete;
    MouseHorWheelEvent(MouseHorWheelEvent&&) = delete;
    MouseHorWheelEvent(MouseHorWheelEvent const& e) : MouseEvent(e) {}
    explicit MouseHorWheelEvent(MSLLHOOKSTRUCT const& eventData) : MouseEvent(eventData) {}
};

class MouseHorWheelStartEvent : public MouseHorWheelEvent
{
public:
    MouseHorWheelStartEvent() = delete;
    MouseHorWheelStartEvent(MouseHorWheelStartEvent&&) = delete;
    MouseHorWheelStartEvent(MouseHorWheelStartEvent const& e) : MouseHorWheelEvent(e) {}
    explicit MouseHorWheelStartEvent(MSLLHOOKSTRUCT const& eventData) : MouseHorWheelEvent(eventData) {}
};

class MouseHorWheelFinishEvent : public MouseHorWheelEvent
{
public:
    MouseHorWheelFinishEvent() = delete;
    MouseHorWheelFinishEvent(MouseHorWheelFinishEvent&&) = delete;
    MouseHorWheelFinishEvent(MouseHorWheelFinishEvent const& e) : MouseHorWheelEvent(e) {}
    explicit MouseHorWheelFinishEvent(MSLLHOOKSTRUCT const& eventData) : MouseHorWheelEvent(eventData) {}
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
