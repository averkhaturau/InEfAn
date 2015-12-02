#include "WindowInfo.h"

#include <Psapi.h>

std::wstring WindowInfo::getTitle() const
{
    std::wstring title(MAX_PATH, wchar_t());
    int titleLength = 0;
    while (titleLength = GetWindowTextW(myHWND, (LPWSTR)title.data(), title.size()) == title.size())
        title.resize(title.size());
    return title;
}

std::wstring WindowInfo::getProcessName() const
{
    DWORD procID = 0;
    GetWindowThreadProcessId(myHWND, &procID);
    HANDLE procHandle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, procID);

    std::wstring procFilename(MAX_PATH, wchar_t());
    unsigned int nameLength = 0;
    while (procFilename.size() == (nameLength = GetModuleBaseNameW(procHandle, NULL, (LPWSTR)procFilename.data(), procFilename.size())))
        procFilename.resize(procFilename.size() * 2);

    procFilename.resize(nameLength - 1);
    return procFilename;
}

std::wstring WindowInfo::getProcessFilename() const
{
    DWORD procID = 0;
    GetWindowThreadProcessId(myHWND, &procID);
    HANDLE procHandle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, procID);

    std::wstring procFilename(MAX_PATH, wchar_t());
    unsigned int nameLength = 0;
    while (procFilename.size() == (nameLength = GetModuleFileNameExW(procHandle, NULL, (LPWSTR)procFilename.data(), procFilename.size())))
        procFilename.resize(procFilename.size() * 2);

    procFilename.resize(nameLength - 1);
    return procFilename;
}
