// Error Lookup
// Copyright (c) 2011-2021 Henry++

#include "routine.h"

#include "main.h"
#include "rapp.h"

#include "..\..\mxml\mxml.h"

#include "resource.h"

PR_ARRAY modules = NULL;

PR_HASHTABLE facility = NULL;
PR_HASHTABLE severity = NULL;

LCID lcid = 0;
SIZE_T count_unload = 0;
WCHAR info[MAX_PATH];

VOID NTAPI _app_dereferencemoduleprocedure (PVOID entry)
{
	PITEM_MODULE ptr_item = entry;

	SAFE_DELETE_REFERENCE (ptr_item->description);
	SAFE_DELETE_REFERENCE (ptr_item->path);

	SAFE_DELETE_LIBRARY (ptr_item->hlib);
}

ULONG _app_getcode (HWND hwnd, PBOOLEAN is_hex)
{
	ULONG result = 0;
	PR_STRING text;

	if (is_hex)
		*is_hex = FALSE;

	text = _r_ctrl_gettext (hwnd, IDC_CODE_CTL);

	if (text)
	{
		if ((result = _r_str_toulongex (_r_obj_getstring (text), 10)) == 0)
		{
			result = _r_str_toulongex (_r_obj_getstring (text), 16);

			if (is_hex)
				*is_hex = TRUE;
		}

		_r_obj_dereference (text);
	}

	return result;
}

PR_STRING _app_formatmessage (ULONG code, HINSTANCE hinstance, ULONG langId)
{
	if (!hinstance)
		return NULL;

	ULONG allocated_length;
	PR_STRING buffer;
	ULONG attempts;
	ULONG chars;
	ULONG err_code;

	attempts = 6;
	allocated_length = 0x400;
	buffer = _r_obj_createstringex (NULL, allocated_length * sizeof (WCHAR));

	do
	{
		chars = FormatMessage (FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS, hinstance, code, langId, buffer->buffer, allocated_length, NULL);

		if (!chars)
		{
			err_code = GetLastError ();

			if (err_code == ERROR_INSUFFICIENT_BUFFER)
			{
				_r_obj_dereference (buffer);

				allocated_length *= 2;
				buffer = _r_obj_createstringex (NULL, allocated_length * sizeof (WCHAR));
			}
			else if (err_code == ERROR_RESOURCE_LANG_NOT_FOUND)
			{
				_r_obj_dereference (buffer);

				if (langId)
					return _app_formatmessage (code, hinstance, 0);

				return NULL;
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
	}
	while (attempts--);

	if (!chars || buffer->buffer[0] == L'%')
	{
		_r_obj_dereference (buffer);
		return NULL;
	}

	_r_obj_trimstring (buffer, L"\r\n ");

	return buffer;
}

VOID _app_showdescription (HWND hwnd, SIZE_T idx)
{
	PITEM_MODULE ptr_module;

	if (idx != SIZE_MAX)
	{
		ptr_module = _r_obj_getarrayitem (modules, idx);

		if (ptr_module)
		{
			_r_ctrl_settextformat (hwnd, IDC_DESCRIPTION_CTL, L"%s\r\n\r\n%s", info, _r_obj_getstringorempty (ptr_module->text));

			_r_status_settextformat (hwnd, IDC_STATUSBAR, 1, L"%s - %s", _r_obj_getstringorempty (ptr_module->description), _r_obj_getstringorempty (ptr_module->path));
		}
	}
	else
	{
		SetDlgItemText (hwnd, IDC_DESCRIPTION_CTL, info);

		_r_status_settext (hwnd, IDC_STATUSBAR, 1, NULL);
	}
}

VOID _app_print (HWND hwnd)
{
	PITEM_MODULE ptr_module;
	PR_KEYSTORE facility_table;
	PR_KEYSTORE severity_table;
	PR_STRING buffer;
	ULONG error_code;
	ULONG severity_code;
	ULONG facility_code;
	INT item_count = 0;

	error_code = _app_getcode (hwnd, NULL);

	severity_code = HRESULT_SEVERITY (error_code);
	facility_code = HRESULT_FACILITY (error_code);

	facility_table = _r_obj_findhashtable (facility, facility_code);
	severity_table = _r_obj_findhashtable (severity, severity_code);

	// clear first
	_r_listview_deleteallitems (hwnd, IDC_LISTVIEW);

	// print information
	_r_str_printf (info, RTL_NUMBER_OF (info),
		L"Code (dec.): " FORMAT_DEC L"\r\nCode (hex.): " FORMAT_HEX L"\r\nFacility: %s (0x%02" TEXT (PRIX32) L")\r\nSeverity: %s (0x%02" TEXT (PRIX32) L")",
		error_code,
		error_code,
		facility_table ? _r_obj_getstringorempty (facility_table->value_string) : L"n/a",
		facility_code,
		severity_table ? _r_obj_getstringorempty (severity_table->value_string) : L"n/a",
		severity_code
	);

	// print modules
	for (SIZE_T i = 0; i < _r_obj_getarraysize (modules); i++)
	{
		ptr_module = _r_obj_getarrayitem (modules, i);

		if (!ptr_module || !ptr_module->hlib || !_r_config_getbooleanex (_r_obj_getstring (ptr_module->path), TRUE, SECTION_MODULE))
			continue;

		buffer = _app_formatmessage (error_code, ptr_module->hlib, lcid);
		_r_obj_movereference (&ptr_module->text, buffer);

		if (buffer)
		{
			_r_listview_additemex (hwnd, IDC_LISTVIEW, item_count, 0, _r_obj_getstring (ptr_module->description), I_IMAGENONE, I_GROUPIDNONE, i);
			item_count += 1;
		}
		else
		{
			SetDlgItemText (hwnd, IDC_DESCRIPTION_CTL, info);
		}
	}

	_r_listview_setcolumn (hwnd, IDC_LISTVIEW, 0, NULL, -100);

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

VOID _app_loaddatabase (HWND hwnd)
{
	PVOID buffer = _r_loadresource (NULL, MAKEINTRESOURCE (1), RT_RCDATA, NULL);

	HMENU hmenu = GetSubMenu (GetSubMenu (GetMenu (hwnd), 1), MODULES_MENU);
	DeleteMenu (hmenu, 0, MF_BYPOSITION); // delete separator

	mxml_node_t *xml_node = NULL;
	mxml_node_t *root_node;
	mxml_node_t *items_node;

	PR_HASHTABLE temporary_facility_table = _r_obj_createhashtableex (sizeof (R_KEYSTORE), 64, &_r_util_dereferencekeystoreprocedure);
	PR_HASHTABLE temporary_severity_table = _r_obj_createhashtableex (sizeof (R_KEYSTORE), 64, &_r_util_dereferencekeystoreprocedure);

	WCHAR database_path[MAX_PATH];
	_r_str_printf (database_path, RTL_NUMBER_OF (database_path), L"%s\\modules.xml", _r_app_getdirectory ());

	if (_r_fs_exists (database_path))
	{
		HANDLE hfile = CreateFile (database_path, FILE_GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (_r_fs_isvalidhandle (hfile))
		{
			xml_node = mxmlLoadFd (NULL, hfile, MXML_OPAQUE_CALLBACK);
			CloseHandle (hfile);
		}
	}

	if (!xml_node && buffer)
		xml_node = mxmlLoadString (NULL, buffer, MXML_OPAQUE_CALLBACK);

	if (xml_node)
	{
		root_node = mxmlFindElement (xml_node, xml_node, "root", NULL, NULL, MXML_DESCEND);

		if (root_node)
		{
			// load modules information
			MENUITEMINFO mii;
			R_KEYSTORE keystore;
			UINT index;
			LPCSTR text;
			BOOLEAN is_enabled;

			_r_obj_cleararray (modules);

			items_node = mxmlFindElement (root_node, root_node, "module", NULL, NULL, MXML_DESCEND);

			if (items_node)
			{
				index = 0;

				ITEM_MODULE module;
				ULONG load_flags = LOAD_LIBRARY_AS_DATAFILE;

				if (_r_sys_isosversiongreaterorequal (WINDOWS_VISTA))
					load_flags |= LOAD_LIBRARY_AS_IMAGE_RESOURCE;

				for (mxml_node_t* item = mxmlGetFirstChild (items_node); item; item = mxmlGetNextSibling (item))
				{
					text = mxmlElementGetAttr (item, "file");

					if (_r_str_isempty_a (text))
						continue;

					memset (&module, 0, sizeof (module));

					module.path = _r_str_multibyte2unicode (text);
					module.description = _r_str_multibyte2unicode (mxmlElementGetAttr (item, "text"));

					is_enabled = _r_config_getbooleanex (_r_obj_getstring (module.path), TRUE, SECTION_MODULE);

					if (is_enabled)
						module.hlib = LoadLibraryEx (_r_obj_getstring (module.path), NULL, load_flags);

					if (!is_enabled || !module.hlib)
						count_unload += 1;

					// insert menu
					memset (&mii, 0, sizeof (mii));

					mii.cbSize = sizeof (mii);
					mii.fMask = MIIM_ID | MIIM_STATE | MIIM_STRING;
					mii.fType = MFT_STRING;
					mii.dwTypeData = (LPWSTR)_r_obj_getstring (module.description);
					mii.fState = is_enabled ? MFS_CHECKED : MF_UNCHECKED;
					mii.wID = IDX_MODULES + index;

					if (!module.hlib && is_enabled)
						mii.fState |= MFS_DISABLED | MF_GRAYED;

					InsertMenuItem (hmenu, mii.wID, FALSE, &mii);

					_r_obj_addarrayitem (modules, &module);

					index += 1;
				}
			}

			// load facility information
			items_node = mxmlFindElement (root_node, root_node, "facility", NULL, NULL, MXML_DESCEND);

			if (items_node)
			{
				for (mxml_node_t* item = mxmlGetFirstChild (items_node); item; item = mxmlGetNextSibling (item))
				{
					text = mxmlElementGetAttr (item, "text");

					if (_r_str_isempty_a (text))
						continue;

					keystore.value_string = _r_str_multibyte2unicode (text);

					if (!keystore.value_string)
						continue;

					_r_obj_addhashtableitem (temporary_facility_table, _r_str_toulong_a (mxmlElementGetAttr (item, "code")), &keystore);
				}
			}

			// load severity information
			items_node = mxmlFindElement (root_node, root_node, "severity", NULL, NULL, MXML_DESCEND);

			if (items_node)
			{
				for (mxml_node_t* item = mxmlGetFirstChild (items_node); item; item = mxmlGetNextSibling (item))
				{
					text = mxmlElementGetAttr (item, "text");

					if (_r_str_isempty_a (text))
						continue;

					keystore.value_string = _r_str_multibyte2unicode (text);

					if (!keystore.value_string)
						continue;

					_r_obj_addhashtableitem (temporary_severity_table, _r_str_toulong_a (mxmlElementGetAttr (item, "code")), &keystore);
				}
			}
		}

		mxmlDelete (xml_node);
	}

	_r_obj_movereference (&facility, temporary_facility_table);
	_r_obj_movereference (&severity, temporary_severity_table);

	if (_r_obj_isarrayempty (modules))
	{
		AppendMenu (hmenu, MF_STRING, IDX_MODULES, _r_locale_getstring (IDS_STATUS_EMPTY2));

		_r_menu_enableitem (hmenu, IDX_MODULES, MF_BYCOMMAND, FALSE);
	}

	_r_status_settextformat (hwnd, IDC_STATUSBAR, 0, _r_locale_getstring (IDS_STATUS_TOTAL), _r_obj_getarraysize (modules) - count_unload, _r_obj_getarraysize (modules));
}

VOID _app_resizewindow (HWND hwnd, LPARAM lparam)
{
	RECT rect = {0};
	SendDlgItemMessage (hwnd, IDC_STATUSBAR, SB_GETRECT, 0, (LPARAM)&rect);

	INT statusbar_height = _r_calc_rectheight (&rect);

	GetWindowRect (GetDlgItem (hwnd, IDC_LISTVIEW), &rect);
	INT listview_width = _r_calc_rectwidth (&rect);

	GetClientRect (GetDlgItem (hwnd, IDC_LISTVIEW), &rect);
	INT listview_height = (HIWORD (lparam) - (rect.top - rect.bottom) - statusbar_height) - _r_dc_getdpi (hwnd, 80);
	listview_height -= _r_calc_rectheight (&rect);

	GetClientRect (GetDlgItem (hwnd, IDC_DESCRIPTION_CTL), &rect);
	INT edit_width = (LOWORD (lparam) - listview_width) - _r_dc_getdpi (hwnd, 36);
	INT edit_height = (HIWORD (lparam) - (rect.top - rect.bottom) - statusbar_height) - _r_dc_getdpi (hwnd, 42);
	edit_height -= _r_calc_rectheight (&rect);

	HDWP hwdp = BeginDeferWindowPos (3);

	hwdp = DeferWindowPos (hwdp, GetDlgItem (hwnd, IDC_LISTVIEW), NULL, 0, 0, listview_width, listview_height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
	hwdp = DeferWindowPos (hwdp, GetDlgItem (hwnd, IDC_DESCRIPTION), NULL, 0, 0, edit_width, _r_dc_getdpi (hwnd, 14), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER);
	hwdp = DeferWindowPos (hwdp, GetDlgItem (hwnd, IDC_DESCRIPTION_CTL), NULL, 0, 0, edit_width, edit_height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER);

	EndDeferWindowPos (hwdp);

	// resize statusbar parts
	INT parts[] = {listview_width + _r_dc_getdpi (hwnd, 24), -1};
	SendDlgItemMessage (hwnd, IDC_STATUSBAR, SB_SETPARTS, 2, (LPARAM)parts);

	// resize column width
	_r_listview_setcolumn (hwnd, IDC_LISTVIEW, 0, NULL, -100);

	// resize statusbar
	SendDlgItemMessage (hwnd, IDC_STATUSBAR, WM_SIZE, 0, 0);
}

INT_PTR CALLBACK DlgProc (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			modules = _r_obj_createarrayex (sizeof (ITEM_MODULE), 64, &_app_dereferencemoduleprocedure);

			// configure listview
			_r_listview_setstyle (hwnd, IDC_LISTVIEW, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP, FALSE);

			_r_listview_addcolumn (hwnd, IDC_LISTVIEW, 0, _r_locale_getstring (IDS_MODULES), -95, LVCFMT_LEFT);

			// configure controls
			SendDlgItemMessage (hwnd, IDC_CODE_UD, UDM_SETRANGE32, 0, INT32_MAX);

			// configure menu
			HMENU hmenu = GetMenu (hwnd);

			if (hmenu)
			{
				_r_menu_checkitem (hmenu, IDM_ALWAYSONTOP_CHK, 0, MF_BYCOMMAND, _r_config_getboolean (L"AlwaysOnTop", APP_ALWAYSONTOP));
				_r_menu_checkitem (hmenu, IDM_INSERTBUFFER_CHK, 0, MF_BYCOMMAND, _r_config_getboolean (L"InsertBufferAtStartup", FALSE));
				_r_menu_checkitem (hmenu, IDM_CLASSICUI_CHK, 0, MF_BYCOMMAND, _r_config_getboolean (L"ClassicUI", APP_CLASSICUI));
				_r_menu_checkitem (hmenu, IDM_CHECKUPDATES_CHK, 0, MF_BYCOMMAND, _r_config_getboolean (L"CheckUpdates", TRUE));
			}

			// load xml database
			_app_loaddatabase (hwnd);

			if (_r_config_getboolean (L"InsertBufferAtStartup", FALSE))
			{
				PR_STRING clipboard_text = _r_clipboard_get (hwnd);

				if (clipboard_text)
				{
					SetDlgItemText (hwnd, IDC_CODE_CTL, _r_obj_getstring (clipboard_text));

					_r_obj_dereference (clipboard_text);
				}
			}
			else
			{
				SetDlgItemText (hwnd, IDC_CODE_CTL, _r_config_getstring (L"LatestCode", L"0x00000000"));
			}

			break;
		}

		case WM_NCCREATE:
		{
			_r_wnd_enablenonclientscaling (hwnd);
			break;
		}

		case WM_DESTROY:
		{
			PR_STRING window_text = _r_ctrl_gettext (hwnd, IDC_CODE_CTL);

			if (window_text)
			{
				_r_config_setstring (L"LatestCode", _r_obj_getstring (window_text));

				_r_obj_dereference (window_text);
			}

			PostQuitMessage (0);

			break;
		}

		case RM_LOCALIZE:
		{
			// get locale id
			lcid = _r_str_toulongex (_r_locale_getstring (IDS_LCID), 16);

			_r_listview_setcolumn (hwnd, IDC_LISTVIEW, 0, _r_locale_getstring (IDS_MODULES), 0);

			// localize
			HMENU hmenu = GetMenu (hwnd);

			if (hmenu)
			{
				_r_menu_setitemtext (hmenu, 0, TRUE, _r_locale_getstring (IDS_FILE));
				_r_menu_setitemtext (hmenu, 1, TRUE, _r_locale_getstring (IDS_SETTINGS));
				_r_menu_setitemtext (hmenu, 2, TRUE, _r_locale_getstring (IDS_HELP));

				_r_menu_setitemtextformat (hmenu, IDM_EXIT, FALSE, L"%s\tEsc", _r_locale_getstring (IDS_EXIT));
				_r_menu_setitemtext (hmenu, IDM_ALWAYSONTOP_CHK, FALSE, _r_locale_getstring (IDS_ALWAYSONTOP_CHK));
				_r_menu_setitemtext (hmenu, IDM_INSERTBUFFER_CHK, FALSE, _r_locale_getstring (IDS_INSERTBUFFER_CHK));
				_r_menu_setitemtextformat (hmenu, IDM_CLASSICUI_CHK, FALSE, L"%s*", _r_locale_getstring (IDS_CLASSICUI_CHK));
				_r_menu_setitemtext (hmenu, IDM_CHECKUPDATES_CHK, FALSE, _r_locale_getstring (IDS_CHECKUPDATES_CHK));
				_r_menu_setitemtext (GetSubMenu (hmenu, 1), MODULES_MENU, TRUE, _r_locale_getstring (IDS_MODULES));
				_r_menu_setitemtextformat (GetSubMenu (hmenu, 1), LANG_MENU, TRUE, L"%s (Language)", _r_locale_getstring (IDS_LANGUAGE));

				_r_menu_setitemtext (hmenu, IDM_WEBSITE, FALSE, _r_locale_getstring (IDS_WEBSITE));
				_r_menu_setitemtext (hmenu, IDM_CHECKUPDATES, FALSE, _r_locale_getstring (IDS_CHECKUPDATES));

				_r_menu_setitemtextformat (hmenu, IDM_ABOUT, FALSE, L"%s\tF1", _r_locale_getstring (IDS_ABOUT));

				_r_locale_enum ((HWND)GetSubMenu (hmenu, 1), LANG_MENU, IDX_LANGUAGE); // enum localizations
			}

			SetDlgItemText (hwnd, IDC_CODE, _r_locale_getstring (IDS_CODE));
			SetDlgItemText (hwnd, IDC_DESCRIPTION, _r_locale_getstring (IDS_DESCRIPTION));

			_app_print (hwnd);

			_r_status_settextformat (hwnd, IDC_STATUSBAR, 0, _r_locale_getstring (IDS_STATUS_TOTAL), _r_obj_getarraysize (modules) - count_unload, _r_obj_getarraysize (modules));

			SendDlgItemMessage (hwnd, IDC_LISTVIEW, LVM_RESETEMPTYTEXT, 0, 0);

			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR lphdr = (LPNMHDR)lparam;

			switch (lphdr->code)
			{
				case NM_CLICK:
				case LVN_ITEMCHANGED:
				{
					if (lphdr->idFrom == IDC_LISTVIEW)
					{
						LPNMITEMACTIVATE lpnm = (LPNMITEMACTIVATE)lparam;

						if (lpnm->iItem != -1)
						{
							SIZE_T idx = _r_listview_getitemlparam (hwnd, IDC_LISTVIEW, lpnm->iItem);

							_app_showdescription (hwnd, idx);
						}
						else
						{
							_app_showdescription (hwnd, SIZE_MAX);
						}
					}

					break;
				}

				case LVN_GETINFOTIP:
				{
					LPNMLVGETINFOTIP lpnmlv = (LPNMLVGETINFOTIP)lparam;

					PITEM_MODULE ptr_module = _r_obj_getarrayitem (modules, _r_listview_getitemlparam (hwnd, IDC_LISTVIEW, lpnmlv->iItem));

					if (ptr_module)
						_r_str_printf (lpnmlv->pszText, lpnmlv->cchTextMax, L"%s\r\n%s", _r_obj_getstringorempty (ptr_module->description), _r_obj_getstringorempty (ptr_module->path));

					break;
				}

				case LVN_GETEMPTYMARKUP:
				{
					NMLVEMPTYMARKUP* lpnmlv = (NMLVEMPTYMARKUP*)lparam;

					lpnmlv->dwFlags = EMF_CENTERED;
					_r_str_copy (lpnmlv->szMarkup, RTL_NUMBER_OF (lpnmlv->szMarkup), _r_locale_getstring (IDS_STATUS_EMPTY));

					SetWindowLongPtr (hwnd, DWLP_MSGRESULT, TRUE);
					return TRUE;
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
			_app_resizewindow (hwnd, lparam);
			break;
		}

		case WM_COMMAND:
		{
			INT ctrl_id = LOWORD (wparam);
			INT notify_code = HIWORD (wparam);

			if (ctrl_id == IDC_CODE_CTL && notify_code == EN_CHANGE)
			{
				_app_print (hwnd);
				return FALSE;
			}

			if (notify_code == 0 && ctrl_id >= IDX_LANGUAGE && ctrl_id <= IDX_LANGUAGE + (INT)_r_locale_getcount ())
			{
				_r_locale_applyfrommenu (GetSubMenu (GetSubMenu (GetMenu (hwnd), 1), LANG_MENU), ctrl_id);
				return FALSE;
			}
			else if ((ctrl_id >= IDX_MODULES && ctrl_id <= IDX_MODULES + (INT)_r_obj_getarraysize (modules)))
			{
				SIZE_T idx = (SIZE_T)ctrl_id - IDX_MODULES;
				PITEM_MODULE ptr_module = _r_obj_getarrayitem (modules, idx);

				if (!ptr_module)
					return FALSE;

				LPCWSTR path = _r_obj_getstring (ptr_module->path);
				BOOLEAN is_enabled = !_r_config_getbooleanex (path, TRUE, SECTION_MODULE);

				_r_menu_checkitem (GetMenu (hwnd), IDX_MODULES + (ctrl_id - IDX_MODULES), 0, MF_BYCOMMAND, is_enabled);

				_r_config_setbooleanex (path, is_enabled, SECTION_MODULE);

				if (is_enabled)
				{
					ULONG load_flags = _r_sys_isosversiongreaterorequal (WINDOWS_VISTA) ? LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE : LOAD_LIBRARY_AS_DATAFILE;

					ptr_module->hlib = LoadLibraryEx (path, NULL, load_flags);

					if (ptr_module->hlib)
						count_unload -= 1;
				}
				else
				{
					SAFE_DELETE_REFERENCE (ptr_module->text);
					SAFE_DELETE_LIBRARY (ptr_module->hlib);

					count_unload += 1;
				}

				_r_status_settextformat (hwnd, IDC_STATUSBAR, 0, _r_locale_getstring (IDS_STATUS_TOTAL), _r_obj_getarraysize (modules) - count_unload, _r_obj_getarraysize (modules));

				_app_print (hwnd);

				return FALSE;
			}

			switch (ctrl_id)
			{
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
					ShellExecute (hwnd, NULL, APP_WEBSITE_URL, NULL, NULL, SW_SHOWDEFAULT);
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

	if (_r_app_initialize (APP_NAME, APP_NAME_SHORT, APP_VERSION, APP_COPYRIGHT))
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
