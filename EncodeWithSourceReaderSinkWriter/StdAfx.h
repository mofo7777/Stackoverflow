//----------------------------------------------------------------------------------------------
// StdAfx.h
//----------------------------------------------------------------------------------------------
#ifndef STDAFX_H
#define STDAFX_H

#pragma once
#define WIN32_LEAN_AND_MEAN
#define STRICT

#pragma comment(lib, "mf")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid")
#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "strmiids")

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
#include <initguid.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <evr.h>
#include <uuids.h>
#include <d3d9.h>
#include <Evr9.h>
#include <dxva.h>
#include <dxvahd.h>
#include <wmcodecdsp.h>
#include <wmsdkidl.h>
#include <ks.h>
#include <Ksmedia.h>

//----------------------------------------------------------------------------------------------
// Common Project Files
#ifdef _DEBUG
#define MF_USE_LOGGING 1
#else
#define MF_USE_LOGGING 0
#endif

#include "MFMacro.h"
#include "MFTrace.h"
#include "MFLogging.h"
#include "MFTExternTrace.h"
#include "MFGuid.h"
#include "MFLogCommon.h"
#include "MFLogMediaType.h"

#endif