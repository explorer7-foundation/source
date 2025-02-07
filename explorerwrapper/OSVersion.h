#pragma once
#include "common.h"

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
	ULONG BuildRevision();
};

extern COSVersion g_osVersion;