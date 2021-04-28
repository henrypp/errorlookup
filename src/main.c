// Error Lookup
// Copyright (c) 2011-2021 Henry++

#include "routine.h"

#include "main.h"
#include "rapp.h"

#include "resource.h"

STATIC_DATA config;

R_SPINLOCK lock_checkbox;

VOID NTAPI _app_dereferencemoduleprocedure (_In_ PVOID entry)
{
	PITEM_MODULE ptr_item = entry;

	SAFE_DELETE_REFERENCE (ptr_item->path);
	SAFE_DELETE_REFERENCE (ptr_item->full_path);
	SAFE_DELETE_REFERENCE (ptr_item->description);
	SAFE_DELETE_REFERENCE (ptr_item->text);

	SAFE_DELETE_LIBRARY (ptr_item->hlib);
}

ULONG _app_getcode (_In_ HWND hwnd, _Out_opt_ PBOOLEAN is_hex)
{
	ULONG result = 0;
	PR_STRING text;

	if (is_hex)
		*is_hex = FALSE;

	text = _r_ctrl_gettext (hwnd, IDC_CODE_CTL);

	if (text)
	{
		if ((result = _r_str_toulongex (text->buffer, 10)) == 0)
		{
			result = _r_str_toulongex (text->buffer, 16);

			if (is_hex)
				*is_hex = TRUE;
		}

		_r_obj_dereference (text);
	}

	return result;
}

VOID _app_moduleopendirectory (_In_ SIZE_T module_hash)
{
	PITEM_MODULE ptr_module;
	HMODULE hlib;

	ptr_module = _r_obj_findhashtable (config.modules, module_hash);

	if (!ptr_module)
		return;

	if (!ptr_module->full_path && ptr_module->path)
	{
		hlib = LoadLibraryEx (ptr_module->path->buffer, NULL, _r_sys_isosversiongreaterorequal (WINDOWS_VISTA) ? LOAD_LIBRARY_SEARCH_USER_DIRS | LOAD_LIBRARY_SEARCH_SYSTEM32 : 0);

		if (hlib)
		{
			_r_obj_movereference (&ptr_module->full_path, _r_path_getmodulepath (hlib));

			FreeLibrary (hlib);
		}
	}

	if (ptr_module->full_path)
		_r_shell_openfile (ptr_module->full_path->buffer);
}

_Success_ (return)
BOOLEAN _app_modulegettooltip (_Out_writes_ (buffer_size) LPWSTR buffer, _In_ SIZE_T buffer_size, _In_ SIZE_T module_hash)
{
	PITEM_MODULE ptr_module;

	ptr_module = _r_obj_findhashtable (config.modules, module_hash);

	if (!ptr_module)
		return FALSE;

	return _r_str_printf (buffer, buffer_size, L"%s: %s\r\n%s: %s", _r_locale_getstring (IDS_FILE), _r_obj_getstringorempty (ptr_module->path), _r_locale_getstring (IDS_DESCRIPTION), _r_obj_getstringorempty (ptr_module->description));
}

INT CALLBACK _app_listviewcompare_callback (_In_ LPARAM lparam1, _In_ LPARAM lparam2, _In_ LPARAM lparam)
{
	HWND hlistview = (HWND)lparam;
	HWND hwnd = GetParent (hlistview);
	INT listview_id = GetDlgCtrlID (hlistview);

	INT item1 = (INT)(INT_PTR)lparam1;
	INT item2 = (INT)(INT_PTR)lparam2;

	if (item1 == -1 || item2 == -1)
		return 0;

	WCHAR config_name[128];
	_r_str_printf (config_name, RTL_NUMBER_OF (config_name), L"listview\\%04" TEXT (PRIX32), listview_id);

	INT column_id = _r_config_getintegerex (L"SortColumn", 0, config_name);
	BOOLEAN is_descend = _r_config_getbooleanex (L"SortIsDescending", FALSE, config_name);
	PR_STRING item_text_1 = _r_listview_getitemtext (hwnd, listview_id, item1, column_id);
	PR_STRING item_text_2 = _r_listview_getitemtext (hwnd, listview_id, item2, column_id);
	INT result = 0;

	if (item_text_1 && item_text_2)
	{
		result = _r_str_compare_logical (item_text_1->buffer, item_text_2->buffer);
	}

	if (item_text_1)
		_r_obj_dereference (item_text_1);

	if (item_text_2)
		_r_obj_dereference (item_text_2);

	return is_descend ? -result : result;
}

VOID _app_listviewsort (_In_ HWND hwnd, _In_ INT listview_id, _In_ INT column_id, _In_ BOOLEAN is_notifycode)
{
	WCHAR config_name[128];
	HWND hlistview;
	INT column_count;
	BOOLEAN is_descend;

	hlistview = GetDlgItem (hwnd, listview_id);

	if (!hlistview)
		return;

	column_count = _r_listview_getcolumncount (hwnd, listview_id);

	if (!column_count)
		return;

	_r_str_printf (config_name, RTL_NUMBER_OF (config_name), L"listview\\%04" TEXT (PRIX32), listview_id);

	is_descend = _r_config_getbooleanex (L"SortIsDescending", FALSE, config_name);

	if (is_notifycode)
		is_descend = !is_descend;

	if (column_id == -1)
		column_id = _r_config_getintegerex (L"SortColumn", 0, config_name);

	column_id = _r_calc_clamp (column_id, 0, column_count - 1); // set range

	if (is_notifycode)
	{
		_r_config_setbooleanex (L"SortIsDescending", is_descend, config_name);
		_r_config_setintegerex (L"SortColumn", column_id, config_name);
	}

	for (INT i = 0; i < column_count; i++)
		_r_listview_setcolumnsortindex (hwnd, listview_id, i, 0);

	_r_listview_setcolumnsortindex (hwnd, listview_id, column_id, is_descend ? -1 : 1);

	SendMessage (hlistview, LVM_SORTITEMSEX, (WPARAM)hlistview, (LPARAM)&_app_listviewcompare_callback);
}

VOID _app_refreshstatus (_In_ HWND hwnd)
{
	SIZE_T modules_count;

	modules_count = _r_obj_gethashtablesize (config.modules);

	_r_status_settextformat (hwnd, IDC_STATUSBAR, 0, _r_locale_getstring (IDS_STATUS_TOTAL), modules_count - config.count_unload, modules_count);
}

FORCEINLINE BOOLEAN _app_isstringblacklisted (_In_ LPCWSTR string)
{
	LPCWSTR blacklist[] = {
		L"%1",
		L"Application",
		L"Classic",
		L"Critical",
		L"Error",
		L"Debug",
		L"Info",
		L"Information",
		L"Operational",
		L"System",
		L"Start",
		L"Stop",
		L"Unknown"
		L"Verbose"
		L"Warning"
	};

	for (SIZE_T i = 0; i < RTL_NUMBER_OF (blacklist); i++)
	{
		if (_r_str_compare (string, blacklist[i]) == 0)
			return TRUE;
	}

	return FALSE;
}

_Ret_maybenull_
PR_STRING _app_formatmessage (_In_ ULONG code, _In_ HINSTANCE hinstance, _In_ ULONG lang_id)
{
	ULONG allocated_length;
	PR_STRING buffer;
	ULONG attempts;
	ULONG chars;

	attempts = 6;
	allocated_length = 0x400;
	buffer = _r_obj_createstringex (NULL, allocated_length * sizeof (WCHAR));

	do
	{
		chars = FormatMessage (FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS, hinstance, code, lang_id, buffer->buffer, allocated_length, NULL);

		if (!chars)
		{
			if (GetLastError () == ERROR_INSUFFICIENT_BUFFER)
			{
				allocated_length *= 2;
				_r_obj_movereference (&buffer, _r_obj_createstringex (NULL, allocated_length * sizeof (WCHAR)));
			}
			else
			{
				_r_obj_dereference (buffer);

				if (lang_id)
					return _app_formatmessage (code, hinstance, 0);

				return NULL;
			}
		}
		else
		{
			break;
		}
	}
	while (attempts--);

	if (!chars || _app_isstringblacklisted (buffer->buffer))
	{
		_r_obj_dereference (buffer);

		return NULL;
	}

	_r_obj_trimstring (buffer, L"\r\n ");

	return buffer;
}

VOID _app_showdescription (_In_ HWND hwnd, _In_ SIZE_T module_hash)
{
	PITEM_MODULE ptr_module;

	if (module_hash != SIZE_MAX)
	{
		ptr_module = _r_obj_findhashtable (config.modules, module_hash);

		if (ptr_module)
		{
			_r_ctrl_settextformat (hwnd, IDC_DESCRIPTION_CTL, L"%s\r\n\r\n%s", config.info, _r_obj_getstringorempty (ptr_module->text));

			_r_status_settextformat (hwnd, IDC_STATUSBAR, 1, L"%s - %s", _r_obj_getstringorempty (ptr_module->description), _r_obj_getstringorempty (ptr_module->path));
		}
	}
	else
	{
		_r_ctrl_settext (hwnd, IDC_DESCRIPTION_CTL, config.info);

		_r_status_settext (hwnd, IDC_STATUSBAR, 1, NULL);
	}
}

VOID _app_print (_In_ HWND hwnd)
{
	PITEM_MODULE ptr_module;
	PR_HASHSTORE facility_table;
	PR_HASHSTORE severity_table;
	PR_STRING buffer;
	SIZE_T enum_key;
	SIZE_T module_hash;
	ULONG error_code;
	ULONG severity_code;
	ULONG facility_code;
	INT item_count = 0;

	error_code = _app_getcode (hwnd, NULL);

	severity_code = HRESULT_SEVERITY (error_code);
	facility_code = HRESULT_FACILITY (error_code);

	facility_table = _r_obj_findhashtable (config.facility, facility_code);
	severity_table = _r_obj_findhashtable (config.severity, severity_code);

	// clear first
	_r_listview_deleteallitems (hwnd, IDC_LISTVIEW);

	// print information
	_r_str_printf (config.info, RTL_NUMBER_OF (config.info),
				   L"Code (dec.): " FORMAT_DEC L"\r\nCode (hex.): " FORMAT_HEX L"\r\nFacility: %s (0x%02" TEXT (PRIX32) L")\r\nSeverity: %s (0x%02" TEXT (PRIX32) L")",
				   error_code,
				   error_code,
				   facility_table ? _r_obj_getstringorempty (facility_table->value_string) : L"n/a",
				   facility_code,
				   severity_table ? _r_obj_getstringorempty (severity_table->value_string) : L"n/a",
				   severity_code
	);

	// print modules
	enum_key = 0;

	while (_r_obj_enumhashtable (config.modules, &ptr_module, &module_hash, &enum_key))
	{
		if (!ptr_module->hlib || !ptr_module->path || !_r_config_getbooleanex (ptr_module->path->buffer, TRUE, SECTION_MODULE))
			continue;

		buffer = _app_formatmessage (error_code, ptr_module->hlib, config.lcid);
		_r_obj_movereference (&ptr_module->text, buffer);

		if (buffer)
		{
			_r_listview_additemex (hwnd, IDC_LISTVIEW, item_count, 0, _r_obj_getstring (ptr_module->description), I_IMAGENONE, I_GROUPIDNONE, module_hash);
			item_count += 1;
		}
		else
		{
			_r_ctrl_settext (hwnd, IDC_DESCRIPTION_CTL, config.info);
		}
	}

	_r_listview_setcolumn (hwnd, IDC_LISTVIEW, 0, NULL, -100);

	_app_listviewsort (hwnd, IDC_LISTVIEW, -1, FALSE);

	// show description for first item
	if (!item_count)
	{
		_app_showdescription (hwnd, SIZE_MAX);
	}
	else
	{
		ListView_SetItemState (GetDlgItem (hwnd, IDC_LISTVIEW), 0, LVIS_ACTIVATING, LVIS_ACTIVATING); // select item
	}
}

VOID _app_parsemodulescallback (_Inout_ PR_XML_LIBRARY xml_library, _In_ PVOID lparam)
{
	LPCWSTR file_value;
	LPCWSTR text_value;
	UINT file_length;
	UINT text_length;
	BOOLEAN is_enabled;
	PR_HASHTABLE hashtable;

	if (!_r_xml_getattribute (xml_library, L"file", &file_value, &file_length))
		return;

	if (!_r_xml_getattribute (xml_library, L"text", &text_value, &text_length))
		return;

	ITEM_MODULE module;
	SIZE_T module_hash;
	ULONG load_flags = LOAD_LIBRARY_AS_DATAFILE;

	if (_r_sys_isosversiongreaterorequal (WINDOWS_VISTA))
		load_flags |= LOAD_LIBRARY_AS_IMAGE_RESOURCE;

	module_hash = _r_str_hash (file_value);
	hashtable = (PR_HASHTABLE)lparam;

	if (!module_hash || _r_obj_findhashtable (hashtable, module_hash))
		return;

	memset (&module, 0, sizeof (module));

	module.path = _r_obj_createstringex (file_value, file_length * sizeof (WCHAR));
	module.description = _r_obj_createstringex (text_value, text_length * sizeof (WCHAR));

	is_enabled = _r_config_getbooleanex (module.path->buffer, TRUE, SECTION_MODULE);

	if (is_enabled)
		module.hlib = LoadLibraryEx (module.path->buffer, NULL, load_flags);

	if (!is_enabled || !module.hlib)
		config.count_unload += 1;

	_r_obj_addhashtableitem (hashtable, module_hash, &module);
}

VOID _app_parsecodescallback (_Inout_ PR_XML_LIBRARY xml_library, _In_ PVOID lparam)
{
	LPCWSTR text_value;
	LONG64 code;
	UINT text_length;
	R_HASHSTORE hashstore;
	PR_HASHTABLE hashtable;

	code = _r_xml_getattribute_long64 (xml_library, L"code");

	if (!_r_xml_getattribute (xml_library, L"text", &text_value, &text_length))
		return;

	hashtable = (PR_HASHTABLE)lparam;

	_r_obj_initializehashstore (&hashstore, _r_obj_createstringex (text_value, text_length * sizeof (WCHAR)), 0);

	_r_obj_addhashtableitem (hashtable, (SIZE_T)code, &hashstore);
}

VOID _app_loaddatabase (_In_ HWND hwnd)
{
	R_XML_LIBRARY xml_library;
	HRESULT hr;
	PVOID buffer;
	ULONG buffer_size;

	config.count_unload = 0;

	if (!config.modules)
	{
		config.modules = _r_obj_createhashtableex (sizeof (ITEM_MODULE), 128, &_app_dereferencemoduleprocedure);
	}
	else
	{
		_r_obj_clearhashtable (config.modules);
	}

	if (!config.facility)
	{
		config.facility = _r_obj_createhashtableex (sizeof (R_HASHSTORE), 64, &_r_util_dereferencehashstoreprocedure);
	}
	else
	{
		_r_obj_clearhashtable (config.facility);
	}

	if (!config.severity)
	{
		config.severity = _r_obj_createhashtableex (sizeof (R_HASHSTORE), 64, &_r_util_dereferencehashstoreprocedure);
	}
	else
	{
		_r_obj_clearhashtable (config.severity);
	}

	hr = _r_xml_initializelibrary (&xml_library, TRUE, NULL);

	if (hr != S_OK)
	{
		_r_show_errormessage (hwnd, NULL, hr, L"_r_xml_initializelibrary", NULL);
		return;
	}

	WCHAR database_path[512];
	_r_str_printf (database_path, RTL_NUMBER_OF (database_path), L"%s\\modules.xml", _r_app_getdirectory ());

	hr = S_FALSE;

	if (_r_fs_exists (database_path))
		hr = _r_xml_parsefile (&xml_library, database_path);

	if (hr != S_OK && (buffer = _r_res_loadresource (NULL, MAKEINTRESOURCE (1), RT_RCDATA, &buffer_size)))
		hr = _r_xml_parsestring (&xml_library, buffer, buffer_size);

	if (hr == S_OK)
	{
		if (_r_xml_findchildbytagname (&xml_library, L"module"))
		{
			while (_r_xml_enumchilditemsbytagname (&xml_library, L"item"))
			{
				_app_parsemodulescallback (&xml_library, config.modules);
			}
		}

		if (_r_xml_findchildbytagname (&xml_library, L"facility"))
		{
			while (_r_xml_enumchilditemsbytagname (&xml_library, L"item"))
			{
				_app_parsecodescallback (&xml_library, config.facility);
			}
		}

		if (_r_xml_findchildbytagname (&xml_library, L"severity"))
		{
			while (_r_xml_enumchilditemsbytagname (&xml_library, L"item"))
			{
				_app_parsecodescallback (&xml_library, config.severity);
			}
		}
	}

	_r_xml_destroylibrary (&xml_library);

	_app_refreshstatus (hwnd);
}

INT_PTR CALLBACK SettingsProc (_In_ HWND hwnd, _In_ UINT msg, _In_  WPARAM wparam, _In_ LPARAM lparam)
{
	switch (msg)
	{
		case RM_INITIALIZE:
		{
			INT dialog_id = (INT)wparam;

			switch (dialog_id)
			{
				case IDD_MODULES:
				{
					PITEM_MODULE ptr_module;
					SIZE_T enum_key = 0;
					SIZE_T module_hash;
					INT index = 0;

					_r_listview_deleteallitems (hwnd, IDC_MODULES);
					_r_listview_deleteallcolumns (hwnd, IDC_MODULES);

					_r_listview_setstyle (hwnd, IDC_MODULES, LVS_EX_CHECKBOXES | LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP, FALSE);

					_r_listview_addcolumn (hwnd, IDC_MODULES, 0, L"", 100, LVCFMT_LEFT);
					_r_listview_addcolumn (hwnd, IDC_MODULES, 1, L"", 100, LVCFMT_LEFT);

					_r_spinlock_acquireshared (&lock_checkbox);

					while (_r_obj_enumhashtable (config.modules, &ptr_module, &module_hash, &enum_key))
					{
						_r_listview_additemex (hwnd, IDC_MODULES, index, 0, _r_obj_getstring (ptr_module->path), I_IMAGENONE, I_GROUPIDNONE, module_hash);
						_r_listview_setitem (hwnd, IDC_MODULES, index, 1, _r_obj_getstring (ptr_module->description));

						if (ptr_module->path && _r_config_getbooleanex (ptr_module->path->buffer, TRUE, SECTION_MODULE))
							_r_listview_setitemcheck (hwnd, IDC_MODULES, index, TRUE);

						index += 1;
					}

					_r_spinlock_releaseshared (&lock_checkbox);

					_app_listviewsort (hwnd, IDC_MODULES, -1, FALSE);

					break;
				}
			}

			break;
		}

		case RM_LOCALIZE:
		{
			INT dialog_id = (INT)wparam;

			switch (dialog_id)
			{
				case IDD_MODULES:
				{
					HWND hlistview;

					hlistview = GetDlgItem (hwnd, IDC_MODULES);

					if (hlistview)
					{
						_r_listview_setcolumn (hwnd, IDC_MODULES, 0, _r_locale_getstring (IDS_FILE), -36);
						_r_listview_setcolumn (hwnd, IDC_MODULES, 1, _r_locale_getstring (IDS_DESCRIPTION), -64);
					}

					break;
				}
			}

			break;
		}

		case WM_SIZE:
		{
			HWND hlistview;
			RECT rect;

			hlistview = GetDlgItem (hwnd, IDC_MODULES);

			if (hlistview)
			{
				if (GetClientRect (hlistview, &rect))
				{
					_r_listview_setcolumn (hwnd, IDC_MODULES, 0, NULL, _r_calc_percentval (36, rect.right));
					_r_listview_setcolumn (hwnd, IDC_MODULES, 1, NULL, _r_calc_percentval (64, rect.right));
				}
			}

			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR lphdr = (LPNMHDR)lparam;

			switch (lphdr->code)
			{
				case NM_DBLCLK:
				{
					LPNMITEMACTIVATE lpnm;
					SIZE_T module_hash;

					if (lphdr->idFrom != IDC_MODULES)
						break;

					lpnm = (LPNMITEMACTIVATE)lparam;

					if (lpnm->iItem == -1)
						break;

					module_hash = _r_listview_getitemlparam (hwnd, IDC_MODULES, lpnm->iItem);

					_app_moduleopendirectory (module_hash);

					break;
				}

				case NM_CUSTOMDRAW:
				{
					if (lphdr->idFrom != IDC_MODULES)
						break;

					LPNMLVCUSTOMDRAW lpnmlv = (LPNMLVCUSTOMDRAW)lparam;
					LONG_PTR result = CDRF_DODEFAULT;

					switch (lpnmlv->nmcd.dwDrawStage)
					{
						case CDDS_PREPAINT:
						{
							result = CDRF_NOTIFYITEMDRAW;
							break;
						}

						case CDDS_ITEMPREPAINT:
						{
							PITEM_MODULE ptr_module;

							if (lpnmlv->dwItemType != LVCDI_ITEM)
								break;

							ptr_module = _r_obj_findhashtable (config.modules, lpnmlv->nmcd.lItemlParam);

							if (ptr_module)
							{
								ULONG new_clr = 0;

								if (!ptr_module->hlib && ptr_module->path && _r_config_getbooleanex (ptr_module->path->buffer, TRUE, SECTION_MODULE))
								{
									new_clr = GetSysColor (COLOR_GRAYTEXT);
								}

								if (new_clr)
								{
									lpnmlv->clrTextBk = new_clr;
									lpnmlv->clrText = _r_dc_getcolorbrightness (new_clr);

									result = CDRF_NEWFONT;
								}
							}

							break;
						}

						break;
					}

					SetWindowLongPtr (hwnd, DWLP_MSGRESULT, result);
					return result;
				}

				case LVN_ITEMCHANGED:
				{
					LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW)lparam;
					INT listview_id = (INT)(INT_PTR)lpnmlv->hdr.idFrom;

					if (listview_id != IDC_MODULES)
						break;

					if ((lpnmlv->uChanged & LVIF_STATE) != 0)
					{
						if ((lpnmlv->uNewState & LVIS_STATEIMAGEMASK) == INDEXTOSTATEIMAGEMASK (1) || ((lpnmlv->uNewState & LVIS_STATEIMAGEMASK) == INDEXTOSTATEIMAGEMASK (2)))
						{
							PITEM_MODULE ptr_module;
							ULONG load_flags;
							BOOLEAN is_enabled;

							if (_r_spinlock_islocked (&lock_checkbox))
								break;

							ptr_module = _r_obj_findhashtable (config.modules, lpnmlv->lParam);
							is_enabled = (lpnmlv->uNewState & LVIS_STATEIMAGEMASK) == INDEXTOSTATEIMAGEMASK (2);

							if (ptr_module)
							{
								_r_config_setbooleanex (ptr_module->path->buffer, is_enabled, SECTION_MODULE);

								SAFE_DELETE_LIBRARY (ptr_module->hlib);

								if (is_enabled)
								{
									load_flags = _r_sys_isosversiongreaterorequal (WINDOWS_VISTA) ? LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE : LOAD_LIBRARY_AS_DATAFILE;

									ptr_module->hlib = LoadLibraryEx (ptr_module->path->buffer, NULL, load_flags);

									if (ptr_module->hlib)
										config.count_unload -= 1;
								}
								else
								{
									SAFE_DELETE_REFERENCE (ptr_module->text);

									config.count_unload += 1;
								}

								HWND hmain = _r_app_gethwnd ();

								if (hmain)
								{
									_app_refreshstatus (hmain);
									_app_print (hmain);
								}
							}
						}
					}

					break;
				}

				case LVN_GETINFOTIP:
				{
					LPNMLVGETINFOTIP lpnmlv;
					SIZE_T module_hash;

					lpnmlv = (LPNMLVGETINFOTIP)lparam;

					if (lpnmlv->hdr.idFrom != IDC_MODULES)
						break;

					module_hash = _r_listview_getitemlparam (hwnd, IDC_MODULES, lpnmlv->iItem);

					_app_modulegettooltip (lpnmlv->pszText, lpnmlv->cchTextMax, module_hash);

					break;
				}

				case LVN_COLUMNCLICK:
				{
					LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW)lparam;
					INT ctrl_id = (INT)(INT_PTR)lpnmlv->hdr.idFrom;

					if (ctrl_id == IDC_MODULES)
						_app_listviewsort (hwnd, ctrl_id, lpnmlv->iSubItem, TRUE);

					break;
				}

			}

			break;
		}

	}

	return FALSE;
}

INT_PTR CALLBACK DlgProc (_In_ HWND hwnd, _In_ UINT msg, _In_ WPARAM wparam, _In_ LPARAM lparam)
{
	static R_LAYOUT_MANAGER layout_manager = {0};

	switch (msg)
	{
		case WM_INITDIALOG:
		{
			_r_spinlock_initialize (&lock_checkbox);

			// configure listview
			_r_listview_setstyle (hwnd, IDC_LISTVIEW, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP, FALSE);

			_r_listview_addcolumn (hwnd, IDC_LISTVIEW, 0, _r_locale_getstring (IDS_MODULES), 100, LVCFMT_LEFT);

			// configure controls
			SendDlgItemMessage (hwnd, IDC_CODE_UD, UDM_SETRANGE32, 0, INT32_MAX);

			if (_r_config_getboolean (L"InsertBufferAtStartup", FALSE))
			{
				PR_STRING clipboard_text = _r_clipboard_get (hwnd);

				if (clipboard_text)
				{
					_r_ctrl_settext (hwnd, IDC_CODE_CTL, clipboard_text->buffer);

					_r_obj_dereference (clipboard_text);
				}
			}
			else
			{
				_r_ctrl_settext (hwnd, IDC_CODE_CTL, _r_config_getstring (L"LatestCode", L"0x00000000"));
			}

			// configure settings
			_r_settings_addpage (IDD_MODULES, IDS_MODULES);

			// initialize layout manager
			_r_layout_initializemanager (&layout_manager, hwnd);

			break;
		}

		case WM_NCCREATE:
		{
			_r_wnd_enablenonclientscaling (hwnd);
			break;
		}

		case RM_INITIALIZE:
		{
			// load xml database
			_app_loaddatabase (hwnd);

			// configure menu
			HMENU hmenu = GetMenu (hwnd);

			if (hmenu)
			{
				_r_menu_checkitem (hmenu, IDM_ALWAYSONTOP_CHK, 0, MF_BYCOMMAND, _r_config_getboolean (L"AlwaysOnTop", APP_ALWAYSONTOP));
				_r_menu_checkitem (hmenu, IDM_INSERTBUFFER_CHK, 0, MF_BYCOMMAND, _r_config_getboolean (L"InsertBufferAtStartup", FALSE));
				_r_menu_checkitem (hmenu, IDM_CLASSICUI_CHK, 0, MF_BYCOMMAND, _r_config_getboolean (L"ClassicUI", APP_CLASSICUI));
				_r_menu_checkitem (hmenu, IDM_CHECKUPDATES_CHK, 0, MF_BYCOMMAND, _r_config_getboolean (L"CheckUpdates", TRUE));
			}

			break;
		}

		case WM_DESTROY:
		{
			PR_STRING window_text;

			window_text = _r_ctrl_gettext (hwnd, IDC_CODE_CTL);

			_r_config_setstring (L"LatestCode", _r_obj_getstring (window_text));

			if (window_text)
				_r_obj_dereference (window_text);

			PostQuitMessage (0);

			break;
		}

		case RM_LOCALIZE:
		{
			// get locale id
			config.lcid = _r_str_toulongex (_r_locale_getstring (IDS_LCID), 16);

			_r_listview_setcolumn (hwnd, IDC_LISTVIEW, 0, _r_locale_getstring (IDS_MODULES), 0);

			// localize
			HMENU hmenu = GetMenu (hwnd);

			if (hmenu)
			{
				_r_menu_setitemtext (hmenu, 0, TRUE, _r_locale_getstring (IDS_FILE));
				_r_menu_setitemtext (hmenu, 1, TRUE, _r_locale_getstring (IDS_SETTINGS));
				_r_menu_setitemtext (hmenu, 2, TRUE, _r_locale_getstring (IDS_HELP));

				_r_menu_setitemtextformat (hmenu, IDM_SETTINGS, FALSE, L"%s...\tF2", _r_locale_getstring (IDS_SETTINGS));
				_r_menu_setitemtextformat (hmenu, IDM_EXIT, FALSE, L"%s\tEsc", _r_locale_getstring (IDS_EXIT));
				_r_menu_setitemtext (hmenu, IDM_ALWAYSONTOP_CHK, FALSE, _r_locale_getstring (IDS_ALWAYSONTOP_CHK));
				_r_menu_setitemtext (hmenu, IDM_INSERTBUFFER_CHK, FALSE, _r_locale_getstring (IDS_INSERTBUFFER_CHK));
				_r_menu_setitemtextformat (hmenu, IDM_CLASSICUI_CHK, FALSE, L"%s*", _r_locale_getstring (IDS_CLASSICUI_CHK));
				_r_menu_setitemtext (hmenu, IDM_CHECKUPDATES_CHK, FALSE, _r_locale_getstring (IDS_CHECKUPDATES_CHK));
				_r_menu_setitemtextformat (GetSubMenu (hmenu, 1), LANG_MENU, TRUE, L"%s (Language)", _r_locale_getstring (IDS_LANGUAGE));

				_r_menu_setitemtext (hmenu, IDM_WEBSITE, FALSE, _r_locale_getstring (IDS_WEBSITE));
				_r_menu_setitemtext (hmenu, IDM_CHECKUPDATES, FALSE, _r_locale_getstring (IDS_CHECKUPDATES));

				_r_menu_setitemtextformat (hmenu, IDM_ABOUT, FALSE, L"%s\tF1", _r_locale_getstring (IDS_ABOUT));

				_r_locale_enum (GetSubMenu (hmenu, 1), LANG_MENU, IDX_LANGUAGE); // enum localizations
			}

			_r_ctrl_settextformat (hwnd, IDC_CODE, L"%s:", _r_locale_getstring (IDS_CODE));
			_r_ctrl_settextformat (hwnd, IDC_DESCRIPTION, L"%s:", _r_locale_getstring (IDS_DESCRIPTION));

			_app_print (hwnd);

			_app_refreshstatus (hwnd);

			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR lphdr = (LPNMHDR)lparam;

			switch (lphdr->code)
			{
				case NM_DBLCLK:
				{
					if (lphdr->idFrom != IDC_LISTVIEW)
						break;

					LPNMITEMACTIVATE lpnm = (LPNMITEMACTIVATE)lparam;

					if (lpnm->iItem == -1)
						break;

					SIZE_T module_hash = _r_listview_getitemlparam (hwnd, IDC_LISTVIEW, lpnm->iItem);

					_app_moduleopendirectory (module_hash);

					break;
				}

				case NM_CLICK:
				case LVN_ITEMCHANGED:
				{
					if (lphdr->idFrom != IDC_LISTVIEW)
						break;

					LPNMITEMACTIVATE lpnm = (LPNMITEMACTIVATE)lparam;

					if (lpnm->iItem != -1)
					{
						SIZE_T module_hash = _r_listview_getitemlparam (hwnd, IDC_LISTVIEW, lpnm->iItem);

						_app_showdescription (hwnd, module_hash);
					}
					else
					{
						_app_showdescription (hwnd, SIZE_MAX);
					}

					break;
				}

				case LVN_GETINFOTIP:
				{
					LPNMLVGETINFOTIP lpnmlv = (LPNMLVGETINFOTIP)lparam;

					if (lpnmlv->hdr.idFrom != IDC_LISTVIEW)
						break;

					SIZE_T module_hash = _r_listview_getitemlparam (hwnd, IDC_LISTVIEW, lpnmlv->iItem);

					_app_modulegettooltip (lpnmlv->pszText, lpnmlv->cchTextMax, module_hash);

					break;
				}

				case LVN_COLUMNCLICK:
				{
					LPNMLISTVIEW lpnmlv = (LPNMLISTVIEW)lparam;
					INT ctrl_id = (INT)(INT_PTR)lpnmlv->hdr.idFrom;

					if (ctrl_id == IDC_LISTVIEW)
						_app_listviewsort (hwnd, ctrl_id, lpnmlv->iSubItem, TRUE);

					break;
				}

				case UDN_DELTAPOS:
				{
					if (lphdr->idFrom == IDC_CODE_UD)
					{
						LPNMUPDOWN lpnmud = (LPNMUPDOWN)lparam;
						ULONG code;
						BOOLEAN is_hex;

						code = _app_getcode (hwnd, &is_hex);

						_r_ctrl_settextformat (hwnd, IDC_CODE_CTL, is_hex ? FORMAT_HEX : FORMAT_DEC, code + lpnmud->iDelta);
						_app_print (hwnd);

						return TRUE;
					}

					break;
				}
			}

			break;
		}

		case WM_SIZE:
		{
			RECT rect;
			HWND hlistview = GetDlgItem (hwnd, IDC_MODULES);

			if (!_r_layout_resize (&layout_manager, wparam))
				break;

			// resize statusbar parts
			hlistview = GetDlgItem (hwnd, IDC_LISTVIEW);

			if (!hlistview || !GetClientRect (hlistview, &rect))
				break;

			INT parts[] = {rect.right + _r_dc_getdpi (hwnd, 24), -1};
			SendDlgItemMessage (hwnd, IDC_STATUSBAR, SB_SETPARTS, 2, (LPARAM)parts);

			// resize column width
			_r_listview_setcolumn (hwnd, IDC_LISTVIEW, 0, NULL, rect.right);

			break;
		}

		case WM_GETMINMAXINFO:
		{
			_r_layout_resizeminimumsize (&layout_manager, lparam);
			break;
		}

		case WM_COMMAND:
		{
			INT ctrl_id = LOWORD (wparam);
			INT notify_code = HIWORD (wparam);

			if (!notify_code && ctrl_id >= IDX_LANGUAGE && ctrl_id <= IDX_LANGUAGE + (INT)_r_locale_getcount ())
			{
				_r_locale_applyfrommenu (GetSubMenu (GetSubMenu (GetMenu (hwnd), 1), LANG_MENU), ctrl_id);
				return FALSE;
			}
			else if (ctrl_id == IDC_CODE_CTL && notify_code == EN_CHANGE)
			{
				_app_print (hwnd);
				return FALSE;
			}

			switch (ctrl_id)
			{
				case IDM_SETTINGS:
				{
					_r_settings_createwindow (hwnd, &SettingsProc, 0);
					break;
				}

				case IDCANCEL: // process Esc key
				case IDM_EXIT:
				{
					DestroyWindow (hwnd);
					break;
				}

				case IDM_ALWAYSONTOP_CHK:
				{
					BOOLEAN new_val = !_r_config_getboolean (L"AlwaysOnTop", APP_ALWAYSONTOP);

					_r_menu_checkitem (GetMenu (hwnd), ctrl_id, 0, MF_BYCOMMAND, new_val);
					_r_config_setboolean (L"AlwaysOnTop", new_val);

					_r_wnd_top (hwnd, new_val);

					break;
				}

				case IDM_INSERTBUFFER_CHK:
				{
					BOOLEAN new_val = !_r_config_getboolean (L"InsertBufferAtStartup", FALSE);

					_r_menu_checkitem (GetMenu (hwnd), ctrl_id, 0, MF_BYCOMMAND, new_val);
					_r_config_setboolean (L"InsertBufferAtStartup", new_val);

					break;
				}

				case IDM_CHECKUPDATES_CHK:
				{
					BOOLEAN new_val = !_r_config_getboolean (L"CheckUpdates", TRUE);

					_r_menu_checkitem (GetMenu (hwnd), ctrl_id, 0, MF_BYCOMMAND, new_val);
					_r_config_setboolean (L"CheckUpdates", new_val);

					break;
				}

				case IDM_CLASSICUI_CHK:
				{
					BOOLEAN new_val = !_r_config_getboolean (L"ClassicUI", APP_CLASSICUI);

					_r_menu_checkitem (GetMenu (hwnd), ctrl_id, 0, MF_BYCOMMAND, new_val);
					_r_config_setboolean (L"ClassicUI", new_val);

					_r_app_restart (hwnd);

					break;
				}

				case IDM_WEBSITE:
				{
					ShellExecute (hwnd, NULL, _r_app_getwebsite_url (), NULL, NULL, SW_SHOWDEFAULT);
					break;
				}

				case IDM_CHECKUPDATES:
				{
					_r_update_check (hwnd);
					break;
				}

				case IDM_ABOUT:
				{
					_r_show_aboutmessage (hwnd);
					break;
				}

				case IDM_ZOOM:
				{
					ShowWindow (hwnd, IsZoomed (hwnd) ? SW_RESTORE : SW_MAXIMIZE);
					break;
				}
			}

			break;
		}
	}

	return FALSE;
}

INT APIENTRY wWinMain (_In_ HINSTANCE hinst, _In_opt_ HINSTANCE prev_hinst, _In_ LPWSTR cmdline, _In_ INT show_cmd)
{
	MSG msg;

	RtlSecureZeroMemory (&config, sizeof (config));

	if (_r_app_initialize ())
	{
		if (_r_app_createwindow (IDD_MAIN, IDI_MAIN, &DlgProc))
		{
			HACCEL haccel = LoadAccelerators (hinst, MAKEINTRESOURCE (IDA_MAIN));

			if (haccel)
			{
				while (GetMessage (&msg, NULL, 0, 0) > 0)
				{
					HWND hwnd = GetActiveWindow ();

					if (!TranslateAccelerator (hwnd, haccel, &msg) && !IsDialogMessage (hwnd, &msg))
					{
						TranslateMessage (&msg);
						DispatchMessage (&msg);
					}
				}

				DestroyAcceleratorTable (haccel);
			}
		}
	}

	return ERROR_SUCCESS;
}
