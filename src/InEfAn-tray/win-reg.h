#pragma once

#include <atlbase.h>
#include <string>

class RegistryHelper
{
public:
    explicit RegistryHelper(HKEY key, LPTSTR keyName);

    std::wstring readValue(const LPTSTR valueName) const;

    bool writeValue(const LPTSTR valueName, std::wstring const& value) const;

private:
    HKEY key_;
    LPTSTR keyName_;
};

