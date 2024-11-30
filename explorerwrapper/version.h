#pragma once
#include "framework.h"

class COSVersion
{
private:
	RTL_OSVERSIONINFOEXW m_osvi;
	void _FillVersionInfo();

public:
	COSVersion();
	ULONG BuildNumber();
	ULONG MajorVersion();
	ULONG MinorVersion();
};

extern COSVersion g_osVersion;