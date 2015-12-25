#include "log-helpers.h"

#include <string>
#include <sstream>
#include <locale>
#include <codecvt>

std::string toUtf8(std::wstring arg)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
    return convert.to_bytes(arg);
}

std::string toUtf8(const wchar_t* arg)
{
    return toUtf8(std::wstring(arg));
}

std::string toUtf8(wchar_t* arg)
{
    return toUtf8(std::wstring(arg));
}

using std::chrono::system_clock;

std::string timestamp(system_clock::time_point const& time)
{
    const time_t localunixtime = system_clock::to_time_t(time);
    char buff[25];
#pragma warning(push)
#pragma warning(disable:4996)
    strftime(buff, sizeof(buff) - 4, "%Y-%m-%d %H:%M:%S.", localtime(&localunixtime));
    sprintf_s(&buff[20], 5, "%03u ", time.time_since_epoch() % 1000);
#pragma warning(pop)
    return buff;
}