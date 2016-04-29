#pragma once

#include <string>
#include <functional>
#include <Windows.h>


class LanguageChangeListener
{
public:
    void checkChange();

    static bool langChangePossible(WPARAM wparam, KBDLLHOOKSTRUCT kbsrtuct);

    void setCallback(std::function<void(std::wstring const&)> fn) { callback = fn; };
private:
    std::wstring lang;
    std::function<void(std::wstring const&)> callback;

    std::wstring getForegroundWindowInputLanguage()const;
};
