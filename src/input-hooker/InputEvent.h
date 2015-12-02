#pragma once

#include <Windows.h>
#include "keycodes.h"

class InputEvent
{
public:
	virtual DWORD time()const=0;
	virtual std::string description()const=0;
	virtual std::string inputDevice()const=0;
};


class MouseEvent: public InputEvent
{
public:
	MouseEvent(WPARAM wp, MSLLHOOKSTRUCT const& me):wparam(wp){memcpy(&eventData, &me, sizeof(me));}
	virtual DWORD time()const override{ return eventData.time; };
	virtual std::string description()const override{
			switch (wparam){
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
	virtual std::string inputDevice()const override{ return "mouse"; };

private:
	WPARAM wparam;
	MSLLHOOKSTRUCT eventData;
};

class KeyboardEvent : public InputEvent
{
public:
	KeyboardEvent(WPARAM wp, KBDLLHOOKSTRUCT const& me) :wparam(wp){ memcpy(&eventData, &me, sizeof(me)); }
	virtual DWORD time()const override { return eventData.time; };
	virtual std::string description()const override{ return std::string(wparam == WM_KEYDOWN || wparam == WM_SYSKEYDOWN ? "down " : "up ") + vKeyCodes[eventData.vkCode % sizeof(vKeyCodes)]; };
	virtual std::string inputDevice()const override{ return "keyboard"; };

private:
	WPARAM wparam;
	KBDLLHOOKSTRUCT eventData;
};