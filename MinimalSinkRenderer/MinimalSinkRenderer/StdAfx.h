//----------------------------------------------------------------------------------------------
// StdAfx.h
//----------------------------------------------------------------------------------------------
#ifndef STDAFX_H
#define STDAFX_H

#pragma once
#define WIN32_LEAN_AND_MEAN
#define STRICT

#pragma comment(lib, "mfplat")
#pragma comment(lib, "shlwapi")
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
#include <initguid.h>
#include <Shlwapi.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mferror.h>
#include <wmcodecdsp.h>
#include <dshow.h>
#include <ks.h>

//----------------------------------------------------------------------------------------------
// Microsoft DirectX SDK (June 2010)
#ifdef _DEBUG
#define D3D_DEBUG_INFO
#endif
#include <d3d9.h>
#include <Evr9.h>

//----------------------------------------------------------------------------------------------
// Common Project Files
#ifdef _DEBUG
  #define MF_USE_LOGGING 1
  //#define MF_USE_LOGREFCOUNT
  //#define MF_TRACE_SINK
  //#define MF_TRACE_STREAM
#else
  #define MF_USE_LOGGING 0
#endif

//----------------------------------------------------------------------------------------------
// Project Files
#include "..\Common\MFMacro.h"
#include "..\Common\MFTrace.h"
#include "..\Common\MFLogging.h"
#include "..\Common\MFTExternTrace.h"
#include "..\Common\MFClassFactory.h"
#include "..\Common\MFGuid.h"
#include "..\Common\MFRegistry.h"
#include "..\Common\MFLogMediaType.h"
#include "..\Common\MFCriticSection.h"
#include "..\Common\MFState.h"
#include "..\Common\MFTime.h"

//----------------------------------------------------------------------------------------------
// Project Files
class CMinimalSkinkRenderer;

#define TRACE_FRAME_NUMBER 1
#define BMP_IMAGE_FILE L"Image.bmp"

#include "StreamSkinkRenderer.h"
#include "MinimalSkinkRenderer.h"

#endif