// Error Lookup
// Copyright (c) 2011-2018 Henry++

#ifndef __MAIN_H__
#define __MAIN_H__

#include <windows.h>
#include <commctrl.h>

#include "resource.hpp"
#include "app.hpp"

#define MODULES_MENU 4
#define LANG_MENU 5

// pugixml document configuration
#define PUGIXML_LOAD_FLAGS (pugi::parse_escapes)
#define PUGIXML_LOAD_ENCODING pugi::encoding_auto

#define SECTION_MODULE L"module"

#define FORMAT_DEC L"%lu"
#define FORMAT_HEX L"0x%.8lx"

struct ITEM_MODULE
{
	HMODULE hlib = nullptr;

	LPWSTR text = nullptr;

	WCHAR path[MAX_PATH] = {0};
	WCHAR description[MAX_PATH] = {0};
};

#endif // __MAIN_H__
