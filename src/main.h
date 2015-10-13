// Error Lookup
// Copyright (c) 2011-2015 Henry++

#ifndef __MAIN_H__
#define __MAIN_H__

#include <windows.h>
#include "resource.h"

// config
#define APP_NAME L"Error Lookup"
#define APP_NAME_SHORT L"errorlookup"
#define APP_VERSION L"2.1.143"
#define APP_VERSION_RES 2,1,143,0
#define APP_AUTHOR L"Henry++"
#define APP_COPYRIGHT L"(c) 2011-2015 " APP_AUTHOR L". All rights reserved."

#define APP_WEBSITE L"http://www.henrypp.org"
#define APP_GITHUB L"https://github.com/henrypp"

#ifdef _WIN64
#define APP_MACHINE L"64"
#else
#define APP_MACHINE L"32"
#endif // _WIN64

#define APP_SETTINGS_COUNT 3

// libs
#pragma comment(lib,"dxerr.lib")

// sev.
#define STATUS_SEVERITY_SUCCESS 0x0
#define STATUS_SEVERITY_INFORMATIONAL 0x1
#define STATUS_SEVERITY_WARNING 0x2
#define STATUS_SEVERITY_ERROR 0x3

// fac.
#define FACILITY_CODCLASS_ERROR_CODE 0x6
#define FACILITY_FILTER_MANAGER 0x1C
#define FACILITY_VIDEO 0x1B
#define FACILITY_MONITOR 0x1D
#define FACILITY_GRAPHICS_KERNEL 0x1E
#define FACILITY_WIN32K_NTUSER 0x3E
#define FACILITY_WIN32K_NTGDI 0x3F
#define FACILITY_RESUME_KEY_FILTER 0x40
#define FACILITY_RDBSS 0x41
#define FACILITY_BTH_ATT 0x42
#define FACILITY_SECUREBOOT 0x43
#define FACILITY_AUDIO_KERNEL 0x44
#define FACILITY_VOLSNAP 0x50
#define FACILITY_SHARED_VHDX 0X5C
#define FACILITY_INTERIX 0x99
#define FACILITY_SPACES 0xE7
#define FACILITY_MAXIMUM_VALUE 0xE8
#define FACILITY_EAP_MESSAGE 0x0842
#define FACILITY_D3D 0x876
#define FACILITY_D3DX 0x877
#define FACILITY_DSOUND_DMUSIC 0x878
#define FACILITY_D3D10 0x879
#define FACILITY_D3D11_OR_AE 0x87C
#define FACILITY_XAUDIO2 0x896
#define FACILITY_XAPO 0x897
#define FACILITY_DWRITE 0x898
#define FACILITY_APO 0X87D
#define FACILITY_WSAPI 0x889
#define FACILITY_XACTENGINE 0xAC7
#define FACILITY_D2D 0x899

// routine
#define SWITCH_ROUTINE(x)  case x: buffer.Format(L"%s (0x%02x)\0", L#x, x); break;

#endif // __MAIN_H__
