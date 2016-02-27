#include "win-firewall.h"

#include <windows.h>
#include <netfw.h>
#include <crtdbg.h>
#include <comip.h>
#include <comdef.h>
#include <assert.h>
#include <atlbase.h>

WindowsFirewall::WindowsFirewall()
{
    _COM_SMARTPTR_TYPEDEF(INetFwMgr, __uuidof(INetFwMgr));
    _COM_SMARTPTR_TYPEDEF(INetFwPolicy, __uuidof(INetFwPolicy));
    fwProfile = NULL;
    try {
        INetFwMgrPtr fwMgr(__uuidof(NetFwMgr), NULL, CLSCTX_INPROC_SERVER);
        INetFwPolicyPtr fwPolicy;

        _com_util::CheckError(fwMgr->get_LocalPolicy(&fwPolicy));
        // Retrieve the firewall profile currently in effect.
        _com_util::CheckError(fwPolicy->get_CurrentProfile(&fwProfile));
    } catch (_com_error&) {
    }
}

bool WindowsFirewall::isEnabled()
{
    try {
        VARIANT_BOOL fwEnabled;
        return fwProfile && (_com_util::CheckError(fwProfile->get_FirewallEnabled(&fwEnabled)), (fwEnabled != VARIANT_FALSE));
    } catch (_com_error&) {
    }

    return false;
}

WindowsFirewall::FirewallStatus WindowsFirewall::getFirewallStatus(std::wstring const& imageName)
{
    FirewallStatus result = UNKNOWN;

    if (!fwProfile) {
        return result;
    }

    _COM_SMARTPTR_TYPEDEF(INetFwAuthorizedApplication,  __uuidof(INetFwAuthorizedApplication));
    _COM_SMARTPTR_TYPEDEF(INetFwAuthorizedApplications, __uuidof(INetFwAuthorizedApplications));

    try {
        _bstr_t procImgFilename(imageName.c_str());
        INetFwAuthorizedApplicationsPtr fwApps;
        INetFwAuthorizedApplicationPtr fwApp;

        _com_util::CheckError(fwProfile->get_AuthorizedApplications(&fwApps));

        if (SUCCEEDED(fwApps->Item(procImgFilename, &fwApp))) {
            VARIANT_BOOL fwEnabled;
            _com_util::CheckError(fwApp->get_Enabled(&fwEnabled));

            result = (fwEnabled == VARIANT_FALSE ? BLOCKED : ALLOWED);
        } else {
            result = DOESNT_EXIST;
        }
    } catch (_com_error&) {
    }

    return result;
}

bool WindowsFirewall::addApplicationPolicy(std::wstring const& imageName, std::wstring const& applicationName)
{
    auto tryAddApp = [&]() {
        try {
            _COM_SMARTPTR_TYPEDEF(INetFwAuthorizedApplication,  __uuidof(INetFwAuthorizedApplication));
            _COM_SMARTPTR_TYPEDEF(INetFwAuthorizedApplications, __uuidof(INetFwAuthorizedApplications));

            // query authorized firewall applications
            INetFwAuthorizedApplicationsPtr fwApps;
            _com_util::CheckError(fwProfile->get_AuthorizedApplications(&fwApps));

            // add us to this list
            INetFwAuthorizedApplicationPtr fwApp(__uuidof(NetFwAuthorizedApplication), NULL, CLSCTX_INPROC_SERVER);
            _bstr_t imgNameStr(imageName.c_str());
            _com_util::CheckError(fwApp->put_ProcessImageFileName(imgNameStr));
            _bstr_t appNameStr(applicationName.c_str());
            _com_util::CheckError(fwApp->put_Name(appNameStr));
            _com_util::CheckError(fwApps->Add(fwApp));

            return true;
        } catch (_com_error&) {
        }
        // if we are here, we got an exception
        return false;
    };

    return fwProfile && (getFirewallStatus(imageName) == ALLOWED || tryAddApp());
}

void allowFirewallForMe()
{
    WindowsFirewall wfw;
    wchar_t path[MAX_PATH] = {};
    if (GetModuleFileNameW(NULL, path, _countof(path)) && wfw.isEnabled() && WindowsFirewall::ALLOWED != wfw.getFirewallStatus(path))
        wfw.addApplicationPolicy(path, _T(BRAND_FULLNAME));
}