#pragma once
#include "common.h"

MIDL_INTERFACE("00000000-0000-0000-0000-000000000000")
IAppResolverNotify7: public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE RegisterNotification(unsigned int,long,PVOID*,int) = 0;
	virtual HRESULT STDMETHODCALLTYPE UnregisterNotification(unsigned int) = 0;
	virtual HRESULT STDMETHODCALLTYPE AddHotkey(unsigned int, PVOID*, PVOID*, int) = 0;
};

MIDL_INTERFACE("00000000-0000-0000-0000-000000000000")
IAppResolverNotify8: public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE OnChangeNotify(long,PVOID*,PVOID*) = 0;
	virtual HRESULT STDMETHODCALLTYPE AddHotkey(unsigned int, PVOID*, PVOID*, int) = 0;
};

class CAppResolverNotify8: public IAppResolverNotify8
{
public:
	//constructor
	CAppResolverNotify8(IAppResolverNotify7 *oldresolver);
	//destructor
	~CAppResolverNotify8();
	//IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,void **ppvObject);    
    ULONG STDMETHODCALLTYPE AddRef( void);    
    ULONG STDMETHODCALLTYPE Release( void);
	//IAppResolverNotify8
    HRESULT STDMETHODCALLTYPE OnChangeNotify(long,PVOID*,PVOID*);
	HRESULT STDMETHODCALLTYPE AddHotkey(unsigned int, PVOID*, PVOID*, int);
private:
	IAppResolverNotify7 *m_resolver7;
	long m_cRef;
};
