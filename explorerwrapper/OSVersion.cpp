#include "OSVersion.h"

typedef NTSTATUS (NTAPI *RtlGetVersion_t)(PRTL_OSVERSIONINFOEXW);

void COSVersion::_FillVersionInfo()
{
	HMODULE hNtDll = GetModuleHandleW(L"ntdll.dll");
	RtlGetVersion_t RtlGetVersion = (RtlGetVersion_t)GetProcAddress(hNtDll, "RtlGetVersion");
	if (RtlGetVersion)
	{
		RtlGetVersion(&m_osvi);
	}
}

COSVersion::COSVersion()
{
	ZeroMemory(&m_osvi, sizeof(RTL_OSVERSIONINFOEXW));
	m_osvi.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);
}

ULONG COSVersion::BuildNumber()
{
	if (!m_osvi.dwBuildNumber)
		_FillVersionInfo();
	return m_osvi.dwBuildNumber;
}

ULONG COSVersion::MajorVersion()
{
	if (!m_osvi.dwMajorVersion)
		_FillVersionInfo();
	return m_osvi.dwMajorVersion;
}

ULONG COSVersion::MinorVersion()
{
	if (!m_osvi.dwMinorVersion)
		_FillVersionInfo();
	return m_osvi.dwMinorVersion;
}

// https://stackoverflow.com/questions/47926094/detecting-windows-10-os-build-minor-version
ULONG COSVersion::BuildRevision()
{
    DWORD ubr = 0, ubr_size = sizeof(DWORD);
    HKEY hKey;
    LONG lRes = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        L"Software\\Microsoft\\Windows NT\\CurrentVersion",
        0,
        KEY_READ,
        &hKey
    );
    if (lRes == ERROR_SUCCESS)
    {
        RegQueryValueExW(
            hKey,
            UNIFIEDBUILDREVISION_VALUE,
            0,
            NULL,
            (LPBYTE)&ubr,
            (LPDWORD)&ubr_size
        );
    }
    return ubr;
}