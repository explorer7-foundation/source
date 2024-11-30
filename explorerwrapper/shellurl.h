#pragma once
#include "framework.h"

DEFINE_GUID(GUID_88df9332_6adb_4604_8218_508673ef7f8a, 0x88df9332, 0x6adb, 0x4604, 0x82, 0x18, 0x50, 0x86, 0x73, 0xef, 0x7f, 0x8a);
DEFINE_GUID(GUID_4f33718d_bae1_4f9b_96f2_d2a16e683346, 0x4f33718d, 0xbae1, 0x4f9b, 0x96, 0xf2, 0xd2, 0xa1, 0x6e, 0x68, 0x33, 0x46);

typedef unsigned short ushort;
typedef unsigned long ulong;
typedef unsigned long uint;
typedef unsigned char uchar;

MIDL_INTERFACE("4f33718d-bae1-4f9b-96f2-d2a16e683346")
IShellURL7 : public IUnknown
{
public:
	STDMETHOD(ParseFromOutsideSource)(ushort const*, ulong) = 0;
	STDMETHOD(GetUrl)(ushort*, ulong) = 0;
	STDMETHOD(SetUrl)(ushort const*, ulong) = 0;
	STDMETHOD(GetDisplayName)(ushort*, ulong) = 0;
	STDMETHOD(GetPidl)(LPITEMIDLIST*) = 0;
	STDMETHOD(SetPidl)(LPITEMIDLIST) = 0;
	STDMETHOD(SetPidlAndArgs)(LPITEMIDLIST, ushort const*) = 0;
	STDMETHOD(GetArgs)(void) = 0;
	STDMETHOD(AddPath)(LPITEMIDLIST) = 0;
	STDMETHOD(SetCancelObject)(void*) = 0;
	STDMETHOD(StartAsyncPathParse)(HWND__*, ushort const*, ulong, void*) = 0;
	STDMETHOD(GetParseResult)(void) = 0;
	STDMETHOD(SetUsnSource)(ulong) = 0;
	STDMETHOD(GetUsnSource)(ulong*) = 0;
	STDMETHOD(SetNavFlags)(int, int) = 0;
	STDMETHOD(GetCookie)(ulong*) = 0;
	STDMETHOD(Execute)(void*, int*, ulong) = 0;
	STDMETHOD(SetCurrentWorkingDir)(LPITEMIDLIST) = 0;
	STDMETHOD(SetMessageBoxParent)(HWND__*) = 0;
	STDMETHOD(GetPidlNoGenerate)(LPITEMIDLIST*) = 0;
	STDMETHOD(GetStandardParsingFlags)(int) = 0;
};

MIDL_INTERFACE("88df9332-6adb-4604-8218-508673ef7f8a")
IShellURL10 : public IUnknown
{
public:
	STDMETHOD(ParseFromOutsideSource)(ushort const*, ulong) = 0;
	STDMETHOD(GetUrl)(ushort*, ulong) = 0;
	STDMETHOD(SetUrl)(ushort const*, ulong) = 0;
	STDMETHOD(GetDisplayName)(ushort*, ulong) = 0;
	STDMETHOD(GetPidl)(LPITEMIDLIST*) = 0;
	STDMETHOD(SetPidl)(LPITEMIDLIST) = 0;
	STDMETHOD(SetPidlAndArgs)(LPITEMIDLIST, ushort const*) = 0;
	STDMETHOD(GetArgs)(void) = 0;
	STDMETHOD(AddPath)(LPITEMIDLIST) = 0;
	STDMETHOD(SetCancelObject)(void*) = 0;
	STDMETHOD(StartAsyncPathParse)(HWND__*, ushort const*, ulong, void*) = 0;
	STDMETHOD(GetParseResult)(void) = 0;
	STDMETHOD(SetRequestID)(int) = 0;
	STDMETHOD(GetRequestID)(int*) = 0;
	STDMETHOD(SetNavFlags)(int, int) = 0;
	STDMETHOD(GetCookie)(ulong*) = 0;
	STDMETHOD(Execute)(void*, int*, ulong) = 0;
	STDMETHOD(SetCurrentWorkingDir)(LPITEMIDLIST) = 0;
	STDMETHOD(SetMessageBoxParent)(HWND__*) = 0;
	STDMETHOD(GetPidlNoGenerate)(LPITEMIDLIST*) = 0;
	STDMETHOD(GetStandardParsingFlags)(int) = 0;
	STDMETHOD(GetUrlAlloc)(ushort**) = 0;
	STDMETHOD(GetDisplayNameAlloc)(ushort**) = 0;
};

class CShellURLWrapper : public IShellURL7
{
public:
	CShellURLWrapper(IShellURL10* actual);

	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);
	STDMETHODIMP ParseFromOutsideSource(ushort const*, ulong);
	STDMETHODIMP GetUrl(ushort*, ulong);
	STDMETHODIMP SetUrl(ushort const*, ulong);
	STDMETHODIMP GetDisplayName(ushort*, ulong);
	STDMETHODIMP GetPidl(LPITEMIDLIST*);
	STDMETHODIMP SetPidl(LPITEMIDLIST);
	STDMETHODIMP SetPidlAndArgs(LPITEMIDLIST, ushort const*);
	STDMETHODIMP GetArgs(void);
	STDMETHODIMP AddPath(LPITEMIDLIST);
	STDMETHODIMP SetCancelObject(void*);
	STDMETHODIMP StartAsyncPathParse(HWND__*, ushort const*, ulong, void*);
	STDMETHODIMP GetParseResult(void);
	STDMETHODIMP SetUsnSource(ulong);
	STDMETHODIMP GetUsnSource(ulong*);
	STDMETHODIMP SetNavFlags(int, int);
	STDMETHODIMP GetCookie(ulong*);
	STDMETHODIMP Execute(void*, int*, ulong);
	STDMETHODIMP SetCurrentWorkingDir(LPITEMIDLIST);
	STDMETHODIMP SetMessageBoxParent(HWND__*);
	STDMETHODIMP GetPidlNoGenerate(LPITEMIDLIST*);
	STDMETHODIMP GetStandardParsingFlags(int);

	ULONG m_cRef;
	IShellURL10* m_actual;
};