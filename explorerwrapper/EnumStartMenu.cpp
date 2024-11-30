#include "EnumStartMenu.h"
#include "dbgprint.h"

int CALLBACK CleanStartMenuItem(void *p,void *pData)
{
	PSTARTMENUITEM item = (PSTARTMENUITEM)p;
	ILFree(item->pidlRelative);
	CoTaskMemFree(item->pszAppID);
	return 1;
}

CEnumStartMenu::CEnumStartMenu()
{
	m_cRef = 1;
	m_enumidx = 0;
	m_count = 0;
	m_limit = 1000;
	hArrItems = DSA_Create(sizeof(STARTMENUITEM),4);	
	dbgprintf(L"CEnumStartMenu::CEnumStartMenu\n");
}

CEnumStartMenu::~CEnumStartMenu()
{
	DSA_DestroyCallback(hArrItems,CleanStartMenuItem,NULL);
}

HRESULT STDMETHODCALLTYPE CEnumStartMenu::QueryInterface(REFIID riid,void **ppvObject)
{
	dbgprintf(L"CEnumStartMenu::QueryInterface NOT IMPLEMENTED\n");
	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CEnumStartMenu::AddRef(void)
{
	return InterlockedIncrement(&m_cRef);
}

ULONG STDMETHODCALLTYPE CEnumStartMenu::Release(void)
{
	if (InterlockedDecrement(&m_cRef) == 0)
	{
		delete this;
		return 0;
	}
	return m_cRef;
}

HRESULT STDMETHODCALLTYPE CEnumStartMenu::Clone(IEnumStartMenuItem **ppenum)
{
	dbgprintf(L"CEnumStartMenu::Clone\n");
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CEnumStartMenu::Next(ULONG celt,PSTARTMENUITEM rgelt,ULONG *pceltFetched)
{
	if ( celt != 1 )
		return E_NOTIMPL;
	if ( m_enumidx >= m_count || m_enumidx >= m_limit )
	{
		if (pceltFetched) *pceltFetched = 0;
		return S_FALSE;
	}
	DSA_GetItem(hArrItems,m_enumidx,rgelt);
	dbgprintf(L"Pos %d R %X FT %X_%X Item %s",rgelt->iPinPos,rgelt->ueminfo.R,
		rgelt->ueminfo.ftExecute.dwHighDateTime,rgelt->ueminfo.ftExecute.dwLowDateTime,rgelt->pszAppID);
	m_enumidx++;
	if (pceltFetched) *pceltFetched = 1;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CEnumStartMenu::Reset()
{
	dbgprintf(L"CEnumStartMenu::Reset\n");
	m_enumidx = 0;
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CEnumStartMenu::Skip(ULONG celt)
{
	dbgprintf(L"CEnumStartMenu::Skip\n");
	return E_NOTIMPL;
}

void STDMETHODCALLTYPE CEnumStartMenu::AddItem(PSTARTMENUITEM item)
{
	DSA_InsertItem(hArrItems,DSA_APPEND,item);
	m_count++;
}

int inline PosForCompare(int pos)
{
	if (pos == -2) return 1000;
	if (pos == -1) return 2000;
	return pos;
}

int CALLBACK CompareItems(void *p1,void *p2,LPARAM lParam)
{
	PSTARTMENUITEM item1 = (PSTARTMENUITEM)p1;
	PSTARTMENUITEM item2 = (PSTARTMENUITEM)p2;
	int pinpos1 = PosForCompare(item1->iPinPos);
	int pinpos2 = PosForCompare(item2->iPinPos);
	int compare = pinpos1 - pinpos2; //pin pos - ascending
	if (compare == 0)
		compare = item2->ueminfo.R - item1->ueminfo.R; //R - descending
	if (compare == 0)
		compare = CompareFileTime(&item2->ueminfo.ftExecute,&item1->ueminfo.ftExecute); //filetime - descending
	return compare;
}

void STDMETHODCALLTYPE CEnumStartMenu::Sort()
{
	DSA_Sort(hArrItems,CompareItems,0);
}

void STDMETHODCALLTYPE CEnumStartMenu::RemoveDuplicates()
{
	int i1 = 1;	
	PSTARTMENUITEM item1;
	PSTARTMENUITEM item2;
	while ( (i1<m_limit) && (item1 = (PSTARTMENUITEM)DSA_GetItemPtr(hArrItems,i1)) )
	{		
		int i2 = i1+1;
		BOOL duplicate = FALSE;
		if (item1->iPinPos == -1)
		{
			while ( (i2<m_limit) && (item2 = (PSTARTMENUITEM)DSA_GetItemPtr(hArrItems,i2)) )
			{
				if (lstrcmp(item1->pszAppID,item2->pszAppID) == 0)
				{
					duplicate = TRUE;
					break;					
				}
				i2++;
			}
		}
		if (duplicate)
		{
			CleanStartMenuItem(item2,NULL);
			DSA_DeleteItem(hArrItems,i2);
			m_count--;
		}
		else
		i1++;
	}
}

void STDMETHODCALLTYPE CEnumStartMenu::SetLimit(long limit)
{
	m_limit = limit;
}