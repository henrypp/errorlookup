// Error Lookup
// Copyright (c) 2011-2021 Henry++

#include "routine.h"

#include "main.h"
#include "rapp.h"

#include "resource.h"

STATIC_DATA config = {0};

VOID NTAPI _app_dereferencemoduleprocedure (
	_In_ PVOID entry
)
{
	PITEM_MODULE ptr_item;

	ptr_item = entry;

	if (ptr_item->path)
		_r_obj_dereference (ptr_item->path);

	if (ptr_item->full_path)
		_r_obj_dereference (ptr_item->full_path);

	if (ptr_item->description)
		_r_obj_dereference (ptr_item->description);

	if (ptr_item->text)
		_r_obj_dereference (ptr_item->text);

	if (ptr_item->hlib)
		FreeLibrary (ptr_item->hlib);
}

VOID _app_moduleopendirectory (
	_In_ ULONG_PTR module_hash
)
{
	PITEM_MODULE ptr_module;
	HMODULE hlib;

	ptr_module = _r_obj_findhashtable (config.modules, module_hash);

	if (!ptr_module)
		return;

	if (!ptr_module->full_path && ptr_module->path)
	{
		hlib = _r_sys_loadlibrary (ptr_module->path->buffer);

		if (hlib)
		{
			_r_obj_movereference (&ptr_module->full_path, _r_path_getmodulepath (hlib));

			FreeLibrary (hlib);
		}
	}

	if (ptr_module->full_path)
		_r_shell_showfile (ptr_module->full_path->buffer);
}

VOID _app_modulegettooltip (
	_Out_writes_ (buffer_size) LPWSTR buffer,
	_In_ SIZE_T buffer_size,
	_In_ ULONG_PTR module_hash
)
{
	PITEM_MODULE ptr_module;

	ptr_module = _r_obj_findhashtable (config.modules, module_hash);

	if (!ptr_module)
	{
		*buffer = UNICODE_NULL;
		return;
	}

	_r_str_printf (
		buffer,
		buffer_size,
		L"%s: %s\r\n%s: %s",
		_r_locale_getstring (IDS_FILE),
		_r_obj_getstringorempty (ptr_module->path),
		_r_locale_getstring (IDS_DESCRIPTION),
		_r_obj_getstringorempty (ptr_module->description)
	);
}

INT CALLBACK _app_listviewcompare_callback (
	_In_ LPARAM lparam1,
	_In_ LPARAM lparam2,
	_In_ LPARAM lparam
)
{
	static R_STRINGREF sr1 = PR_STRINGREF_INIT (L"Windows (");

	HWND hlistview;
	HWND hwnd;

	PR_STRING item_text_1;
	PR_STRING item_text_2;

	INT listview_id;
	INT column_id;
	INT result;

	INT item1;
	INT item2;

	BOOLEAN is_descend;

	item1 = (INT)(INT_PTR)lparam1;
	item2 = (INT)(INT_PTR)lparam2;

	if (item1 == -1 || item2 == -1)
		return 0;

	hlistview = (HWND)lparam;
	hwnd = GetParent (hlistview);
	listview_id = GetDlgCtrlID (hlistview);

	WCHAR config_name[128];
	_r_str_printf (config_name, RTL_NUMBER_OF (config_name), L"listview\\%04" TEXT (PRIX32), listview_id);

	column_id = _r_config_getlong_ex (L"SortColumn", 0, config_name);

	item_text_1 = _r_listview_getitemtext (hwnd, listview_id, item1, column_id);
	item_text_2 = _r_listview_getitemtext (hwnd, listview_id, item2, column_id);

	is_descend = _r_config_getboolean_ex (L"SortIsDescending", FALSE, config_name);

	result = 0;

	if (item_text_1 && item_text_2)
	{
		if (listview_id == IDC_LISTVIEW)
		{
			if (_r_str_isstartswith (&item_text_1->sr, &sr1, TRUE))
			{
				result = -1;
			}
			else if (_r_str_isstartswith (&item_text_2->sr, &sr1, TRUE))
			{
				result = 1;
			}
		}

		if (!result)
			result = _r_str_compare_logical (item_text_1, item_text_2);
	}

	if (item_text_1)
		_r_obj_dereference (item_text_1);

	if (item_text_2)
		_r_obj_dereference (item_text_2);

	return is_descend ? -result : result;
}

VOID _app_listviewsort (
	_In_ HWND hwnd,
	_In_ INT listview_id,
	_In_ INT column_id,
	_In_ BOOLEAN is_notifycode
)
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

	is_descend = _r_config_getboolean_ex (L"SortIsDescending", FALSE, config_name);

	if (is_notifycode)
		is_descend = !is_descend;

	if (column_id == -1)
		column_id = _r_config_getlong_ex (L"SortColumn", 0, config_name);

	column_id = _r_calc_clamp (column_id, 0, column_count - 1); // set range

	if (is_notifycode)
	{
		_r_config_setboolean_ex (L"SortIsDescending", is_descend, config_name);
		_r_config_setlong_ex (L"SortColumn", column_id, config_name);
	}

	for (INT i = 0; i < column_count; i++)
		_r_listview_setcolumnsortindex (hwnd, listview_id, i, 0);

	_r_listview_setcolumnsortindex (hwnd, listview_id, column_id, is_descend ? -1 : 1);

	SendMessage (hlistview, LVM_SORTITEMSEX, (WPARAM)hlistview, (LPARAM)&_app_listviewcompare_callback);
}

VOID _app_refreshstatus (
	_In_ HWND hwnd
)
{
	SIZE_T modules_count;

	modules_count = _r_obj_gethashtablesize (config.modules);

	_r_status_settextformat (
		hwnd,
		IDC_STATUSBAR,
		0,
		_r_locale_getstring (IDS_STATUS_TOTAL),
		modules_count - config.count_unload,
		modules_count
	);
}

VOID _app_showdescription (
	_In_ HWND hwnd,
	_In_ ULONG_PTR module_hash
)
{
	PITEM_MODULE ptr_module;

	if (module_hash != SIZE_MAX)
	{
		ptr_module = _r_obj_findhashtable (config.modules, module_hash);

		if (ptr_module)
		{
			_r_ctrl_setstringformat (
				hwnd,
				IDC_DESCRIPTION_CTL,
				L"%s\r\n\r\n%s",
				config.info,
				_r_obj_getstringorempty (ptr_module->text)
			);

			_r_status_settextformat (
				hwnd,
				IDC_STATUSBAR,
				1,
				L"%s - %s",
				_r_obj_getstringorempty (ptr_module->description),
				_r_obj_getstringorempty (ptr_module->path)
			);
		}
	}
	else
	{
		_r_ctrl_setstring (
			hwnd,
			IDC_DESCRIPTION_CTL,
			config.info
		);

		_r_status_settext (
			hwnd,
			IDC_STATUSBAR,
			1,
			NULL
		);
	}
}

VOID _app_print (
	_In_ HWND hwnd
)
{
	PITEM_MODULE ptr_module;
	PR_STRING severity_string;
	PR_STRING facility_string;
	PR_STRING buffer;
	SIZE_T enum_key;
	ULONG_PTR module_hash;
	ULONG error_code;
	ULONG severity_code;
	ULONG facility_code;
	INT item_count;
	ULONG status;

	error_code = (ULONG)_r_ctrl_getinteger (hwnd, IDC_CODE_CTL, NULL);

	severity_code = HRESULT_SEVERITY (error_code);
	facility_code = HRESULT_FACILITY (error_code);

	severity_string = _r_obj_findhashtablepointer (config.severity, severity_code);
	facility_string = _r_obj_findhashtablepointer (config.facility, facility_code);

	// clear first
	_r_listview_deleteallitems (hwnd, IDC_LISTVIEW);

	// print information
	_r_str_printf (
		config.info, RTL_NUMBER_OF (config.info),
		L"Code (dec.): " FORMAT_DEC L"\r\nCode (hex.): " FORMAT_HEX L"\r\nSeverity: %s (0x%02" TEXT (PRIX32) L")\r\nFacility: %s (0x%02" TEXT (PRIX32) L")",
		error_code,
		error_code,
		_r_obj_getstringordefault (severity_string, L"n/a"),
		severity_code,
		_r_obj_getstringordefault (facility_string, L"n/a"),
		facility_code
	);

	// print modules
	enum_key = 0;
	item_count = 0;

	while (_r_obj_enumhashtable (config.modules, &ptr_module, &module_hash, &enum_key))
	{
		if (!ptr_module->hlib || !ptr_module->path || !_r_config_getboolean_ex (ptr_module->path->buffer, TRUE, SECTION_MODULE))
			continue;

		status = _r_sys_formatmessage (error_code, ptr_module->hlib, config.lcid, &buffer);

		_r_obj_movereference (&ptr_module->text, buffer);

		if (status == ERROR_SUCCESS)
		{
			_r_listview_additem_ex (
				hwnd,
				IDC_LISTVIEW,
				item_count,
				_r_obj_getstring (ptr_module->description),
				I_IMAGENONE,
				I_GROUPIDNONE,
				module_hash
			);

			item_count += 1;
		}
		else
		{
			_r_ctrl_setstring (hwnd, IDC_DESCRIPTION_CTL, config.info);
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
		_r_listview_setitemstate (hwnd, IDC_LISTVIEW, 0, LVIS_ACTIVATING, LVIS_ACTIVATING); // select item
	}

	if (severity_string)
		_r_obj_dereference (severity_string);

	if (facility_string)
		_r_obj_dereference (facility_string);
}

VOID _app_parsexmlcallback (
	_Inout_ PR_XML_LIBRARY xml_library,
	_Inout_ PR_HASHTABLE hashtable,
	_In_ BOOLEAN is_modules
)
{

	R_STRINGREF file_value;
	R_STRINGREF text_value;
	ITEM_MODULE module;
	ULONG64 code;
	ULONG_PTR module_hash;
	ULONG load_flags;
	BOOLEAN is_enabled;

	if (is_modules)
	{
		if (!_r_xml_getattribute (xml_library, L"file", &file_value))
			return;

		if (!_r_xml_getattribute (xml_library, L"text", &text_value))
			return;

		load_flags = LOAD_LIBRARY_AS_DATAFILE;

		if (_r_sys_isosversiongreaterorequal (WINDOWS_7))
			load_flags |= LOAD_LIBRARY_AS_IMAGE_RESOURCE;

		module_hash = _r_str_gethash3 (&file_value, TRUE);

		if (!module_hash || _r_obj_findhashtable (hashtable, module_hash))
			return;

		RtlZeroMemory (&module, sizeof (module));

		module.path = _r_obj_createstring3 (&file_value);
		module.description = _r_obj_createstring3 (&text_value);

		is_enabled = _r_config_getboolean_ex (module.path->buffer, TRUE, SECTION_MODULE);

		if (is_enabled)
			module.hlib = LoadLibraryEx (module.path->buffer, NULL, load_flags);

		if (!is_enabled || !module.hlib)
			config.count_unload += 1;

		_r_obj_addhashtableitem (hashtable, module_hash, &module);
	}
	else
	{
		code = _r_xml_getattribute_long64 (xml_library, L"code");

		if (!_r_xml_getattribute (xml_library, L"text", &text_value))
			return;

		_r_obj_addhashtablepointer (hashtable, (ULONG_PTR)code, _r_obj_createstring3 (&text_value));
	}
}

VOID _app_loaddatabase (
	_In_ HWND hwnd
)
{
	R_XML_LIBRARY xml_library;
	R_BYTEREF bytes;
	HRESULT hr;

	config.count_unload = 0;

	if (!config.modules)
	{
		config.modules = _r_obj_createhashtable_ex (
			sizeof (ITEM_MODULE),
			128,
			&_app_dereferencemoduleprocedure
		);
	}
	else
	{
		_r_obj_clearhashtable (config.modules);
	}

	if (!config.facility)
	{
		config.facility = _r_obj_createhashtablepointer (64);
	}
	else
	{
		_r_obj_clearhashtable (config.facility);
	}

	if (!config.severity)
	{
		config.severity = _r_obj_createhashtablepointer (64);
	}
	else
	{
		_r_obj_clearhashtable (config.severity);
	}

	hr = _r_xml_initializelibrary (&xml_library, TRUE);

	if (hr != S_OK)
	{
		_r_show_errormessage (hwnd, NULL, hr, NULL);
		return;
	}

	WCHAR database_path[512];
	_r_str_printf (database_path, RTL_NUMBER_OF (database_path), L"%s\\modules.xml", _r_app_getdirectory ()->buffer);

	hr = S_FALSE;

	if (_r_fs_exists (database_path))
		hr = _r_xml_parsefile (&xml_library, database_path);

	if (hr != S_OK && _r_res_loadresource (NULL, MAKEINTRESOURCE (1), RT_RCDATA, &bytes))
		hr = _r_xml_parsestring (&xml_library, bytes.buffer, (ULONG)bytes.length);

	if (hr == S_OK)
	{
		if (_r_xml_findchildbytagname (&xml_library, L"module"))
		{
			while (_r_xml_enumchilditemsbytagname (&xml_library, L"item"))
			{
				_app_parsexmlcallback (&xml_library, config.modules, TRUE);
			}
		}

		if (_r_xml_findchildbytagname (&xml_library, L"facility"))
		{
			while (_r_xml_enumchilditemsbytagname (&xml_library, L"item"))
			{
				_app_parsexmlcallback (&xml_library, config.facility, FALSE);
			}
		}

		if (_r_xml_findchildbytagname (&xml_library, L"severity"))
		{
			while (_r_xml_enumchilditemsbytagname (&xml_library, L"item"))
			{
				_app_parsexmlcallback (&xml_library, config.severity, FALSE);
			}
		}
	}

	_r_xml_destroylibrary (&xml_library);

	_app_refreshstatus (hwnd);
}

INT_PTR CALLBACK SettingsProc (
	_In_ HWND hwnd,
	_In_ UINT msg,
	_In_ WPARAM wparam,
	_In_ LPARAM lparam
)
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
					ULONG_PTR module_hash;
					INT index = 0;

					_r_listview_deleteallitems (hwnd, IDC_MODULES);
					_r_listview_deleteallcolumns (hwnd, IDC_MODULES);

					_r_listview_setstyle (
						hwnd,
						IDC_MODULES,
						LVS_EX_CHECKBOXES | LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP,
						FALSE
					);

					_r_listview_addcolumn (hwnd, IDC_MODULES, 0, L"", 100, LVCFMT_LEFT);
					_r_listview_addcolumn (hwnd, IDC_MODULES, 1, L"", 100, LVCFMT_LEFT);

					while (_r_obj_enumhashtable (config.modules, &ptr_module, &module_hash, &enum_key))
					{
						_r_listview_additem_ex (
							hwnd,
							IDC_MODULES,
							index,
							_r_obj_getstring (ptr_module->path),
							I_IMAGENONE,
							I_GROUPIDNONE,
							module_hash
						);

						_r_listview_setitem (hwnd, IDC_MODULES, index, 1, _r_obj_getstring (ptr_module->description));

						if (ptr_module->path && _r_config_getboolean_ex (ptr_module->path->buffer, TRUE, SECTION_MODULE))
							_r_listview_setitemcheck (hwnd, IDC_MODULES, index, TRUE);

						index += 1;
					}

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
					_r_listview_setcolumn (hwnd, IDC_MODULES, 0, _r_locale_getstring (IDS_FILE), -36);
					_r_listview_setcolumn (hwnd, IDC_MODULES, 1, _r_locale_getstring (IDS_DESCRIPTION), -64);

					break;
				}
			}

			break;
		}

		case RM_CONFIG_SAVE:
		{
			INT dialog_id = (INT)wparam;

			switch (dialog_id)
			{
				case IDD_MODULES:
				{
					HWND hmain;
					PITEM_MODULE ptr_module;
					ULONG load_flags;
					INT item_count;
					ULONG_PTR module_hash;
					BOOLEAN is_enabled;
					BOOLEAN is_enabled_default;

					item_count = _r_listview_getitemcount (hwnd, IDC_MODULES);

					if (!item_count)
						break;

					if (_r_sys_isosversiongreaterorequal (WINDOWS_7))
					{
						load_flags = LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE;

					}
					else
					{
						load_flags = LOAD_LIBRARY_AS_DATAFILE;
					}

					//config.count_unload = 0;

					for (INT i = 0; i < item_count; i++)
					{
						module_hash = _r_listview_getitemlparam (hwnd, IDC_MODULES, i);
						ptr_module = _r_obj_findhashtable (config.modules, module_hash);

						if (!ptr_module)
							continue;

						is_enabled = _r_listview_isitemchecked (hwnd, IDC_MODULES, i);
						is_enabled_default = _r_config_getboolean_ex (ptr_module->path->buffer, TRUE, SECTION_MODULE);

						if (is_enabled == is_enabled_default)
							continue;

						_r_config_setboolean_ex (ptr_module->path->buffer, is_enabled, SECTION_MODULE);

						if (is_enabled)
						{
							if (!ptr_module->hlib)
								ptr_module->hlib = LoadLibraryEx (ptr_module->path->buffer, NULL, load_flags);

							if (ptr_module->hlib)
								config.count_unload -= 1;
						}
						else
						{
							SAFE_DELETE_LIBRARY (ptr_module->hlib);
							SAFE_DELETE_REFERENCE (ptr_module->text);

							config.count_unload += 1;
						}
					}

					hmain = _r_app_gethwnd ();

					if (hmain)
					{
						_app_refreshstatus (hmain);
						_app_print (hmain);
					}

					break;
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
					ULONG_PTR module_hash;

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
					LPNMLVCUSTOMDRAW lpnmlv;
					LONG_PTR result;

					if (lphdr->idFrom != IDC_MODULES)
						break;

					lpnmlv = (LPNMLVCUSTOMDRAW)lparam;
					result = CDRF_DODEFAULT;

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
							ULONG new_clr;

							if (lpnmlv->dwItemType != LVCDI_ITEM)
								break;

							ptr_module = _r_obj_findhashtable (config.modules, lpnmlv->nmcd.lItemlParam);

							if (!ptr_module)
								break;

							if (!ptr_module->hlib && ptr_module->path && _r_config_getboolean_ex (ptr_module->path->buffer, TRUE, SECTION_MODULE))
							{
								new_clr = GetSysColor (COLOR_GRAYTEXT);

								lpnmlv->clrTextBk = new_clr;
								lpnmlv->clrText = _r_dc_getcolorbrightness (new_clr);

								result = CDRF_NEWFONT;
							}

							break;
						}

						break;
					}

					SetWindowLongPtr (hwnd, DWLP_MSGRESULT, result);
					return result;
				}

				case LVN_GETINFOTIP:
				{
					LPNMLVGETINFOTIP lpnmlv;
					ULONG_PTR module_hash;

					lpnmlv = (LPNMLVGETINFOTIP)lparam;

					if (lpnmlv->hdr.idFrom != IDC_MODULES)
						break;

					module_hash = _r_listview_getitemlparam (hwnd, IDC_MODULES, lpnmlv->iItem);

					_app_modulegettooltip (lpnmlv->pszText, lpnmlv->cchTextMax, module_hash);

					break;
				}

				case LVN_COLUMNCLICK:
				{
					LPNMLISTVIEW lpnmlv;
					INT ctrl_id;

					lpnmlv = (LPNMLISTVIEW)lparam;
					ctrl_id = (INT)(INT_PTR)lpnmlv->hdr.idFrom;

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

INT_PTR CALLBACK DlgProc (
	_In_ HWND hwnd,
	_In_ UINT msg,
	_In_ WPARAM wparam,
	_In_ LPARAM lparam
)
{
	static R_LAYOUT_MANAGER layout_manager = {0};

	switch (msg)
	{
		case WM_INITDIALOG:
		{
			PR_STRING string;

			_r_app_sethwnd (hwnd); // HACK!!!

			// configure listview
			_r_listview_setstyle (
				hwnd,
				IDC_LISTVIEW,
				LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP,
				FALSE
			);

			_r_listview_addcolumn (hwnd, IDC_LISTVIEW, 0, _r_locale_getstring (IDS_MODULES), 100, LVCFMT_LEFT);

			// configure controls
			SendDlgItemMessage (hwnd, IDC_CODE_UD, UDM_SETRANGE32, 0, INT32_MAX);

			// set error code text
			if (_r_config_getboolean (L"InsertBufferAtStartup", FALSE))
			{
				string = _r_clipboard_get (hwnd);

				if (string)
				{
					_r_str_trimstring2 (string, L"\r\n ", 0);

					if (_r_obj_isstringempty2 (string))
						_r_obj_clearreference (&string);
				}
			}
			else
			{
				string = NULL;
			}

			if (!string)
				string = _r_config_getstring (L"LatestCode", L"0x00000000");

			if (string)
			{
				_r_ctrl_setstring (hwnd, IDC_CODE_CTL, string->buffer);

				_r_obj_dereference (string);
			}

			// configure settings
			_r_settings_addpage (IDD_MODULES, IDS_MODULES);

			// initialize layout manager
			_r_layout_initializemanager (&layout_manager, hwnd);

			break;
		}

		case RM_INITIALIZE:
		{
			HMENU hmenu;

			// load xml database
			_app_loaddatabase (hwnd);

			// configure menu
			hmenu = GetMenu (hwnd);

			if (hmenu)
			{
				_r_menu_checkitem (hmenu, IDM_ALWAYSONTOP_CHK, 0, MF_BYCOMMAND, _r_config_getboolean (L"AlwaysOnTop", FALSE));
				_r_menu_checkitem (hmenu, IDM_INSERTBUFFER_CHK, 0, MF_BYCOMMAND, _r_config_getboolean (L"InsertBufferAtStartup", FALSE));
				_r_menu_checkitem (hmenu, IDM_CHECKUPDATES_CHK, 0, MF_BYCOMMAND, _r_update_isenabled (FALSE));
			}

			break;
		}

		case WM_DESTROY:
		{
			PR_STRING window_text;

			window_text = _r_ctrl_getstring (hwnd, IDC_CODE_CTL);

			_r_config_setstring (L"LatestCode", _r_obj_getstring (window_text));

			if (window_text)
				_r_obj_dereference (window_text);

			PostQuitMessage (0);

			break;
		}

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;

			hdc = BeginPaint (hwnd, &ps);

			if (hdc)
			{
				_r_dc_drawwindow (hdc, hwnd, FALSE);

				EndPaint (hwnd, &ps);
			}

			break;
		}

		case WM_ERASEBKGND:
		{
			return TRUE;
		}

		case RM_LOCALIZE:
		{
			// get locale id
			PR_STRING string;
			HMENU hmenu;
			LONG64 value;
			ULONG number;

			string = _r_locale_getstring_ex (IDS_LCID);

			if (string)
			{
				if (_r_str_tointeger64 (&string->sr, 0, NULL, &value))
				{
					if (LongLongToULong (value, &number) == S_OK)
						config.lcid = number;
				}

				_r_obj_dereference (string);
			}
			else
			{
				config.lcid = 0;
			}

			_r_listview_setcolumn (hwnd, IDC_LISTVIEW, 0, _r_locale_getstring (IDS_MODULES), 0);

			// localize
			hmenu = GetMenu (hwnd);

			if (hmenu)
			{
				_r_menu_setitemtext (hmenu, 0, TRUE, _r_locale_getstring (IDS_FILE));
				_r_menu_setitemtext (hmenu, 1, TRUE, _r_locale_getstring (IDS_SETTINGS));
				_r_menu_setitemtext (hmenu, 2, TRUE, _r_locale_getstring (IDS_HELP));

				_r_menu_setitemtextformat (hmenu, IDM_SETTINGS, FALSE, L"%s...\tF2", _r_locale_getstring (IDS_SETTINGS));
				_r_menu_setitemtextformat (hmenu, IDM_EXIT, FALSE, L"%s\tEsc", _r_locale_getstring (IDS_EXIT));
				_r_menu_setitemtext (hmenu, IDM_ALWAYSONTOP_CHK, FALSE, _r_locale_getstring (IDS_ALWAYSONTOP_CHK));
				_r_menu_setitemtext (hmenu, IDM_INSERTBUFFER_CHK, FALSE, _r_locale_getstring (IDS_INSERTBUFFER_CHK));
				_r_menu_setitemtext (hmenu, IDM_CHECKUPDATES_CHK, FALSE, _r_locale_getstring (IDS_CHECKUPDATES_CHK));
				_r_menu_setitemtextformat (GetSubMenu (hmenu, 1), LANG_MENU, TRUE, L"%s (Language)", _r_locale_getstring (IDS_LANGUAGE));

				_r_menu_setitemtext (hmenu, IDM_WEBSITE, FALSE, _r_locale_getstring (IDS_WEBSITE));
				_r_menu_setitemtext (hmenu, IDM_CHECKUPDATES, FALSE, _r_locale_getstring (IDS_CHECKUPDATES));

				_r_menu_setitemtextformat (hmenu, IDM_ABOUT, FALSE, L"%s\tF1", _r_locale_getstring (IDS_ABOUT));

				_r_locale_enum (GetSubMenu (hmenu, 1), LANG_MENU, IDX_LANGUAGE); // enum localizations
			}

			_r_ctrl_setstringformat (hwnd, IDC_CODE, L"%s:", _r_locale_getstring (IDS_CODE));
			_r_ctrl_setstringformat (hwnd, IDC_DESCRIPTION, L"%s:", _r_locale_getstring (IDS_DESCRIPTION));

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
					LPNMITEMACTIVATE lpnmlv;
					ULONG_PTR module_hash;

					if (lphdr->idFrom != IDC_LISTVIEW)
						break;

					lpnmlv = (LPNMITEMACTIVATE)lparam;

					if (lpnmlv->iItem == -1)
						break;

					module_hash = _r_listview_getitemlparam (hwnd, IDC_LISTVIEW, lpnmlv->iItem);

					_app_moduleopendirectory (module_hash);

					break;
				}

				case NM_CLICK:
				case LVN_ITEMCHANGED:
				{
					LPNMITEMACTIVATE lpnmlv;
					ULONG_PTR module_hash;

					if (lphdr->idFrom != IDC_LISTVIEW)
						break;

					lpnmlv = (LPNMITEMACTIVATE)lparam;

					if (lpnmlv->iItem != -1)
					{
						module_hash = _r_listview_getitemlparam (hwnd, IDC_LISTVIEW, lpnmlv->iItem);

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
					LPNMLVGETINFOTIP lpnmlv;
					ULONG_PTR module_hash;

					if (lphdr->idFrom != IDC_LISTVIEW)
						break;

					lpnmlv = (LPNMLVGETINFOTIP)lparam;

					module_hash = _r_listview_getitemlparam (hwnd, IDC_LISTVIEW, lpnmlv->iItem);

					_app_modulegettooltip (lpnmlv->pszText, lpnmlv->cchTextMax, module_hash);

					break;
				}

				case LVN_COLUMNCLICK:
				{
					LPNMLISTVIEW lpnmlv;
					INT ctrl_id;

					if (lphdr->idFrom != IDC_LISTVIEW)
						break;

					lpnmlv = (LPNMLISTVIEW)lparam;
					ctrl_id = (INT)(INT_PTR)lphdr->idFrom;

					_app_listviewsort (hwnd, ctrl_id, lpnmlv->iSubItem, TRUE);

					break;
				}

				case UDN_DELTAPOS:
				{
					LPNMUPDOWN lpnmud;
					ULONG code;
					ULONG base;

					if (lphdr->idFrom != IDC_CODE_UD)
						break;

					lpnmud = (LPNMUPDOWN)lparam;

					code = (ULONG)_r_ctrl_getinteger (hwnd, IDC_CODE_CTL, &base);

					_r_ctrl_setstringformat (hwnd, IDC_CODE_CTL, base != 10 ? FORMAT_HEX : FORMAT_DEC, code + lpnmud->iDelta);

					_app_print (hwnd);

					return TRUE;
				}
			}

			break;
		}

		case WM_SIZE:
		{
			LONG width;
			LONG dpi_value;

			if (!_r_layout_resize (&layout_manager, wparam))
				break;

			// resize statusbar parts
			width = _r_ctrl_getwidth (hwnd, IDC_LISTVIEW);

			if (!width)
				break;

			dpi_value = _r_dc_getwindowdpi (hwnd);

			INT parts[] = {width + _r_dc_getdpi (24, dpi_value), -1};

			_r_status_setparts (hwnd, IDC_STATUSBAR, parts, RTL_NUMBER_OF (parts));

			// resize column width
			_r_listview_setcolumn (hwnd, IDC_LISTVIEW, 0, NULL, width);

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

			if (notify_code == 0 && ctrl_id >= IDX_LANGUAGE && ctrl_id <= IDX_LANGUAGE + (INT)(INT_PTR)_r_locale_getcount () + 1)
			{
				HMENU hmenu;
				HMENU hsubmenu;

				hmenu = GetMenu (hwnd);

				if (hmenu)
				{
					hsubmenu = GetSubMenu (GetSubMenu (hmenu, 1), LANG_MENU);

					if (hsubmenu)
						_r_locale_apply (hsubmenu, ctrl_id, IDX_LANGUAGE);
				}

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
					BOOLEAN new_val = !_r_config_getboolean (L"AlwaysOnTop", FALSE);

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
					BOOLEAN new_val = !_r_update_isenabled (FALSE);

					_r_menu_checkitem (GetMenu (hwnd), ctrl_id, 0, MF_BYCOMMAND, new_val);
					_r_update_enable (new_val);

					break;
				}

				case IDM_WEBSITE:
				{
					_r_shell_opendefault (_r_app_getwebsite_url ());
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

INT APIENTRY wWinMain (
	_In_ HINSTANCE hinst,
	_In_opt_ HINSTANCE prev_hinst,
	_In_ LPWSTR cmdline,
	_In_ INT show_cmd
)
{
	HWND hwnd;

	if (!_r_app_initialize ())
		return ERROR_APP_INIT_FAILURE;

	hwnd = _r_app_createwindow (hinst, MAKEINTRESOURCE (IDD_MAIN), MAKEINTRESOURCE (IDI_MAIN), &DlgProc);

	if (!hwnd)
		return ERROR_APP_INIT_FAILURE;

	return _r_wnd_messageloop (hwnd, MAKEINTRESOURCE (IDA_MAIN));
}
