#pragma once

#include <string>
#include <atlbase.h>

struct INetFwProfile;

class WindowsFirewall
{
public:
    enum FirewallStatus {
        DOESNT_EXIST = -2,
        BLOCKED = -1,
        UNKNOWN = 0,
        ALLOWED = 1
    };
    WindowsFirewall();
    bool isEnabled();
    FirewallStatus getFirewallStatus(std::wstring const& imageName);
    // returns true if succeeded
    bool addApplicationPolicy(std::wstring const& imageName, std::wstring const& applicationName);
private:
    CComPtr<INetFwProfile> fwProfile;
};

void allowFirewallForMe();