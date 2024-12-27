// Error Lookup
// Copyright (c) 2011-2025 Henry++

#pragma once

#include "routine.h"

#include "resource.h"
#include "app.h"

#define LANG_MENU 5

#define SECTION_MODULE L"module"

#define FORMAT_DEC L"%" TEXT (PR_LONG)
#define FORMAT_HEX L"0x%08" TEXT (PRIX32)

typedef struct _STATIC_DATA
{
	WCHAR info[256];

	PR_HASHTABLE modules;
	PR_HASHTABLE facility;
	PR_HASHTABLE severity;

	ULONG_PTR count_unload;

	ULONG lcid;
} STATIC_DATA, *PSTATIC_DATA;

typedef struct _ITEM_MODULE
{
	PR_STRING file_name;
	PR_STRING full_path;
	PR_STRING description;
	PR_STRING string;
	PVOID hlib;
	BOOLEAN is_internal;
} ITEM_MODULE, *PITEM_MODULE;
