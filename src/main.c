// 	Error Lookup
//	Copyright © 2011, 2012, 2015 Henry++

#include <windows.h>

#include "main.h"

#include "resource.h"
#include "routine.h"

#include <dxerr.h> // direct x
#include <iphlpapi.h> // ip helper api
#include <ras.h> // ras

#define EMPTY_STRING L"none"

DWORD _Errlib_GetCode(HWND hwnd, INT ctrl)
{
	WCHAR buff[100] = {0};
	DWORD result = 0;

	//wcstoul; 0x00000080;

	if(GetDlgItemText(hwnd, ctrl, buff, 100))
	{
		//swscanf_s(buff, L"%ul64", &result);

		if((result = wcstoul(buff, NULL, 10)) == NULL)
		{
			result = wcstoul(buff, NULL, 16);
		}
	}

	return result;
}

BOOL _Errlib_GetDesciption(DWORD code, INT module, LPWSTR output, CONST DWORD length)
{
	HMODULE h = NULL;
	BOOL result = 0;
	DWORD lang_id = 0;// MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT);

	switch(module)
	{
		// User-Mode
		case 1:
		{
			if(FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, code, lang_id, output, length, NULL))
			{
				result = TRUE;
			}

			break;
		}

		// Kernel-Mode
		case 2:
		{
			h = LoadLibrary(L"ntdll.dll");

			if(h && FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS, h, code, lang_id, output, length, NULL))
			{
				result = TRUE;
			}

			break;
		}

		// DirectX
		case 3:
		{
			StringCchCopy(output, (size_t)length, DXGetErrorDescription(HRESULT_FROM_WIN32(code)));

			if(wcscmp(output, L"n/a\0") != 0)
			{
				result = TRUE;
			}

			break;
		}

		// Wininet
		case 4:
		{
			h = LoadLibrary(L"wininet.dll");

			if(h && FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS, h, code, lang_id, output, length, NULL))
			{
				result = TRUE;
			}

            break;
		}

		// RAS
		case 5:
		{
			if(RasGetErrorString(code, output, length) == ERROR_SUCCESS)
			{
				result = TRUE;
			}

			break;
		}

		// PDH
		case 6:
		{
			h = LoadLibrary(L"pdh.dll");

			if(h && FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS, h, code, lang_id, output, length, NULL))
			{
				result = TRUE;
			}

            break;
		}

		// IP Helper
		case 7:
		{
			DWORD lengthptr = length;

			if(GetIpErrorString(code, output, &lengthptr) == NO_ERROR)
			{
				result = TRUE;
			}

            break;
		}
	}

	//output[wcslen(output) - sizeof(WCHAR)] = L'\0';

	if(h)
	{
		FreeLibrary(h);
	}

	return result;
}

VOID _Errlib_PrintResult(HWND hwnd, INT ctrl, DWORD code, INT module)
{
	WCHAR buffer[MAX_PATH] = {0}, description[1024] = {0};

	if(!_Errlib_GetDesciption(code, module, description, 1024))
	{
		return;
	}

	INT item = (INT)SendDlgItemMessage(hwnd, ctrl, LVM_GETITEMCOUNT, 0, NULL);

	// Module
	SendDlgItemMessage(hwnd, IDC_MODULE, CB_GETLBTEXT, (WPARAM)module, (LPARAM)&buffer);
	_R_ListviewInsertItem(hwnd, ctrl, buffer + 3, item, 0);

	// Description
	_R_ListviewInsertItem(hwnd, ctrl, description, item, 1);

	// Description #1
	StringCchPrintf(buffer, MAX_PATH, L"%d\0", code);
	SetDlgItemText(hwnd, IDC_DESCRIPTION_1, buffer);

	// Description #2
	StringCchPrintf(buffer, MAX_PATH, L"0x%08x\0", /*HRESULT_FROM_WIN32*/ code);
	SetDlgItemText(hwnd, IDC_DESCRIPTION_2, buffer);

	// Description #3
    switch(HRESULT_SEVERITY(code))
    {
		SWITCH_ROUTINE(STATUS_SEVERITY_SUCCESS)
		SWITCH_ROUTINE(STATUS_SEVERITY_INFORMATIONAL)
		SWITCH_ROUTINE(STATUS_SEVERITY_WARNING)
		SWITCH_ROUTINE(STATUS_SEVERITY_ERROR)

        default:
		{
			StringCchCopy(buffer, MAX_PATH, EMPTY_STRING);
			break;
		}
	}

	SetDlgItemText(hwnd, IDC_DESCRIPTION_3, buffer);

	// Description #4
    switch(HRESULT_FACILITY(code))
    {
		SWITCH_ROUTINE(FACILITY_NULL)
		SWITCH_ROUTINE(FACILITY_RPC)
		SWITCH_ROUTINE(FACILITY_DISPATCH)
        SWITCH_ROUTINE(FACILITY_STORAGE)
        SWITCH_ROUTINE(FACILITY_ITF)
        SWITCH_ROUTINE(FACILITY_WIN32)
        SWITCH_ROUTINE(FACILITY_WINDOWS)
        SWITCH_ROUTINE(FACILITY_SECURITY) // FACILITY_SSPI
        SWITCH_ROUTINE(FACILITY_CONTROL)
        SWITCH_ROUTINE(FACILITY_CERT)
        SWITCH_ROUTINE(FACILITY_INTERNET)
        SWITCH_ROUTINE(FACILITY_MEDIASERVER)
        SWITCH_ROUTINE(FACILITY_MSMQ)
        SWITCH_ROUTINE(FACILITY_SETUPAPI)
        SWITCH_ROUTINE(FACILITY_SCARD)
        SWITCH_ROUTINE(FACILITY_COMPLUS)
        SWITCH_ROUTINE(FACILITY_AAF)
        SWITCH_ROUTINE(FACILITY_URT)
        SWITCH_ROUTINE(FACILITY_ACS)
        SWITCH_ROUTINE(FACILITY_DPLAY)
        SWITCH_ROUTINE(FACILITY_UMI)
        SWITCH_ROUTINE(FACILITY_SXS)
        SWITCH_ROUTINE(FACILITY_WINDOWS_CE)
        SWITCH_ROUTINE(FACILITY_HTTP)
        SWITCH_ROUTINE(FACILITY_USERMODE_COMMONLOG)
        SWITCH_ROUTINE(FACILITY_USERMODE_FILTER_MANAGER)
        SWITCH_ROUTINE(FACILITY_BACKGROUNDCOPY)
        SWITCH_ROUTINE(FACILITY_CONFIGURATION)
        SWITCH_ROUTINE(FACILITY_STATE_MANAGEMENT)
        SWITCH_ROUTINE(FACILITY_METADIRECTORY)
        SWITCH_ROUTINE(FACILITY_WINDOWSUPDATE)
        SWITCH_ROUTINE(FACILITY_DIRECTORYSERVICE)
        SWITCH_ROUTINE(FACILITY_GRAPHICS)
        SWITCH_ROUTINE(FACILITY_SHELL)
        SWITCH_ROUTINE(FACILITY_TPM_SERVICES)
        SWITCH_ROUTINE(FACILITY_TPM_SOFTWARE)
        SWITCH_ROUTINE(FACILITY_PLA)
        SWITCH_ROUTINE(FACILITY_FVE)
        SWITCH_ROUTINE(FACILITY_FWP)
        SWITCH_ROUTINE(FACILITY_WINRM)
        SWITCH_ROUTINE(FACILITY_NDIS)
        SWITCH_ROUTINE(FACILITY_USERMODE_HYPERVISOR)
        SWITCH_ROUTINE(FACILITY_CMI)
        SWITCH_ROUTINE(FACILITY_USERMODE_VIRTUALIZATION)
        SWITCH_ROUTINE(FACILITY_USERMODE_VOLMGR)
        SWITCH_ROUTINE(FACILITY_BCD)
        SWITCH_ROUTINE(FACILITY_USERMODE_VHD)
        SWITCH_ROUTINE(FACILITY_SDIAG)
        SWITCH_ROUTINE(FACILITY_WEBSERVICES)
        SWITCH_ROUTINE(FACILITY_WINDOWS_DEFENDER)
        SWITCH_ROUTINE(FACILITY_OPC)
        SWITCH_ROUTINE(FACILITY_D3D)
        SWITCH_ROUTINE(FACILITY_D3DX)
        SWITCH_ROUTINE(FACILITY_DSOUND_DMUSIC)
        SWITCH_ROUTINE(FACILITY_D3D10)
        SWITCH_ROUTINE(FACILITY_XAUDIO2)
        SWITCH_ROUTINE(FACILITY_XAPO)
        SWITCH_ROUTINE(FACILITY_XACTENGINE)
        SWITCH_ROUTINE(FACILITY_D3D11_OR_AE)
        SWITCH_ROUTINE(FACILITY_D2D)
        SWITCH_ROUTINE(FACILITY_DWRITE)
        SWITCH_ROUTINE(FACILITY_APO)
        SWITCH_ROUTINE(FACILITY_LEAP)
        SWITCH_ROUTINE(FACILITY_WSAPI)
        SWITCH_ROUTINE(FACILITY_DXGI)

        default:
		{
			StringCchCopy(buffer, MAX_PATH, EMPTY_STRING);
			break;
		}
    }

	SetDlgItemText(hwnd, IDC_DESCRIPTION_4, buffer);
}

VOID _Errlib_Clear(HWND hwnd)
{
	SendDlgItemMessage(hwnd, IDC_LISTVIEW, LVM_DELETEALLITEMS, 0, NULL);

	SetDlgItemText(hwnd, IDC_DESCRIPTION_1, NULL);
	SetDlgItemText(hwnd, IDC_DESCRIPTION_2, NULL);
	SetDlgItemText(hwnd, IDC_DESCRIPTION_3, NULL);
	SetDlgItemText(hwnd, IDC_DESCRIPTION_4, NULL);
}

BOOL CALLBACK EnumResLangProc(HMODULE hModule, LPCTSTR lpszType, LPCTSTR lpszName, WORD wIDLanguage, LONG_PTR lParam)
{
	WCHAR buffer[100] = {0};

	StringCchPrintf(buffer, 100, L"%s (%i)", lpszName, wIDLanguage);
	MessageBox((HWND)lParam, buffer, 0, 0);

	SendDlgItemMessage((HWND)lParam, IDC_LANGUAGE, CB_INSERTSTRING, 0, (LPARAM)lpszName);

	return TRUE;
}

INT_PTR WINAPI PagesDlgProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			SetProp(hwnd, L"id", (HANDLE)lparam);

			if((INT)lparam == IDD_SETTINGS_1)
			{
				CheckDlgButton(hwnd, IDC_INSERTBUFFER_CHK, _R_ConfigGet(APP_NAME_SHORT, L"InsertBufferAtStartup", 1) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwnd, IDC_CHECKUPDATES_CHK, _R_ConfigGet(APP_NAME_SHORT, L"CheckUpdates", 1) ? BST_CHECKED : BST_UNCHECKED);

				switch(_R_ConfigGet(APP_NAME_SHORT, L"InputType", 0))
				{
					case 1:
					{
						CheckDlgButton(hwnd, IDC_TYPE_DEC, BST_CHECKED);
						break;
					}
					
					case 2:
					{
						CheckDlgButton(hwnd, IDC_TYPE_HEX, BST_CHECKED);
						break;
					}

					default:
					{
						CheckDlgButton(hwnd, IDC_TYPE_AUTO, BST_CHECKED);
						break;
					}
				}

				EnumResourceLanguages(NULL, NULL, NULL, EnumResLangProc, (LPARAM)hwnd);
			}

			break;
		}

		case WM_DESTROY:
		{
			// save settings if this property is empty
			if(!GetProp(GetParent(hwnd), L"hwnd"))
			{
				if((INT)GetProp(hwnd, L"id") == IDD_SETTINGS_1)
				{
					_R_ConfigSet(APP_NAME_SHORT, L"InsertBufferAtStartup", INT((IsDlgButtonChecked(hwnd, IDC_INSERTBUFFER_CHK) == BST_CHECKED) ? TRUE : FALSE));
					_R_ConfigSet(APP_NAME_SHORT, L"CheckUpdates", INT((IsDlgButtonChecked(hwnd, IDC_CHECKUPDATES_CHK) == BST_CHECKED) ? TRUE : FALSE));

					{
						INT rb = 0;

						if(IsDlgButtonChecked(hwnd, IDC_TYPE_DEC) == BST_CHECKED)
						{
							rb = 1;
						}
						else if(IsDlgButtonChecked(hwnd, IDC_TYPE_HEX) == BST_CHECKED)
						{
							rb = 2;
						}

						_R_ConfigSet(APP_NAME_SHORT, L"InputType", rb);
					}
				}
			}

			break;
		}
	}

	return FALSE;
}


INT_PTR CALLBACK SettingsDlgProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			_R_TreeviewSetStyle(hwnd, IDC_NAV, TVS_EX_DOUBLEBUFFER, 20);

			_R_TreeviewInsertItem(hwnd, IDC_NAV, L"General", -1, (LPARAM)CreateDialogParam(NULL, MAKEINTRESOURCE(IDD_SETTINGS_1), hwnd, PagesDlgProc, IDD_SETTINGS_1));
			_R_TreeviewInsertItem(hwnd, IDC_NAV, L"Modules", -1, (LPARAM)CreateDialogParam(NULL, MAKEINTRESOURCE(IDD_SETTINGS_2), hwnd, PagesDlgProc, IDD_SETTINGS_2));

			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR lphdr = (LPNMHDR)lparam;

			switch(lphdr->code)
			{
				case TVN_SELCHANGED:
				{
					if(wparam == IDC_NAV)
					{
						LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)lparam;

						ShowWindow((HWND)GetProp(hwnd, L"hwnd"), SW_HIDE);

						SetProp(hwnd, L"hwnd", (HANDLE)pnmtv->itemNew.lParam);
						ShowWindow((HWND)pnmtv->itemNew.lParam, SW_SHOW);
					}

					break;
				}
			}

			break;
		}

		case WM_COMMAND:
		{
			switch(LOWORD(wparam))
			{
				case IDOK: // process Enter key
				case IDC_SAVE:
				{
					SetProp(hwnd, L"hwnd", NULL); // indicator for save settings

					// without break;
				}

				case IDCANCEL: // process Esc key
				case IDC_CANCEL:
				{
					EndDialog(hwnd, 0);
					break;
				}
			}

			break;
		}
	}

	return FALSE;
}

LRESULT CALLBACK DlgProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			_R_ListviewSetStyle(hwnd, IDC_LISTVIEW, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP, FALSE);
			
			_R_ListviewInsertColumn(hwnd, IDC_LISTVIEW, (LPWSTR)i18n(IDS_COLUMN_1), _R_ConfigGet(APP_NAME_SHORT, L"Column1", (INT)190), 1, LVCFMT_LEFT);
			_R_ListviewInsertColumn(hwnd, IDC_LISTVIEW, (LPWSTR)i18n(IDS_COLUMN_2), _R_ConfigGet(APP_NAME_SHORT, L"Column2", (INT)370), 2, LVCFMT_LEFT);

			SendDlgItemMessage(hwnd, IDC_MODULE, CB_INSERTSTRING, 0, (LPARAM)L"#0 Retrieve all");
			SendDlgItemMessage(hwnd, IDC_MODULE, CB_INSERTSTRING, 1, (LPARAM)L"#1 Windows (User-Mode)");
			SendDlgItemMessage(hwnd, IDC_MODULE, CB_INSERTSTRING, 2, (LPARAM)L"#2 Windows (Kernel-Mode)");
			SendDlgItemMessage(hwnd, IDC_MODULE, CB_INSERTSTRING, 3, (LPARAM)L"#3 Graphics and Gaming (DirectX)");
			SendDlgItemMessage(hwnd, IDC_MODULE, CB_INSERTSTRING, 4, (LPARAM)L"#4 Windows Internet");
			SendDlgItemMessage(hwnd, IDC_MODULE, CB_INSERTSTRING, 5, (LPARAM)L"#5 Remote Access Service");
			SendDlgItemMessage(hwnd, IDC_MODULE, CB_INSERTSTRING, 6, (LPARAM)L"#6 Performance Data Helper");
			SendDlgItemMessage(hwnd, IDC_MODULE, CB_INSERTSTRING, 7, (LPARAM)L"#7 IP Helper");

			SendDlgItemMessage(hwnd, IDC_MODULE, CB_SETCURSEL, (LPARAM)_R_ConfigGet(APP_NAME_SHORT, L"Module", (INT)0), 0);

			if(_R_ConfigGet(APP_NAME_SHORT, L"InsertBufferAtStartup", 1))
			{
				SetDlgItemText(hwnd, IDC_CODE, _R_ClipboardGet());
			}

			if(_R_ConfigGet(APP_NAME_SHORT, L"CheckUpdates", 1))
			{
				_R_UpdateCheck(TRUE);
			}

			break;
		}

		case WM_DESTROY:
		{
			_R_ConfigSet(APP_NAME_SHORT, L"Module", (DWORD)SendDlgItemMessage(hwnd, IDC_MODULE, CB_GETCURSEL, 0, 0));

			_R_ConfigSet(APP_NAME_SHORT, L"Column1", (DWORD)SendDlgItemMessage(hwnd, IDC_LISTVIEW, LVM_GETCOLUMNWIDTH, 0, 0));
			_R_ConfigSet(APP_NAME_SHORT, L"Column2", (DWORD)SendDlgItemMessage(hwnd, IDC_LISTVIEW, LVM_GETCOLUMNWIDTH, 1, 0));

			PostQuitMessage(0);

			break;
		}

		case WM_QUERYENDSESSION:
		{
			if(lparam == ENDSESSION_CLOSEAPP)
			{
				return TRUE;
			}

			break;
		}

		case WM_CONTEXTMENU:
		{
			if(GetDlgCtrlID((HWND)wparam) == IDC_LISTVIEW)
			{
				HMENU menu = LoadMenu(NULL, MAKEINTRESOURCE(IDM_LISTVIEW)), submenu = GetSubMenu(menu, 0);

				TrackPopupMenuEx(submenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_NOANIMATION, LOWORD(lparam), HIWORD(lparam), hwnd, NULL);

				DestroyMenu(menu);
				DestroyMenu(submenu);
			}

			break;
		}

		case WM_COMMAND:
		{
			switch(LOWORD(wparam))
			{
				case IDM_SETTINGS:
				{
					DialogBox(NULL, MAKEINTRESOURCE(IDD_SETTINGS), hwnd, SettingsDlgProc);
					break;
				}

				case IDCANCEL: // process Esc key
				case IDM_EXIT:
				{
					DestroyWindow(hwnd);
					break;
				}

				case IDOK: // process Enter key
				case IDC_GET:
				{
					DWORD code = _Errlib_GetCode(hwnd, IDC_CODE);

					_Errlib_Clear(hwnd);

					for(INT i = 1; i < (INT)SendDlgItemMessage(hwnd, IDC_MODULE, CB_GETCOUNT, 0, 0); i++)
					{
						_Errlib_PrintResult(hwnd, IDC_LISTVIEW, code, i);
					}

					break;
				}

				case IDM_CLEAR:
				{
					_Errlib_Clear(hwnd);
					break;
				}

				case IDM_WEBSITE:
				{
					ShellExecute(hwnd, NULL, APP_WEBSITE, NULL, NULL, 0);
					break;
				}

				case IDM_CHECKUPDATES:
				{
					_R_UpdateCheck(FALSE);
					break;
				}

				case IDM_ABOUT:
				{
					_R_AboutBox(hwnd/*, APP_NAME, L"Version %s, " APP_MACHINE L"-bit (Unicode)\r\n© 2015 Henry++. All Rights Reserved.\r\n\r\n%s\r\n\r\n%s", APP_VERSION, i18n(IDS_COPYRIGHT), _R_SystemValidVersion(6, 0) ? L"<a href=\"" APP_WEBSITE L"\">" APP_HOST L"</a>" : APP_HOST*/);
					break;
				}
			}

			break;
		}
	}

	return FALSE;
}

INT APIENTRY wWinMain(HINSTANCE, HINSTANCE, LPWSTR, INT)
{
	//SetThreadLocale(LANG_ENGLISH);
	SetThreadUILanguage(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), SORT_DEFAULT));

	//SetThreadLocale(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), SORT_DEFAULT));

	if(_R_Initialize((DLGPROC)DlgProc))
	{
		MSG msg = {0};

		while(GetMessage(&msg, NULL, 0, 0))
		{
			if(!IsDialogMessage(_r_hwnd, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	_R_Uninitialize();

	return ERROR_SUCCESS;
}