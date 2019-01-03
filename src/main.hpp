// Error Lookup
// Copyright (c) 2011-2019 Henry++

#ifndef __MAIN_H__
#define __MAIN_H__

#include <windows.h>
#include <commctrl.h>

#include "resource.hpp"
#include "app.hpp"

#define MODULES_MENU 5
#define LANG_MENU 6

// pugixml document configuration
#define PUGIXML_LOAD_FLAGS (pugi::parse_escapes)
#define PUGIXML_LOAD_ENCODING pugi::encoding_auto

#define SECTION_MODULE L"module"

#define FORMAT_DEC L"%lu"
#define FORMAT_HEX L"0x%.8lx"

struct ITEM_MODULE
{
	WCHAR path[MAX_PATH] = {0};
	WCHAR description[MAX_PATH] = {0};

	LPWSTR text = nullptr;

	HMODULE hlib = nullptr;
};

#endif // __MAIN_H__
