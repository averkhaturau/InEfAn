#pragma once

#include <functional>
#include <Windows.h>

typedef std::function<void(WPARAM, KBDLLHOOKSTRUCT)> Keypressed_t;
typedef std::function<void(WPARAM, MSLLHOOKSTRUCT)> Mouse_t;

typedef void _cdecl setCallbacks_t(Keypressed_t& k, Mouse_t& m, HHOOK kh, HHOOK mh);
