#pragma once

#include "guid.h"
#include "win-reg.h"

std::wstring appId()
{
    const RegistryHelper reg(HKEY_CURRENT_USER, _T("Software\\") _T(BRAND_COMPANYNAME) _T("\\") _T(BRAND_NAME));
    LPTSTR valueName = _T("instanceId");

    auto appId = reg.readValue(valueName);
    if (appId.empty()) {
        appId = guidToString(generateGuid());
        reg.writeValue(valueName, appId);
    }

    return appId;
}