#pragma once
#include "framework.h"

class CRegistryManager
{
private:
	HKEY m_hKeyMachine;
	HKEY m_hKeyUser;
	void _OpenKeys();

public:
	CRegistryManager();

	LSTATUS QueryValue(LPCWSTR lpValueName, LPBYTE lpData, DWORD cbData, LPDWORD lpType = nullptr);
	LSTATUS SetValue(LPCWSTR lpValueName, DWORD dwType, LPCBYTE lpData, DWORD cbData);
};

static CRegistryManager g_registry;