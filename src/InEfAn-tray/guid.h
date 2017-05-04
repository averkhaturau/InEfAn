#pragma once

#include <windows.h>
#include <rpc.h>
#include <ObjBase.h>


GUID generateGuid()
{
    GUID guid = {};
    if (CoCreateGuid(&guid) != S_OK)
        throw std::runtime_error("CoCreateGuid failed");
    return guid;
}

std::wstring guidToString(GUID const& guid)
{
    RPC_WSTR str = 0;
    if (UuidToStringW(&guid, &str) !=  S_OK)
        throw std::runtime_error("UuidToString failed");
    std::wstring result(reinterpret_cast<wchar_t*>(str));
    RpcStringFreeW(&str);
    return result;
}

