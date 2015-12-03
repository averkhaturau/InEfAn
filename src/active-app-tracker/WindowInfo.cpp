#include "WindowInfo.h"

#include <Psapi.h>
#include <functional>

// helper function
inline std::wstring readFromFuncAndResizeIfNeeded(std::function<size_t(std::wstring&)>&& fn)
{
    std::wstring result(MAX_PATH, wchar_t());
    size_t writtenLength = 0;
    while ((writtenLength = fn(result)) == result.size())
        result.resize(result.size() * 2);
    result.resize(writtenLength);
    return result;
}

template<class Fn_t>
inline std::wstring readProcessSomething(HWND hwnd, Fn_t fn){
    DWORD procID = 0;
    DWORD threadId = GetWindowThreadProcessId(hwnd, &procID);
    HANDLE procHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);
    if (!procHandle)
        throw std::runtime_error("Cannot OpenProcess with error: " + std::to_string(GetLastError()));

    return readFromFuncAndResizeIfNeeded([&](std::wstring& s){return fn(procHandle, NULL, (LPWSTR)s.data(), s.size()); });
}


std::wstring WindowInfo::getTitle() const
{
    return readFromFuncAndResizeIfNeeded([this](std::wstring& s){return static_cast<size_t>(GetWindowTextW(myHWND, (LPWSTR)s.data(), static_cast<int>(s.size())));});
}

std::wstring WindowInfo::getProcessName() const
{
    return readProcessSomething(myHWND, &GetModuleBaseNameW);
}

std::wstring WindowInfo::getProcessFilename() const
{
    return readProcessSomething(myHWND, &GetModuleFileNameExW);
}
