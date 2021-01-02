// Error Lookup
// Copyright (c) 2011-2021 Henry++

#pragma once

#include "routine.h"

#include "resource.h"
#include "app.h"

#define LANG_MENU 5

#define SECTION_MODULE L"module"

#define FORMAT_DEC L"%" TEXT (PR_ULONG)
#define FORMAT_HEX L"0x%08" TEXT (PRIX32)

typedef struct _ITEM_MODULE
{
	PR_STRING path;
	PR_STRING full_path;
	PR_STRING description;
	PR_STRING text;
	HMODULE hlib;
} ITEM_MODULE, *PITEM_MODULE;
