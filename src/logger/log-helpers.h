#pragma once

#include <chrono>
#include <string>
#include <xutility>
#include <iomanip>

template<class Arg_t> inline std::string toUtf8(Arg_t s) { static_assert(false, "Cannot instantiate toUtf8 for the type"); return std::string(); };

template<> inline std::string toUtf8(std::string s) { return s; }
inline std::string toUtf8(const char* s) { return std::string(s); };
//template<unsigned N> inline std::string toUtf8(const char(&s)[N]) { return std::string(s); }

// next are custom typed to convert to utf8
std::string toUtf8(std::wstring arg);
std::string toUtf8(const wchar_t* arg);
std::string toUtf8(wchar_t* arg);
//template<unsigned N> inline std::string toUtf8(const wchar_t(&arg)[N]) { return toUtf8(std::wstring(arg)); }

template<class Arg_t> inline std::string toUtf8(Arg_t* arg)
{
    char buff[20];
#pragma warning(push)
#pragma warning(disable:4996)
    // warning C4996 : 'itoa' : The POSIX name for this item is deprecated.Instead, use the ISO C++ conformant name : _itoa.See online help for details.
    return std::string("0x") + itoa(reinterpret_cast<int>(arg), buff, 16);
#pragma warning(pop)
}
template<class Arg_t> inline std::_Smanip<Arg_t> toUtf8(std::_Smanip<Arg_t> arg) { return arg; }

//template<class Arg_t> inline std::enable_if_t<std::is_integral<Arg_t>::value, std::string> toUtf8(Arg_t arg)
//{
//    return std::to_string(arg);
//}
template<> inline std::string toUtf8(int arg) { return std::to_string(arg); }
template<> inline std::string toUtf8(unsigned int arg) { return std::to_string(arg); }
template<> inline std::string toUtf8(unsigned long long arg) { return std::to_string(arg); }
template<> inline std::string toUtf8(unsigned short arg) { return std::to_string(arg); }
template<> inline std::string toUtf8(float arg) { return std::to_string(arg); }
template<> inline std::string toUtf8(bool arg) { return arg ? "true" : "false"; }

std::string timestamp(std::chrono::system_clock::time_point const&);
std::string timestamp_filename(std::chrono::system_clock::time_point const& time = std::chrono::system_clock::now());

