BOOL WINAPI ChangeImportedAddress_FARPROC( HMODULE hModule, LPSTR modulename, FARPROC origfunc, FARPROC newfunc )
{
	DWORD_PTR lpFileBase = (DWORD_PTR)hModule;	
	if ( !lpFileBase || !modulename || !origfunc || !newfunc ) return FALSE;	
    PIMAGE_DOS_HEADER dosHeader;
    PIMAGE_NT_HEADERS pNTHeader;
    PIMAGE_IMPORT_DESCRIPTOR pImportDir;
    DWORD_PTR* pFunctions;    
    LPSTR name;
    DWORD oldpr;
    
    
    dosHeader = (PIMAGE_DOS_HEADER)lpFileBase;
    pNTHeader = (PIMAGE_NT_HEADERS)( lpFileBase + dosHeader->e_lfanew );    
    pImportDir = (PIMAGE_IMPORT_DESCRIPTOR) ( lpFileBase + pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress );
    
    while ( pImportDir->Name )
    {		
		name = (LPSTR) (lpFileBase + pImportDir->Name);
		/*OutputDebugStringA(name);*/
		if ( lstrcmpiA( name, modulename ) == 0 ) break;
		pImportDir++;
    }
    if ( !pImportDir->Name ) return FALSE;
    
    pFunctions = (DWORD_PTR*)(lpFileBase + pImportDir->FirstThunk);
    
    while ( *pFunctions )
    {
		if ( *pFunctions == (DWORD_PTR)origfunc ) break;
		pFunctions++;
    }
    if ( !*pFunctions ) return FALSE;
    
	VirtualProtect( pFunctions, sizeof(DWORD_PTR), PAGE_EXECUTE_READWRITE, &oldpr );
	*pFunctions = (DWORD_PTR)newfunc;
	VirtualProtect( pFunctions, sizeof(DWORD_PTR), oldpr, &oldpr );
    return TRUE;
}

#define ChangeImportedAddress(hModule,modulename,origproc,newproc) ChangeImportedAddress_FARPROC(hModule,modulename,(FARPROC)origproc,(FARPROC)newproc)
