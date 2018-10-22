// Error Lookup
// Copyright (c) 2011-2018 Henry++

#include <windows.h>

#include "main.hpp"
#include "rapp.hpp"
#include "routine.hpp"

#include "pugiconfig.hpp"
#include "..\..\pugixml\src\pugixml.hpp"

#include "resource.hpp"

rapp app (APP_NAME, APP_NAME_SHORT, APP_VERSION, APP_COPYRIGHT);

std::vector<ITEM_MODULE> modules;

std::unordered_map<size_t, LPWSTR> facility;
std::unordered_map<size_t, LPWSTR> severity;

LCID lcid = 0;

INT statusbar_height = 0;
size_t count_unload = 0;

WCHAR info[MAX_PATH] = {0};

DWORD _app_getcode (HWND hwnd, BOOL* is_hex)
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

			if (_wcsnicmp (result, L"%1", 2) == 0)
			{
				result.Clear (); // clear
			}
		}
		else
		{
			if (is_localized)
				result = _app_formatmessage (code, hinstance, false);
		}

		result.Trim (L"\r\n ");

		LocalFree (buffer);
	}

	return result;
}

VOID _app_showdescription (HWND hwnd, size_t idx)
{
	if (idx != LAST_VALUE)
	{
		ITEM_MODULE* const ptr = &modules.at (idx);

		_r_ctrl_settext (hwnd, IDC_DESCRIPTION_CTL, L"%s\r\n\r\n%s", info, ptr->text);

		_r_status_settext (hwnd, IDC_STATUSBAR, 1, _r_fmt (L"%s [%s]", ptr->description, ptr->path));
	}
	else
	{
		SetDlgItemText (hwnd, IDC_DESCRIPTION_CTL, info);
		_r_status_settext (hwnd, IDC_STATUSBAR, 1, nullptr);
	}
}

VOID _app_listviewresize (HWND hwnd)
{
	RECT rc = {0};
	GetClientRect (GetDlgItem (hwnd, IDC_LISTVIEW), &rc);

	_r_listview_setcolumn (hwnd, IDC_LISTVIEW, 0, nullptr, rc.right);
}

VOID _app_print (HWND hwnd)
{
	const DWORD code = _app_getcode (hwnd, nullptr);

	const DWORD severity_code = HRESULT_SEVERITY (code);
	const DWORD facility_code = HRESULT_FACILITY (code);

	rstring buffer;

	// clear first
	_r_listview_deleteallitems (hwnd, IDC_LISTVIEW);

	// print information
	StringCchPrintf (info, _countof (info), app.LocaleString (IDS_INFORMATION, nullptr), code, code, (severity.find (severity_code) != severity.end ()) ? severity[severity_code] : L"n/a", severity_code, (facility.find (facility_code) != facility.end ()) ? facility[facility_code] : L"n/a", facility_code);

	// print modules
	size_t item_count = 0;

	for (size_t i = 0; i < modules.size (); i++)
	{
		ITEM_MODULE* const ptr_module = &modules.at (i);

		if (!ptr_module->hlib || !app.ConfigGet (ptr_module->path, true, SECTION_MODULE).AsBool ())
			continue;

		buffer = _app_formatmessage (code, ptr_module->hlib);

		if (!buffer.IsEmpty ())
		{
			const size_t length = buffer.GetLength () + 1;

			if (ptr_module->text)
			{
				delete[] ptr_module->text;
				ptr_module->text = nullptr;
			}

			ptr_module->text = new WCHAR[length];

			StringCchCopy (ptr_module->text, length, buffer);

			_r_listview_additem (hwnd, IDC_LISTVIEW, item_count, 0, ptr_module->description, LAST_VALUE, LAST_VALUE, i);
			item_count += 1;
		}
		else
		{
			SetDlgItemText (hwnd, IDC_DESCRIPTION_CTL, info);
		}
	}

	_app_listviewresize (hwnd);

	// show description for first item
	if (!item_count)
	{
		_app_showdescription (hwnd, LAST_VALUE);
	}
	else
	{
		ListView_SetItemState (GetDlgItem (hwnd, IDC_LISTVIEW), 0, LVIS_ACTIVATING, LVIS_ACTIVATING); // select item
	}
}

LPVOID _app_loadresource (PDWORD size)
{
	const HINSTANCE hinst = app.GetHINSTANCE ();
	HRSRC hResource = FindResource (hinst, MAKEINTRESOURCE (1), RT_RCDATA);

	if (hResource)
	{
		HGLOBAL hLoadedResource = LoadResource (hinst, hResource);

		if (hLoadedResource)
		{
			LPVOID pLockedResource = LockResource (hLoadedResource);

			if (pLockedResource)
			{
				DWORD dwResourceSize = SizeofResource (hinst, hResource);

				if (dwResourceSize != 0)
				{
					if (size)
						*size = dwResourceSize;

					return pLockedResource;
				}
			}
		}
	}

	return nullptr;
}

VOID _app_loaddatabase (HWND hwnd)
{
	DWORD rc_length = 0;
	LPVOID da = _app_loadresource (&rc_length);

	const HMENU hmenu = GetSubMenu (GetSubMenu (GetMenu (hwnd), 1), MODULES_MENU);
	DeleteMenu (hmenu, 0, MF_BYPOSITION); // delete separator

	if (da)
	{
		pugi::xml_document doc;
		pugi::xml_parse_result result = doc.load_buffer (da, rc_length, PUGIXML_LOAD_FLAGS, PUGIXML_LOAD_ENCODING);

		if (result)
		{
			pugi::xml_node root = doc.child (L"root");

			if (root)
			{
				pugi::xml_node sub_root;

				// load modules information
				{
					// clear it first
					{
						for (size_t i = 0; i < modules.size (); i++)
						{
							if (modules.at (i).hlib)
							{
								FreeLibrary (modules.at (i).hlib);
								modules.at (i).hlib = nullptr;
							}

							if (modules.at (i).text)
							{
								delete[] modules.at (i).text;
								modules.at (i).text = nullptr;
							}
						}

						modules.clear ();
					}

					sub_root = root.child (L"module");

					if (sub_root)
					{
						UINT i = 0;

						for (pugi::xml_node item = sub_root.child (L"item"); item; item = item.next_sibling (L"item"))
						{
							ITEM_MODULE module;
							SecureZeroMemory (&module, sizeof (module));

							StringCchCopy (module.path, _countof (module.path), item.attribute (L"file").as_string ());
							StringCchCopy (module.description, _countof (module.description), !item.attribute (L"text").empty () ? item.attribute (L"text").as_string () : module.path);

							const BOOL is_enabled = app.ConfigGet (module.path, true, SECTION_MODULE).AsBool ();

							if (is_enabled)
								module.hlib = LoadLibraryEx (module.path, nullptr, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);

							if (!is_enabled || !module.hlib)
								count_unload += 1;

							{
								MENUITEMINFO mii = {0};

								mii.cbSize = sizeof (mii);
								mii.fMask = MIIM_ID | MIIM_STATE | MIIM_STRING;
								mii.fType = MFT_STRING;
								mii.dwTypeData = module.description;
								mii.fState = is_enabled ? MFS_CHECKED : MF_UNCHECKED;
								mii.wID = IDX_MODULES + i;

								if (!module.hlib && is_enabled)
									mii.fState |= MFS_DISABLED | MF_GRAYED;

								InsertMenuItem (hmenu, IDX_MODULES + i, false, &mii);
							}

							modules.push_back (module);

							i += 1;
						}
					}
				}

				// load facility information
				{
					facility.clear ();

					sub_root = root.child (L"facility");

					if (sub_root)
					{
						for (pugi::xml_node item = sub_root.child (L"item"); item; item = item.next_sibling (L"item"))
						{
							const size_t code = item.attribute (L"code").as_uint ();
							rstring text = item.attribute (L"text").as_string ();

							size_t length = text.GetLength () + 1;
							LPWSTR ptr2 = new WCHAR[length];

							StringCchCopy (ptr2, length, text);

							(facility)[code] = ptr2;
						}
					}
				}

				// load severity information
				{
					severity.clear ();

					sub_root = root.child (L"severity");

					if (sub_root)
					{
						for (pugi::xml_node item = sub_root.child (L"item"); item; item = item.next_sibling (L"item"))
						{
							const size_t code = item.attribute (L"code").as_uint ();
							rstring text = item.attribute (L"text").as_string ();

							size_t length = text.GetLength () + 1;
							LPWSTR ptr2 = new WCHAR[length];

							StringCchCopy (ptr2, length, text);

							(severity)[code] = ptr2;
						}
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

VOID ResizeWindow (HWND hwnd, INT width, INT height)
{
	RECT rc = {0};
	GetWindowRect (GetDlgItem (hwnd, IDC_LISTVIEW), &rc);
	const INT listview_width = _R_RECT_WIDTH (&rc);

	GetClientRect (GetDlgItem (hwnd, IDC_LISTVIEW), &rc);
	INT listview_height = (height - (rc.top - rc.bottom) - statusbar_height) - app.GetDPI (80);
	listview_height -= _R_RECT_HEIGHT (&rc);

	GetClientRect (GetDlgItem (hwnd, IDC_DESCRIPTION_CTL), &rc);

	const INT edit_width = (width - listview_width) - app.GetDPI (36);
	INT edit_height = (height - (rc.top - rc.bottom) - statusbar_height) - app.GetDPI (42);
	edit_height -= _R_RECT_HEIGHT (&rc);

	SetWindowPos (GetDlgItem (hwnd, IDC_LISTVIEW), nullptr, 0, 0, listview_width, listview_height, SWP_NOMOVE | SWP_NOCOPYBITS | SWP_NOREDRAW | SWP_NOSENDCHANGING | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
	SetWindowPos (GetDlgItem (hwnd, IDC_DESCRIPTION), nullptr, 0, 0, edit_width, app.GetDPI (14), SWP_NOMOVE | SWP_NOCOPYBITS | SWP_NOREDRAW | SWP_NOSENDCHANGING | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE);
	SetWindowPos (GetDlgItem (hwnd, IDC_DESCRIPTION_CTL), nullptr, 0, 0, edit_width, edit_height, SWP_NOMOVE | SWP_NOCOPYBITS | SWP_NOREDRAW | SWP_NOSENDCHANGING | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE);

	// resize statusbar parts
	INT parts[] = {listview_width + app.GetDPI (24), -1};
	SendDlgItemMessage (hwnd, IDC_STATUSBAR, SB_SETPARTS, 2, (LPARAM)parts);

	// resize column width
	_app_listviewresize (hwnd);

	// resize statusbar
	SendDlgItemMessage (hwnd, IDC_STATUSBAR, WM_SIZE, 0, 0);
}

INT_PTR CALLBACK DlgProc (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			// configure listview
			_r_listview_setstyle (hwnd, IDC_LISTVIEW, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);

			// resize support
			{
				RECT rc = {0};
				GetClientRect (GetDlgItem (hwnd, IDC_STATUSBAR), &rc);

				statusbar_height = rc.bottom;
			}

			// configure controls
			SendDlgItemMessage (hwnd, IDC_CODE_UD, UDM_SETRANGE32, 0, INT32_MAX);

			// configure menu
			CheckMenuItem (GetMenu (hwnd), IDM_ALWAYSONTOP_CHK, MF_BYCOMMAND | (app.ConfigGet (L"AlwaysOnTop", false).AsBool () ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem (GetMenu (hwnd), IDM_INSERTBUFFER_CHK, MF_BYCOMMAND | (app.ConfigGet (L"InsertBufferAtStartup", false).AsBool () ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem (GetMenu (hwnd), IDM_CHECKUPDATES_CHK, MF_BYCOMMAND | (app.ConfigGet (L"CheckUpdates", true).AsBool () ? MF_CHECKED : MF_UNCHECKED));
			CheckMenuItem (GetMenu (hwnd), IDM_CLASSICUI_CHK, MF_BYCOMMAND | (app.ConfigGet (L"ClassicUI", _APP_CLASSICUI).AsBool () ? MF_CHECKED : MF_UNCHECKED));

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

		case WM_DESTROY:
		{
			app.ConfigSet (L"LatestCode", _r_ctrl_gettext (hwnd, IDC_CODE_CTL));

			PostQuitMessage (0);

			break;
		}

		case RM_LOCALIZE:
		{
			// get locale id
			lcid = wcstoul (app.LocaleString (IDS_LCID, nullptr), nullptr, 16);

			_r_listview_deleteallcolumns (hwnd, IDC_LISTVIEW);
			_r_listview_addcolumn (hwnd, IDC_LISTVIEW, 0, app.LocaleString (IDS_MODULES, nullptr), 95, LVCFMT_LEFT);

			// localize
			const HMENU menu = GetMenu (hwnd);

			app.LocaleMenu (menu, IDS_FILE, 0, true, nullptr);
			app.LocaleMenu (menu, IDS_EXIT, IDM_EXIT, false, L"\tEsc");

			app.LocaleMenu (menu, IDS_SETTINGS, 1, true, nullptr);
			app.LocaleMenu (menu, IDS_ALWAYSONTOP_CHK, IDM_ALWAYSONTOP_CHK, false, nullptr);
			app.LocaleMenu (menu, IDS_INSERTBUFFER_CHK, IDM_INSERTBUFFER_CHK, false, nullptr);
			app.LocaleMenu (menu, IDS_CHECKUPDATES_CHK, IDM_CHECKUPDATES_CHK, false, nullptr);
			app.LocaleMenu (menu, IDS_CLASSICUI_CHK, IDM_CLASSICUI_CHK, false, nullptr);
			app.LocaleMenu (GetSubMenu (menu, 1), IDS_MODULES, MODULES_MENU, true, nullptr);
			app.LocaleMenu (GetSubMenu (menu, 1), IDS_LANGUAGE, LANG_MENU, true, L" (Language)");

			app.LocaleMenu (menu, IDS_HELP, 2, true, nullptr);
			app.LocaleMenu (menu, IDS_WEBSITE, IDM_WEBSITE, false, nullptr);
			app.LocaleMenu (menu, IDS_CHECKUPDATES, IDM_CHECKUPDATES, false, nullptr);
			app.LocaleMenu (menu, IDS_ABOUT, IDM_ABOUT, false, L"\tF1");

			SetDlgItemText (hwnd, IDC_CODE, app.LocaleString (IDS_CODE, nullptr));
			SetDlgItemText (hwnd, IDC_DESCRIPTION, app.LocaleString (IDS_DESCRIPTION, nullptr));

			_app_print (hwnd);

			_r_status_settext (hwnd, IDC_STATUSBAR, 0, _r_fmt (app.LocaleString (IDS_STATUS_TOTAL, nullptr), modules.size () - count_unload, modules.size ()));

			app.LocaleEnum ((HWND)GetSubMenu (menu, 1), LANG_MENU, true, IDX_LANGUAGE); // enum localizations

			SendDlgItemMessage (hwnd, IDC_LISTVIEW, (LVM_FIRST + 84), 0, 0); // LVM_RESETEMPTYTEXT

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

						if (lpnm->iItem != -1)
						{
							const size_t idx = _r_listview_getitemlparam (hwnd, IDC_LISTVIEW, (size_t)lpnm->iItem);

							_app_showdescription (hwnd, idx);
						}
						else
						{
							_app_showdescription (hwnd, LAST_VALUE);
						}
					}

					break;
				}

				case LVN_GETINFOTIP:
				{
					LPNMLVGETINFOTIP const lpnmlv = (LPNMLVGETINFOTIP)lparam;

					ITEM_MODULE* const ptr = &modules.at (_r_listview_getitemlparam (hwnd, IDC_LISTVIEW, (size_t)lpnmlv->iItem));

					StringCchPrintf (lpnmlv->pszText, lpnmlv->cchTextMax, L"%s [%s]\r\n%s", ptr->description, ptr->path, ptr->text);

					break;
				}

				case LVN_GETEMPTYMARKUP:
				{
					NMLVEMPTYMARKUP* const lpnmlv = (NMLVEMPTYMARKUP*)lparam;

					lpnmlv->dwFlags = EMF_CENTERED;
					StringCchCopy (lpnmlv->szMarkup, _countof (lpnmlv->szMarkup), app.LocaleString (IDS_STATUS_EMPTY, nullptr));

					SetWindowLongPtr (hwnd, DWLP_MSGRESULT, true);
					return true;
				}

				case UDN_DELTAPOS:
				{
					if (hdr->idFrom == IDC_CODE_UD)
					{
						BOOL is_hex = false;
						const DWORD code = _app_getcode (hwnd, &is_hex);

						_r_ctrl_settext (hwnd, IDC_CODE_CTL, is_hex ? FORMAT_HEX : FORMAT_DEC, code + LPNMUPDOWN (lparam)->iDelta);
						_app_print (hwnd);

						return true;
					}

					break;
				}
			}

			break;
		}

		case WM_SIZE:
		{
			ResizeWindow (hwnd, LOWORD (lparam), HIWORD (lparam));
			RedrawWindow (hwnd, nullptr, nullptr, RDW_ALLCHILDREN | RDW_ERASE | RDW_INVALIDATE);

			break;
		}

		case WM_COMMAND:
		{
			if (LOWORD (wparam) == IDC_CODE_CTL && HIWORD (wparam) == EN_CHANGE)
			{
				_app_print (hwnd);
				return false;
			}

			if (HIWORD (wparam) == 0 && LOWORD (wparam) >= IDX_LANGUAGE && LOWORD (wparam) <= IDX_LANGUAGE + app.LocaleGetCount ())
			{
				app.LocaleApplyFromMenu (GetSubMenu (GetSubMenu (GetMenu (hwnd), 1), LANG_MENU), LOWORD (wparam), IDX_LANGUAGE);
				return FALSE;
			}
			else if ((LOWORD (wparam) >= IDX_MODULES && LOWORD (wparam) <= IDX_MODULES + modules.size ()))
			{
				const size_t idx = LOWORD (wparam) - IDX_MODULES;

				ITEM_MODULE *ptr_module = &modules.at (idx);

				const bool is_enabled = !app.ConfigGet (ptr_module->path, true, SECTION_MODULE).AsBool ();

				CheckMenuItem (GetMenu (hwnd), IDX_MODULES + (LOWORD (wparam) - IDX_MODULES), MF_BYCOMMAND | (is_enabled ? MF_CHECKED : MF_UNCHECKED));

				app.ConfigSet (ptr_module->path, is_enabled, SECTION_MODULE);

				if (is_enabled)
				{
					ptr_module->hlib = LoadLibraryEx (ptr_module->path, nullptr, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);

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

					if (ptr_module->text)
					{
						delete[] ptr_module->text;
						ptr_module->text = nullptr;
					}

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
					const bool new_val = !app.ConfigGet (L"AlwaysOnTop", false).AsBool ();

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
					app.UpdateCheck (true);
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

	return false;
}

INT APIENTRY wWinMain (HINSTANCE, HINSTANCE, LPWSTR, INT)
{
	MSG msg = {0};

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

	return (INT)msg.wParam;
}
