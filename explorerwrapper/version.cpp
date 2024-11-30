#include "version.h"

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