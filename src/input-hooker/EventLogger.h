#pragma once

#include "InputEvent.h"
#include <fstream>

class EventLogger
{
public:
    EventLogger(std::ostream& out): outstream(out) {}
    void log(InputEvent const& ie)
    {
        outstream << ie.time() << "\t" << ie.inputDevice() << "\t" << ie.description() << "\n";
    }
private:
    std::ostream& outstream;
};
