//----------------------------------------------------------------------------------------------
// StdAfx.h
//----------------------------------------------------------------------------------------------
#ifndef STDAFX_H
#define STDAFX_H

#pragma once
#define WIN32_LEAN_AND_MEAN
#define STRICT

//----------------------------------------------------------------------------------------------
// Pragma
#pragma comment(lib, "mfplat")
#pragma comment(lib, "shlwapi")
#pragma comment(lib, "dxva2")
#pragma comment(lib, "d3d9")
#pragma comment(lib, "Mf")
#pragma comment(lib, "strmiids")
#pragma comment(lib, "mfuuid")

//----------------------------------------------------------------------------------------------
// Microsoft Windows SDK for Windows 7
#include <WinSDKVer.h>
#include <new>
#include <windows.h>
#include <tchar.h>
#include <assert.h>
#include <strsafe.h>
#ifdef _DEBUG
#include <crtdbg.h>
#endif
#include <commdlg.h>
#include <CommCtrl.h>
#include <Shellapi.h>
#include <initguid.h>
#include <Shlwapi.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mferror.h>
#include <evr.h>
#include <uuids.h>
#include <d3d9.h>
#include <Evr9.h>
#include <dxva.h>
#include <dxvahd.h>
#include <thread>
#include <cguid.h>
#include <atlbase.h>
#include <mutex>

//----------------------------------------------------------------------------------------------
// Common Project Files
#ifdef _DEBUG
#define MF_USE_LOGGING 1
//#define MF_USE_LOGREFCOUNT
//#define TRACE_PLAYER
#else
#define MF_USE_LOGGING 0
#endif

#include ".\Common\MFMacro.h"
#include ".\Common\MFTrace.h"
#include ".\Common\MFLogging.h"
#include ".\Common\MFTExternTrace.h"

//----------------------------------------------------------------------------------------------
// Project Files
#include "Player.h"

#endif