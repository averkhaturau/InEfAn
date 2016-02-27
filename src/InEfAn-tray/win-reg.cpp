#include "win-reg.h"

RegistryHelper::RegistryHelper(HKEY key, LPTSTR keyName)
    : key_(key), keyName_(keyName)
{}

std::wstring RegistryHelper::readValue(const LPTSTR valueName) const
{
    ATL::CRegKey regKey;
    unsigned long valSize = 0;
    std::wstring value;
    if (!(
            ERROR_SUCCESS == regKey.Open(key_, keyName_, KEY_READ) &&
            ERROR_SUCCESS == regKey.QueryStringValue(valueName, NULL, &valSize) && valSize > 0 &&
            (value.resize(valSize), ERROR_SUCCESS == regKey.QueryStringValue(valueName, (LPTSTR)value.data(), &valSize))
        ))
        value.clear();
    else
        value.resize(valSize - 1);
    return value;
}

bool RegistryHelper::writeValue(const LPTSTR valueName, std::wstring const& value) const
{
    ATL::CRegKey regKey;

    return
        (ERROR_SUCCESS == regKey.Open(key_, keyName_, KEY_WRITE) ||
         ERROR_SUCCESS == regKey.Create(key_, keyName_, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_WRITE)) &&
        ERROR_SUCCESS == regKey.SetStringValue(valueName, value.c_str());
}
