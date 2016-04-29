#include "language-change-listener.h"

void LanguageChangeListener::checkChange()
{
    std::wstring currentLang = getForegroundWindowInputLanguage();

    if (callback && lang != currentLang) {
        lang = currentLang;
        callback(lang);
    }
}

bool LanguageChangeListener::langChangePossible(WPARAM wparam, KBDLLHOOKSTRUCT kbsrtuct)
{
    return (wparam == WM_KEYUP || wparam == WM_SYSKEYUP) &&
           kbsrtuct.vkCode == VK_CONTROL || kbsrtuct.vkCode == VK_MENU || (0xA0 <= kbsrtuct.vkCode && kbsrtuct.vkCode <= 0xA5);
}

std::wstring LanguageChangeListener::getForegroundWindowInputLanguage() const
{
    GUITHREADINFO gti = {/*.cbSize = */sizeof(GUITHREADINFO) };
    BOOL res = GetGUIThreadInfo(0, &gti);
    DWORD dwThread = GetWindowThreadProcessId(gti.hwndActive, 0);
    HKL keylayout = GetKeyboardLayout(dwThread);
    WCHAR currentLang[KL_NAMELENGTH + 1] = {};
    int length = GetLocaleInfoW(MAKELCID((UINT)keylayout & 0xffffffff, SORT_DEFAULT), LOCALE_SISO639LANGNAME, currentLang, _countof(currentLang));
    return currentLang;
}
