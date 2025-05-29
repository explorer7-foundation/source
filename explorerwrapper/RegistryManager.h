#pragma once
#include "common.h"
#include <Shlwapi.h>

class CRegistryManager
{
private:
	HKEY m_hKeyMachine;
	HKEY m_hKeyUser;
	void _OpenKeys();

public:
	CRegistryManager();

	LSTATUS QueryValue(LPCWSTR lpValueName, LPBYTE lpData, DWORD cbData, LPDWORD lpType = nullptr);
	HRESULT QueryValueWithFallback(LPCWSTR lpValueName, LPBYTE lpData, DWORD cbData, LPDWORD lpType = nullptr, DWORD dwDefault = 0);
	LSTATUS SetValue(LPCWSTR lpValueName, DWORD dwType, LPCBYTE lpData, DWORD cbData);
};

static CRegistryManager g_registry;

const LPWSTR c_szSubkey = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced";
