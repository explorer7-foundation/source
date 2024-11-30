#pragma once
#include "framework.h"
#include "startmenupin.h"
#include "userassist.h"

typedef struct { 
	LPITEMIDLIST pidlParent;	//+0	//+0
	LPITEMIDLIST pidlRelative;	//+4	//+8
	LPWSTR pszAppID;	//+8	//+10
	UEMINFO ueminfo;	//+Ch	//+18
	int iPinPos;		//+30h	//+3C
	DWORD_PTR fNewApp;	//+34h	//+44
} STARTMENUITEM, *PSTARTMENUITEM;	//38h	//48h

MIDL_INTERFACE("00000000-0000-0000-0000-000000000000")
IEnumStartMenuItem: public IUnknown
{
public:
	STDMETHOD(Next)( ULONG celt, PSTARTMENUITEM rgelt, ULONG * pceltFetched ) PURE;
	STDMETHOD(Skip)( ULONG celt ) PURE;
	STDMETHOD(Reset)(void) PURE;
	STDMETHOD(Clone)(IEnumStartMenuItem**) PURE;
};

class CEnumStartMenu : public IEnumStartMenuItem
{
public:
	CEnumStartMenu();
	~CEnumStartMenu();

	//IUnknown
    STDMETHODIMP QueryInterface(REFIID riid,void **ppvObject);    
	STDMETHODIMP_(ULONG) AddRef( void);
	STDMETHODIMP_(ULONG) Release( void);

	//IEnumUnknown
	STDMETHODIMP Clone(IEnumStartMenuItem **ppenum);
	STDMETHODIMP Next(ULONG celt,PSTARTMENUITEM rgelt,ULONG *pceltFetched);
	STDMETHODIMP Reset();
	STDMETHODIMP Skip(ULONG celt);

	//our methods
	STDMETHODIMP_(void) AddItem(PSTARTMENUITEM item);
	STDMETHODIMP_(void) Sort();
	STDMETHODIMP_(void) RemoveDuplicates();
	STDMETHODIMP_(void) SetLimit(long limit);
private:
	long m_cRef;
	long m_count;
	long m_enumidx;
	long m_limit;
	HDSA hArrItems;
};