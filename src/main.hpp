// Error Lookup
// Copyright (c) 2011-2019 Henry++

#pragma once

#include <windows.h>
#include <commctrl.h>

#include "routine.hpp"
#include "resource.hpp"
#include "app.hpp"

#define MODULES_MENU 5
#define LANG_MENU 6

// pugixml document configuration
#define PUGIXML_LOAD_FLAGS (pugi::parse_escapes)
#define PUGIXML_LOAD_ENCODING (pugi::encoding_auto)

#define SECTION_MODULE L"module"

#define FORMAT_DEC L"%lu"
#define FORMAT_HEX L"0x%.8lx"

typedef struct tagITEM_MODULE
{
	~tagITEM_MODULE()
	{
		if (hlib)
		{
			FreeLibrary (hlib);
			hlib = nullptr;
		}

		SAFE_DELETE_ARRAY (path);
		SAFE_DELETE_ARRAY (description);

		SAFE_DELETE_ARRAY (text);
	}

	LPWSTR path = nullptr;
	LPWSTR description = nullptr;

	LPWSTR text = nullptr;

	HMODULE hlib = nullptr;
} ITEM_MODULE, *PITEM_MODULE;
