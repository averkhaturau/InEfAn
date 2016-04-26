#pragma once

#include "logger.h"
#include "input-hooker/input-event.h"

Logger::LogRecord logEvent(InputDeviceEvent const& ie);


template<class Ev_t>
class EventPreanalyser
{
    EventPreanalyser() = delete;
    EventPreanalyser(EventPreanalyser const&) = delete;
    EventPreanalyser(EventPreanalyser&&) = delete;
public:
    explicit EventPreanalyser(Ev_t const& ev) : ie(ev) {}
    void operator()()
    {
        logEvent(ie);
    }
private:
    Ev_t ie;
};

template<>
class EventPreanalyser<MouseAnyEvent>
{
    EventPreanalyser() = delete;
    EventPreanalyser(EventPreanalyser const&) = delete;
    EventPreanalyser(EventPreanalyser&&) = delete;
public:
    explicit EventPreanalyser(MouseAnyEvent&& ev) : ie(ev) {}
    void operator()();
private:
    MouseAnyEvent ie;
    static MouseAnyEvent lastEvent;
    static UINT_PTR timerHandle;
};

void initEventsListening();



