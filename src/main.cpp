// Error Lookup
// Copyright (c) 2011-2019 Henry++

#include <windows.h>

#include "main.hpp"
#include "rapp.hpp"
#include "routine.hpp"

#include "pugiconfig.hpp"
#include "..\..\pugixml\src\pugixml.hpp"

#include "resource.hpp"

rapp app;

std::vector<PITEM_MODULE> modules;

std::unordered_map<DWORD, LPWSTR> facility;
std::unordered_map<DWORD, LPWSTR> severity;

LCID lcid = 0;

size_t count_unload = 0;

WCHAR info[MAX_PATH] = {0};

DWORD _app_getcode (HWND hwnd, bool* is_hex)
{
	rstring buffer = _r_ctrl_gettext (hwnd, IDC_CODE_CTL);
	DWORD result = 0;

	if (!buffer.IsEmpty ())
	{
		if ((result = buffer.AsUlong (10)) == 0 && (buffer.GetLength () != 1 || buffer.At (0) != L'0'))
		{
			result = buffer.AsUlong (16);

			if (is_hex)
				*is_hex = true;
		}
	}

	return result;
}

rstring _app_formatmessage (DWORD code, HINSTANCE hinstance, BOOL is_localized = true)
{
	rstring result;

	if (hinstance)
	{
		HLOCAL buffer = nullptr;

		if (FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS, hinstance, code, is_localized ? lcid : 0, (LPWSTR)&buffer, 0, nullptr))
		{
			result = (LPCWSTR)buffer;

			if (result[0] == L'%')
				result.Release (); // clear
		}
		else
		{
			if (is_localized)
				result = _app_formatmessage (code, hinstance, false);
		}

		SAFE_LOCAL_FREE (buffer);

		_r_str_trim (result, L"\r\n ");
	}

	return result;
}

void _app_showdescription (HWND hwnd, size_t idx)
{
	if (idx != INVALID_SIZE_T)
	{
		PITEM_MODULE ptr_module = modules.at (idx);

		if (ptr_module)
		{
			_r_ctrl_settext (hwnd, IDC_DESCRIPTION_CTL, L"%s\r\n\r\n%s", info, ptr_module->text);

			_r_status_settext (hwnd, IDC_STATUSBAR, 1, _r_fmt (L"%s - %s", ptr_module->description, ptr_module->path));
		}
	}
	else
	{
		SetDlgItemText (hwnd, IDC_DESCRIPTION_CTL, info);
		_r_status_settext (hwnd, IDC_STATUSBAR, 1, nullptr);
	}
}

void _app_print (HWND hwnd)
{
	const DWORD code = _app_getcode (hwnd, nullptr);

	const DWORD severity_code = HRESULT_SEVERITY (code);
	const DWORD facility_code = HRESULT_FACILITY (code);

	rstring buffer;

	// clear first
	_r_listview_deleteallitems (hwnd, IDC_LISTVIEW);

	// print information
	_r_str_printf (info, _countof (info), L"Code (dec.): " FORMAT_DEC L"\r\nCode (hex.): " FORMAT_HEX L"\r\nSeverity: %s (0x%02" PRIX32 L")\r\nFacility: %s (0x%02" PRIX32 L")", code, code, (severity.find (severity_code) != severity.end ()) ? severity[severity_code] : L"n/a", severity_code, (facility.find (facility_code) != facility.end ()) ? facility[facility_code] : L"n/a", facility_code);

	// print modules
	INT item_count = 0;

	for (size_t i = 0; i < modules.size (); i++)
	{
		PITEM_MODULE ptr_module = modules.at (i);

		if (!ptr_module || !ptr_module->hlib || !app.ConfigGet (ptr_module->path, true, SECTION_MODULE).AsBool ())
			continue;

		buffer = _app_formatmessage (code, ptr_module->hlib);

		if (!buffer.IsEmpty ())
		{
			_r_str_alloc (&ptr_module->text, buffer.GetLength (), buffer);

			_r_listview_additem (hwnd, IDC_LISTVIEW, item_count, 0, ptr_module->description, I_IMAGENONE, I_GROUPIDNONE, i);
			item_count += 1;
		}
		else
		{
			SetDlgItemText (hwnd, IDC_DESCRIPTION_CTL, info);
		}
	}

	_r_listview_setcolumn (hwnd, IDC_LISTVIEW, 0, nullptr, -100);

	// show description for first item
	if (!item_count)
	{
		_app_showdescription (hwnd, INVALID_SIZE_T);
	}
	else
	{
		ListView_SetItemState (GetDlgItem (hwnd, IDC_LISTVIEW), 0, LVIS_ACTIVATING, LVIS_ACTIVATING); // select item
	}
}

void _app_loaddatabase (HWND hwnd)
{
	DWORD rc_length = 0;
	LPVOID pres = _r_loadresource (app.GetHINSTANCE (), MAKEINTRESOURCE (1), RT_RCDATA, &rc_length);

	const HMENU hmenu = GetSubMenu (GetSubMenu (GetMenu (hwnd), 1), MODULES_MENU);
	DeleteMenu (hmenu, 0, MF_BYPOSITION); // delete separator

	pugi::xml_document doc;
	pugi::xml_parse_result result;

	WCHAR database_path[MAX_PATH] = {0};
	_r_str_printf (database_path, _countof (database_path), L"%s\\modules.xml", app.GetDirectory ());

	if (_r_fs_exists (database_path))
		result = doc.load_file (database_path, PUGIXML_LOAD_FLAGS, PUGIXML_LOAD_ENCODING);

	if (result.status != pugi::status_ok && pres)
		result = doc.load_buffer (pres, rc_length, PUGIXML_LOAD_FLAGS, PUGIXML_LOAD_ENCODING);

	if (result.status == pugi::status_ok)
	{
		pugi::xml_node root = doc.child (L"root");

		if (root)
		{
			pugi::xml_node sub_root;

			// load modules information
			{
				for (auto &p : modules)
					SAFE_DELETE (p);

				modules.clear ();

				sub_root = root.child (L"module");

				if (sub_root)
				{
					UINT i = 0;

					const DWORD load_flags = _r_sys_validversion (6, 0) ? LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE : LOAD_LIBRARY_AS_DATAFILE;

					for (pugi::xml_node item = sub_root.child (L"item"); item; item = item.next_sibling (L"item"))
					{
						PITEM_MODULE ptr_module = new ITEM_MODULE;

						const rstring path = item.attribute (L"file").as_string ();
						const rstring description = !item.attribute (L"text").empty () ? item.attribute (L"text").as_string () : path;

						_r_str_alloc (&ptr_module->path, path.GetLength (), path);
						_r_str_alloc (&ptr_module->description, description.GetLength (), description);

						const bool is_enabled = app.ConfigGet (ptr_module->path, true, SECTION_MODULE).AsBool ();

						if (is_enabled)
							ptr_module->hlib = LoadLibraryEx (ptr_module->path, nullptr, load_flags);

						if (!is_enabled || !ptr_module->hlib)
							count_unload += 1;

						{
							MENUITEMINFO mii = {0};

							mii.cbSize = sizeof (mii);
							mii.fMask = MIIM_ID | MIIM_STATE | MIIM_STRING;
							mii.fType = MFT_STRING;
							mii.dwTypeData = ptr_module->description;
							mii.fState = is_enabled ? MFS_CHECKED : MF_UNCHECKED;
							mii.wID = IDX_MODULES + i;

							if (!ptr_module->hlib && is_enabled)
								mii.fState |= MFS_DISABLED | MF_GRAYED;

							InsertMenuItem (hmenu, IDX_MODULES + i, false, &mii);
						}

						modules.push_back (ptr_module);

						i += 1;
					}
				}
			}

			// load facility information
			{
				for (auto &p : facility)
					SAFE_DELETE_ARRAY (p.second);

				facility.clear ();

				sub_root = root.child (L"facility");

				if (sub_root)
				{
					for (pugi::xml_node item = sub_root.child (L"item"); item; item = item.next_sibling (L"item"))
					{
						const DWORD code = (DWORD)item.attribute (L"code").as_ullong ();

						// prevent duplicates
						if (facility.find (code) != facility.end ())
							continue;

						LPWSTR ptr_text = nullptr;

						_r_str_alloc (&ptr_text, INVALID_SIZE_T, item.attribute (L"text").as_string ());
						facility[code] = ptr_text;
					}
				}
			}

			// load severity information
			{
				for (auto &p : severity)
					SAFE_DELETE_ARRAY (p.second);

				severity.clear ();

				sub_root = root.child (L"severity");

				if (sub_root)
				{
					for (pugi::xml_node item = sub_root.child (L"item"); item; item = item.next_sibling (L"item"))
					{
						const DWORD code = (DWORD)item.attribute (L"code").as_ullong ();

						// prevent duplicates
						if (severity.find (code) != severity.end ())
							continue;

						LPWSTR ptr_text = nullptr;

						_r_str_alloc (&ptr_text, INVALID_SIZE_T, item.attribute (L"text").as_string ());
						severity[code] = ptr_text;
					}
				}
			}
		}
	}

	if (modules.empty ())
	{
		AppendMenu (hmenu, MF_STRING, IDX_MODULES, app.LocaleString (IDS_STATUS_EMPTY2, nullptr));
		EnableMenuItem (hmenu, IDX_MODULES, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	}

	_r_status_settext (hwnd, IDC_STATUSBAR, 0, _r_fmt (app.LocaleString (IDS_STATUS_TOTAL, nullptr), modules.size () - count_unload, modules.size ()));
}

void _app_resizewindow (HWND hwnd, LPARAM lparam)
{
	RECT rc = {0};
	SendDlgItemMessage (hwnd, IDC_STATUSBAR, SB_GETRECT, 0, (LPARAM)&rc);

	const INT statusbar_height = _R_RECT_HEIGHT (&rc);

	GetWindowRect (GetDlgItem (hwnd, IDC_LISTVIEW), &rc);

	const INT listview_width = _R_RECT_WIDTH (&rc);

	GetClientRect (GetDlgItem (hwnd, IDC_LISTVIEW), &rc);

	INT listview_height = (HIWORD (lparam) - (rc.top - rc.bottom) - statusbar_height) - _r_dc_getdpi (hwnd, 80);
	listview_height -= _R_RECT_HEIGHT (&rc);

	GetClientRect (GetDlgItem (hwnd, IDC_DESCRIPTION_CTL), &rc);

	const INT edit_width = (LOWORD (lparam) - listview_width) - _r_dc_getdpi (hwnd, 36);
	INT edit_height = (HIWORD (lparam) - (rc.top - rc.bottom) - statusbar_height) - _r_dc_getdpi (hwnd, 42);
	edit_height -= _R_RECT_HEIGHT (&rc);

	HDWP hwdp = BeginDeferWindowPos (3);

	_r_wnd_resize (&hwdp, GetDlgItem (hwnd, IDC_LISTVIEW), nullptr, 0, 0, listview_width, listview_height, SWP_NOMOVE);
	_r_wnd_resize (&hwdp, GetDlgItem (hwnd, IDC_DESCRIPTION), nullptr, 0, 0, edit_width, _r_dc_getdpi (hwnd, 14), SWP_NOMOVE);
	_r_wnd_resize (&hwdp, GetDlgItem (hwnd, IDC_DESCRIPTION_CTL), nullptr, 0, 0, edit_width, edit_height, SWP_NOMOVE);

	EndDeferWindowPos (hwdp);

	// resize statusbar parts
	INT parts[] = {listview_width + _r_dc_getdpi (hwnd, 24), -1};
	SendDlgItemMessage (hwnd, IDC_STATUSBAR, SB_SETPARTS, 2, (LPARAM)parts);

	// resize column width
	_r_listview_setcolumn (hwnd, IDC_LISTVIEW, 0, nullptr, -100);

	// resize statusbar
	SendDlgItemMessage (hwnd, IDC_STATUSBAR, WM_SIZE, 0, 0);
}

INT_PTR CALLBACK DlgProc (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
#ifndef _APP_NO_DARKTHEME
			_r_wnd_setdarktheme (hwnd);
#endif // _APP_NO_DARKTHEME

			// configure listview
			_r_listview_setstyle (hwnd, IDC_LISTVIEW, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);

			_r_listview_addcolumn (hwnd, IDC_LISTVIEW, 0, app.LocaleString (IDS_MODULES, nullptr), -95, LVCFMT_LEFT);

			// configure controls
			SendDlgItemMessage (hwnd, IDC_CODE_UD, UDM_SETRANGE32, 0, INT32_MAX);

			// configure menu
			HMENU hmenu = GetMenu (hwnd);

			CheckMenuItem (hmenu, IDM_ALWAYSONTOP_CHK, MF_BYCOMMAND | (app.ConfigGet (L"AlwaysOnTop", _APP_ALWAYSONTOP).AsBool () ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem (hmenu, IDM_INSERTBUFFER_CHK, MF_BYCOMMAND | (app.ConfigGet (L"InsertBufferAtStartup", false).AsBool () ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem (hmenu, IDM_CHECKUPDATES_CHK, MF_BYCOMMAND | (app.ConfigGet (L"CheckUpdates", true).AsBool () ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem (hmenu, IDM_CLASSICUI_CHK, MF_BYCOMMAND | (app.ConfigGet (L"ClassicUI", _APP_CLASSICUI).AsBool () ? MF_CHECKED : MF_UNCHECKED));

			// load xml database
			_app_loaddatabase (hwnd);

			if (app.ConfigGet (L"InsertBufferAtStartup", false).AsBool ())
			{
				SetDlgItemText (hwnd, IDC_CODE_CTL, _r_clipboard_get (hwnd));
			}
			else
			{
				SetDlgItemText (hwnd, IDC_CODE_CTL, app.ConfigGet (L"LatestCode", L"0x00000000"));
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
			app.ConfigSet (L"LatestCode", _r_ctrl_gettext (hwnd, IDC_CODE_CTL));

			PostQuitMessage (0);

			break;
		}

		case RM_LOCALIZE:
		{
			// get locale id
			lcid = app.LocaleString (IDS_LCID, nullptr).AsUlong (16);

			_r_listview_setcolumn (hwnd, IDC_LISTVIEW, 0, app.LocaleString (IDS_MODULES, nullptr), 0);

			// localize
			const HMENU hmenu = GetMenu (hwnd);

			app.LocaleMenu (hmenu, IDS_FILE, 0, true, nullptr);
			app.LocaleMenu (hmenu, IDS_EXIT, IDM_EXIT, false, L"\tEsc");

			app.LocaleMenu (hmenu, IDS_SETTINGS, 1, true, nullptr);
			app.LocaleMenu (hmenu, IDS_ALWAYSONTOP_CHK, IDM_ALWAYSONTOP_CHK, false, nullptr);
			app.LocaleMenu (hmenu, IDS_INSERTBUFFER_CHK, IDM_INSERTBUFFER_CHK, false, nullptr);
			app.LocaleMenu (hmenu, IDS_CHECKUPDATES_CHK, IDM_CHECKUPDATES_CHK, false, nullptr);
			app.LocaleMenu (hmenu, IDS_CLASSICUI_CHK, IDM_CLASSICUI_CHK, false, nullptr);
			app.LocaleMenu (GetSubMenu (hmenu, 1), IDS_MODULES, MODULES_MENU, true, nullptr);
			app.LocaleMenu (GetSubMenu (hmenu, 1), IDS_LANGUAGE, LANG_MENU, true, L" (Language)");

			app.LocaleMenu (hmenu, IDS_HELP, 2, true, nullptr);
			app.LocaleMenu (hmenu, IDS_WEBSITE, IDM_WEBSITE, false, nullptr);
			app.LocaleMenu (hmenu, IDS_CHECKUPDATES, IDM_CHECKUPDATES, false, nullptr);
			app.LocaleMenu (hmenu, IDS_ABOUT, IDM_ABOUT, false, L"\tF1");

			SetDlgItemText (hwnd, IDC_CODE, app.LocaleString (IDS_CODE, nullptr));
			SetDlgItemText (hwnd, IDC_DESCRIPTION, app.LocaleString (IDS_DESCRIPTION, nullptr));

			_app_print (hwnd);

			_r_status_settext (hwnd, IDC_STATUSBAR, 0, _r_fmt (app.LocaleString (IDS_STATUS_TOTAL, nullptr), modules.size () - count_unload, modules.size ()));

			app.LocaleEnum ((HWND)GetSubMenu (hmenu, 1), LANG_MENU, true, IDX_LANGUAGE); // enum localizations

			SendDlgItemMessage (hwnd, IDC_LISTVIEW, LVM_RESETEMPTYTEXT, 0, 0);

			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR hdr = (LPNMHDR)lparam;

			switch (hdr->code)
			{
				case NM_CLICK:
				case LVN_ITEMCHANGED:
				{
					if (hdr->idFrom == IDC_LISTVIEW)
					{
						LPNMITEMACTIVATE const lpnm = (LPNMITEMACTIVATE)lparam;

						if (lpnm->iItem != INVALID_INT)
						{
							const size_t idx = _r_listview_getitemlparam (hwnd, IDC_LISTVIEW, lpnm->iItem);

							_app_showdescription (hwnd, idx);
						}
						else
						{
							_app_showdescription (hwnd, INVALID_SIZE_T);
						}
					}

					break;
				}

				case LVN_GETINFOTIP:
				{
					LPNMLVGETINFOTIP lpnmlv = (LPNMLVGETINFOTIP)lparam;

					PITEM_MODULE ptr_module = modules.at (_r_listview_getitemlparam (hwnd, IDC_LISTVIEW, lpnmlv->iItem));

					if (ptr_module)
						_r_str_printf (lpnmlv->pszText, lpnmlv->cchTextMax, L"%s\r\n%s", ptr_module->description, ptr_module->path);

					break;
				}

				case LVN_GETEMPTYMARKUP:
				{
					NMLVEMPTYMARKUP* lpnmlv = (NMLVEMPTYMARKUP*)lparam;

					lpnmlv->dwFlags = EMF_CENTERED;
					_r_str_copy (lpnmlv->szMarkup, _countof (lpnmlv->szMarkup), app.LocaleString (IDS_STATUS_EMPTY, nullptr));

					SetWindowLongPtr (hwnd, DWLP_MSGRESULT, TRUE);
					return TRUE;
				}

				case UDN_DELTAPOS:
				{
					if (hdr->idFrom == IDC_CODE_UD)
					{
						bool is_hex = false;
						DWORD code = _app_getcode (hwnd, &is_hex);

						_r_ctrl_settext (hwnd, IDC_CODE_CTL, is_hex ? FORMAT_HEX : FORMAT_DEC, code + LPNMUPDOWN (lparam)->iDelta);
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
			if (LOWORD (wparam) == IDC_CODE_CTL && HIWORD (wparam) == EN_CHANGE)
			{
				_app_print (hwnd);
				return FALSE;
			}

			if (HIWORD (wparam) == 0 && LOWORD (wparam) >= IDX_LANGUAGE && LOWORD (wparam) <= IDX_LANGUAGE + app.LocaleGetCount ())
			{
				app.LocaleApplyFromMenu (GetSubMenu (GetSubMenu (GetMenu (hwnd), 1), LANG_MENU), LOWORD (wparam), IDX_LANGUAGE);
				return FALSE;
			}
			else if ((LOWORD (wparam) >= IDX_MODULES && LOWORD (wparam) <= IDX_MODULES + modules.size ()))
			{
				const size_t idx = LOWORD (wparam) - IDX_MODULES;
				PITEM_MODULE ptr_module = modules.at (idx);

				if (!ptr_module)
					return FALSE;

				const bool is_enabled = !app.ConfigGet (ptr_module->path, true, SECTION_MODULE).AsBool ();

				CheckMenuItem (GetMenu (hwnd), IDX_MODULES + (LOWORD (wparam) - IDX_MODULES), MF_BYCOMMAND | (is_enabled ? MF_CHECKED : MF_UNCHECKED));

				app.ConfigSet (ptr_module->path, is_enabled, SECTION_MODULE);

				if (is_enabled)
				{
					const DWORD load_flags = _r_sys_validversion (6, 0) ? LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE : LOAD_LIBRARY_AS_DATAFILE;

					ptr_module->hlib = LoadLibraryEx (ptr_module->path, nullptr, load_flags);

					if (ptr_module->hlib)
						count_unload -= 1;
				}
				else
				{
					if (ptr_module->hlib)
					{
						FreeLibrary (ptr_module->hlib);
						ptr_module->hlib = nullptr;
					}

					SAFE_DELETE_ARRAY (ptr_module->text);

					count_unload += 1;
				}

				_r_status_settext (hwnd, IDC_STATUSBAR, 0, _r_fmt (app.LocaleString (IDS_STATUS_TOTAL, nullptr), modules.size () - count_unload, modules.size ()));

				_app_print (hwnd);

				return FALSE;
			}

			switch (LOWORD (wparam))
			{
				case IDCANCEL: // process Esc key
				case IDM_EXIT:
				{
					DestroyWindow (hwnd);
					break;
				}

				case IDM_ALWAYSONTOP_CHK:
				{
					const bool new_val = !app.ConfigGet (L"AlwaysOnTop", _APP_ALWAYSONTOP).AsBool ();

					CheckMenuItem (GetMenu (hwnd), LOWORD (wparam), MF_BYCOMMAND | (new_val ? MF_CHECKED : MF_UNCHECKED));
					app.ConfigSet (L"AlwaysOnTop", new_val);

					_r_wnd_top (hwnd, new_val);

					break;
				}

				case IDM_INSERTBUFFER_CHK:
				{
					const bool new_val = !app.ConfigGet (L"InsertBufferAtStartup", false).AsBool ();

					CheckMenuItem (GetMenu (hwnd), LOWORD (wparam), MF_BYCOMMAND | (new_val ? MF_CHECKED : MF_UNCHECKED));
					app.ConfigSet (L"InsertBufferAtStartup", new_val);

					break;
				}

				case IDM_CHECKUPDATES_CHK:
				{
					const bool new_val = !app.ConfigGet (L"CheckUpdates", true).AsBool ();

					CheckMenuItem (GetMenu (hwnd), LOWORD (wparam), MF_BYCOMMAND | (new_val ? MF_CHECKED : MF_UNCHECKED));
					app.ConfigSet (L"CheckUpdates", new_val);

					break;
				}

				case IDM_CLASSICUI_CHK:
				{
					const bool new_val = !app.ConfigGet (L"ClassicUI", _APP_CLASSICUI).AsBool ();

					CheckMenuItem (GetMenu (hwnd), LOWORD (wparam), MF_BYCOMMAND | (new_val ? MF_CHECKED : MF_UNCHECKED));
					app.ConfigSet (L"ClassicUI", new_val);

					break;
				}

				case IDM_WEBSITE:
				{
					ShellExecute (hwnd, nullptr, _APP_WEBSITE_URL, nullptr, nullptr, SW_SHOWDEFAULT);
					break;
				}

				case IDM_CHECKUPDATES:
				{
#ifdef _APP_HAVE_UPDATES
					app.UpdateCheck (hwnd);
#endif // _APP_HAVE_UPDATES

					break;
				}

				case IDM_ABOUT:
				{
					app.CreateAboutWindow (hwnd);
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

INT APIENTRY wWinMain (HINSTANCE, HINSTANCE, LPWSTR, INT)
{
	MSG msg = {0};

	if (app.Initialize (APP_NAME, APP_NAME_SHORT, APP_VERSION, APP_COPYRIGHT))
	{
		if (app.CreateMainWindow (IDD_MAIN, IDI_MAIN, &DlgProc))
		{
			const HACCEL haccel = LoadAccelerators (app.GetHINSTANCE (), MAKEINTRESOURCE (IDA_MAIN));

			if (haccel)
			{
				while (GetMessage (&msg, nullptr, 0, 0) > 0)
				{
					TranslateAccelerator (app.GetHWND (), haccel, &msg);

					if (!IsDialogMessage (app.GetHWND (), &msg))
					{
						TranslateMessage (&msg);
						DispatchMessage (&msg);
					}
				}

				DestroyAcceleratorTable (haccel);
			}
		}
	}

	return (INT)msg.wParam;
}
