// Error Lookup
// Copyright © 2011, 2012, 2015 Henry++

#include <windows.h>

#include "main.h"

#include "resource.h"
#include "routine.h"

#include <dxerr.h> // direct x
#include <iphlpapi.h> // ip helper api
#include <ras.h> // ras

#define NULL_STRING L"(null)"

LCID ui_lcid[] = {
	MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT), SORT_DEFAULT),
	MAKELCID(MAKELANGID(LANG_RUSSIAN, SUBLANG_DEFAULT), SORT_DEFAULT)
};

DWORD _Errlib_GetCode(HWND hwnd, INT ctrl)
{
	WCHAR buff[100] = {0};
	DWORD result = 0;

	if(GetDlgItemText(hwnd, ctrl, buff, 100))
	{
		//swscanf_s(buff, L"%lld", &result); // dec

		if((result = wcstoul(buff, NULL, 10)) == NULL)
		{
			result = wcstoul(buff, NULL, 16);
		}
	}

	return result;
}

BOOL _Errlib_FormatMessage(DWORD code, LPWSTR library, LPWSTR buffer, DWORD size)
{
	HMODULE h = LoadLibraryEx(library, NULL, LOAD_LIBRARY_AS_DATAFILE);

	if(h && FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS, h, code, 0, buffer, size, NULL))
	{
		buffer[wcslen(buffer) - sizeof(WCHAR)] = L'\0';

		return TRUE;
	}

	return FALSE;
}

VOID _Errlib_PrintDescription(HWND hwnd, INT ctrl, DWORD code)
{
	WCHAR buffer[1024] = {0};
	DWORD length = 1024;

	INT i = 0;

	// User-Mode
	if(1)
	{
		if(_Errlib_FormatMessage(code, L"kernel32.dll", buffer, length))
		{
			_r_listviewinsertitem(hwnd, ctrl, L"Windows (User-Mode)", i, 0);
			_r_listviewinsertitem(hwnd, ctrl, buffer, i++, 1);
		}
	}

	// Kernel-Mode
	if(2)
	{
		if(_Errlib_FormatMessage(code, L"ntdll.dll", buffer, length))
		{
			_r_listviewinsertitem(hwnd, ctrl, L"Windows (Kernel-Mode)", i, 0);
			_r_listviewinsertitem(hwnd, ctrl, buffer, i++, 1);
		}
	}

	// DirectX
	if(3)
	{
		StringCchCopy(buffer, (size_t)length, DXGetErrorDescription(HRESULT(code)));

		if(wcscmp(buffer, L"n/a\0") != 0)
		{
			_r_listviewinsertitem(hwnd, ctrl, L"Graphics and Gaming (DirectX)", i, 0);
			_r_listviewinsertitem(hwnd, ctrl, buffer, i++, 1);
		}
	}

	// Wininet
	if(4)
	{
		if(_Errlib_FormatMessage(code, L"wininet.dll", buffer, length))
		{
			_r_listviewinsertitem(hwnd, ctrl, L"Windows Internet (WinINet)", i, 0);
			_r_listviewinsertitem(hwnd, ctrl, buffer, i++, 1);
		}
	}

	// RAS
	if(5)
	{
		if(_Errlib_FormatMessage(code, L"mprmsg.dll", buffer, length))
		{
			_r_listviewinsertitem(hwnd, ctrl, L"Remote Access Service (RAS)", i, 0);
			_r_listviewinsertitem(hwnd, ctrl, buffer, i++, 1);
		}
	}

	// PDH
	if(6)
	{
		if(_Errlib_FormatMessage(code, L"pdh.dll", buffer, length))
		{
			_r_listviewinsertitem(hwnd, ctrl, L"Performance Data Helper (PDH)", i, 0);
			_r_listviewinsertitem(hwnd, ctrl, buffer, i++, 1);
		}
	}

	// IP Helper
	if(7)
	{
		if(_Errlib_FormatMessage(code, L"iphlpapi.dll", buffer, length))
		{
			_r_listviewinsertitem(hwnd, ctrl, L"IP Helper", i, 0);
			_r_listviewinsertitem(hwnd, ctrl, buffer, i++, 1);
		}
	}

	// BSOD
	if(8)
	{
		if(_Errlib_FormatMessage(code, L"ntoskrnl.dll", buffer, length))
		{
			_r_listviewinsertitem(hwnd, ctrl, L"Blue Screen of Dead (STOP)", i, 0);
			_r_listviewinsertitem(hwnd, ctrl, buffer, i++, 1);
		}
	}

	WCHAR* next = NULL;
	WCHAR custom[MAX_PATH] = {0};

	StringCchCopy(custom, MAX_PATH, _r_cfg_read(APP_NAME_SHORT, L"ModuleCustom", LPCWSTR(NULL)));

	wchar_t* tok = wcstok_s(custom, L";", &next);

	while(tok != NULL)
	{

		if(_Errlib_FormatMessage(code, tok, buffer, length))
		{
			_r_listviewinsertitem(hwnd, ctrl, tok, i, 0);
			_r_listviewinsertitem(hwnd, ctrl, buffer, i++, 1);
		}

		tok = wcstok_s(NULL, L";", &next);
	}

}

VOID _Errlib_PrintCode(HWND hwnd, DWORD code)
{
	WCHAR buffer[MAX_PATH] = {0};

	// Decimal
#ifdef _WIN64
	StringCchPrintf(buffer, MAX_PATH, L"%lld\0", code);
#else
	StringCchPrintf(buffer, MAX_PATH, L"%u\0", code);
#endif

	SetDlgItemText(hwnd, IDC_DESCRIPTION_1, buffer);
	 
	// Hexadecimal
#ifdef _WIN64
	StringCchPrintf(buffer, MAX_PATH, L"0x%08llx\0", code);
#else
	StringCchPrintf(buffer, MAX_PATH, L"0x%08x\0", code);
#endif

	SetDlgItemText(hwnd, IDC_DESCRIPTION_2, buffer);

	// Severity
	switch(HRESULT_SEVERITY(code))
	{
		SWITCH_ROUTINE(STATUS_SEVERITY_SUCCESS)
		SWITCH_ROUTINE(STATUS_SEVERITY_INFORMATIONAL)
		SWITCH_ROUTINE(STATUS_SEVERITY_WARNING)
		SWITCH_ROUTINE(STATUS_SEVERITY_ERROR)

		default:
		{
			StringCchCopy(buffer, MAX_PATH, NULL_STRING);
			break;
		}
	}

	SetDlgItemText(hwnd, IDC_DESCRIPTION_3, buffer);

	// Facility
	switch(HRESULT_FACILITY(code))
	{
		SWITCH_ROUTINE(FACILITY_NULL)
		SWITCH_ROUTINE(FACILITY_RPC)
		SWITCH_ROUTINE(FACILITY_DISPATCH)
		SWITCH_ROUTINE(FACILITY_STORAGE)
		SWITCH_ROUTINE(FACILITY_ITF)
		SWITCH_ROUTINE(FACILITY_WIN32)
		SWITCH_ROUTINE(FACILITY_WINDOWS)
		SWITCH_ROUTINE(FACILITY_SSPI) // FACILITY_SECURITY
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
		SWITCH_ROUTINE(FACILITY_AUDIO_KERNEL)
		SWITCH_ROUTINE(FACILITY_WINDOWS_DEFENDER)
		SWITCH_ROUTINE(FACILITY_OPC)

		SWITCH_ROUTINE(FACILITY_BTH_ATT)
		SWITCH_ROUTINE(FACILITY_CODCLASS_ERROR_CODE)
		SWITCH_ROUTINE(FACILITY_FILTER_MANAGER)
		SWITCH_ROUTINE(FACILITY_GRAPHICS_KERNEL)
		SWITCH_ROUTINE(FACILITY_INTERIX)
		SWITCH_ROUTINE(FACILITY_MAXIMUM_VALUE)
		SWITCH_ROUTINE(FACILITY_MONITOR)
		SWITCH_ROUTINE(FACILITY_WIN32K_NTGDI)
		SWITCH_ROUTINE(FACILITY_WIN32K_NTUSER)
		SWITCH_ROUTINE(FACILITY_RDBSS)
		SWITCH_ROUTINE(FACILITY_RESUME_KEY_FILTER)
		SWITCH_ROUTINE(FACILITY_SECUREBOOT)
		SWITCH_ROUTINE(FACILITY_SHARED_VHDX)
		SWITCH_ROUTINE(FACILITY_SPACES)
		SWITCH_ROUTINE(FACILITY_VIDEO)

		SWITCH_ROUTINE(FACILITY_MBN)
		SWITCH_ROUTINE(FACILITY_RAS)
		SWITCH_ROUTINE(FACILITY_UI)
		SWITCH_ROUTINE(FACILITY_XPS)

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

		SWITCH_ROUTINE(FACILITY_XAML)
		SWITCH_ROUTINE(FACILITY_USN)
		SWITCH_ROUTINE(FACILITY_BLBUI)
		SWITCH_ROUTINE(FACILITY_SPP)
		SWITCH_ROUTINE(FACILITY_WSB_ONLINE)
		SWITCH_ROUTINE(FACILITY_BLB_CLI)
		SWITCH_ROUTINE(FACILITY_BLB)
		SWITCH_ROUTINE(FACILITY_WSBAPP)
		SWITCH_ROUTINE(FACILITY_WMAAECMA)
		SWITCH_ROUTINE(FACILITY_WEP)
		SWITCH_ROUTINE(FACILITY_WEB_SOCKET)
		SWITCH_ROUTINE(FACILITY_WEB)
		SWITCH_ROUTINE(FACILITY_USERMODE_VOLSNAP)
		SWITCH_ROUTINE(FACILITY_VISUALCPP)
		SWITCH_ROUTINE(FACILITY_TIERING)
		SWITCH_ROUTINE(FACILITY_SYNCENGINE)
		SWITCH_ROUTINE(FACILITY_SOS)
		SWITCH_ROUTINE(FACILITY_USERMODE_SDBUS)
		SWITCH_ROUTINE(FACILITY_SCRIPT)
		SWITCH_ROUTINE(FACILITY_PARSE)
		SWITCH_ROUTINE(FACILITY_PIDGENX)
		SWITCH_ROUTINE(FACILITY_P2P_INT)
		SWITCH_ROUTINE(FACILITY_P2P)
		SWITCH_ROUTINE(FACILITY_ONLINE_ID)
		SWITCH_ROUTINE(FACILITY_MOBILE)
		SWITCH_ROUTINE(FACILITY_LINGUISTIC_SERVICES)
		SWITCH_ROUTINE(FACILITY_JSCRIPT)
		SWITCH_ROUTINE(FACILITY_ACCELERATOR)
		SWITCH_ROUTINE(FACILITY_EAS)
		SWITCH_ROUTINE(FACILITY_DXGI_DDI)
		SWITCH_ROUTINE(FACILITY_DAF)
		SWITCH_ROUTINE(FACILITY_DEPLOYMENT_SERVICES_UTIL)
		SWITCH_ROUTINE(FACILITY_DEPLOYMENT_SERVICES_TRANSPORT_MANAGEMENT)
		SWITCH_ROUTINE(FACILITY_DEPLOYMENT_SERVICES_TFTP)
		SWITCH_ROUTINE(FACILITY_DEPLOYMENT_SERVICES_PXE)
		SWITCH_ROUTINE(FACILITY_DEPLOYMENT_SERVICES_MULTICAST_SERVER)
		SWITCH_ROUTINE(FACILITY_DEPLOYMENT_SERVICES_MULTICAST_CLIENT)
		SWITCH_ROUTINE(FACILITY_DEPLOYMENT_SERVICES_MANAGEMENT)
		SWITCH_ROUTINE(FACILITY_DEPLOYMENT_SERVICES_IMAGING)
		SWITCH_ROUTINE(FACILITY_DEPLOYMENT_SERVICES_DRIVER_PROVISIONING)
		SWITCH_ROUTINE(FACILITY_DEPLOYMENT_SERVICES_SERVER)
		SWITCH_ROUTINE(FACILITY_DEPLOYMENT_SERVICES_CONTENT_PROVIDER)
		SWITCH_ROUTINE(FACILITY_DEPLOYMENT_SERVICES_BINLSVC)
		SWITCH_ROUTINE(FACILITY_DEFRAG)
		SWITCH_ROUTINE(FACILITY_DEBUGGERS)
		SWITCH_ROUTINE(FACILITY_BLUETOOTH_ATT)
		SWITCH_ROUTINE(FACILITY_AUDIOSTREAMING)
		SWITCH_ROUTINE(FACILITY_AUDIO)
		SWITCH_ROUTINE(FACILITY_ACTION_QUEUE)

		SWITCH_ROUTINE(FACILITY_EAP_MESSAGE)

		default:
		{
			StringCchCopy(buffer, MAX_PATH, NULL_STRING);
			break;
		}
	}

	SetDlgItemText(hwnd, IDC_DESCRIPTION_4, buffer);
}

VOID _Errlib_Clear(HWND hwnd)
{
	SendDlgItemMessage(hwnd, IDC_LISTVIEW, LVM_DELETEALLITEMS, 0, NULL);

	SetDlgItemText(hwnd, IDC_DESCRIPTION_1, NULL_STRING);
	SetDlgItemText(hwnd, IDC_DESCRIPTION_2, NULL_STRING);
	SetDlgItemText(hwnd, IDC_DESCRIPTION_3, NULL_STRING);
	SetDlgItemText(hwnd, IDC_DESCRIPTION_4, NULL_STRING);
}

INT_PTR WINAPI PagesDlgProc(HWND hwnd, UINT msg, WPARAM, LPARAM lparam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			SetProp(hwnd, L"id", (HANDLE)lparam);

			if((INT)lparam == IDD_SETTINGS_1)
			{
				CheckDlgButton(hwnd, IDC_ALWAYSONTOP_CHK, _r_cfg_read(APP_NAME_SHORT, L"AlwaysOnTop", 0) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwnd, IDC_INSERTBUFFER_CHK, _r_cfg_read(APP_NAME_SHORT, L"InsertBufferAtStartup", 1) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwnd, IDC_CHECKUPDATES_CHK, _r_cfg_read(APP_NAME_SHORT, L"CheckUpdates", 1) ? BST_CHECKED : BST_UNCHECKED);

				switch(_r_cfg_read(APP_NAME_SHORT, L"InputType", 0))
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

				WCHAR buffer[MAX_PATH] = {0};

				SendDlgItemMessage(hwnd, IDC_LANGUAGE, CB_INSERTSTRING, 0, (LPARAM)L"System default");
				SendDlgItemMessage(hwnd, IDC_LANGUAGE, CB_SETCURSEL, 0, 0);

				for(INT i = 0; i < ARRAYSIZE(ui_lcid); i++)
				{
					GetLocaleInfo(ui_lcid[i], _r_systemvalidversion(6, 1) ? LOCALE_SLOCALIZEDLANGUAGENAME : LOCALE_SLANGUAGE, buffer, MAX_PATH);

					SendDlgItemMessage(hwnd, IDC_LANGUAGE, CB_INSERTSTRING, i + 1, (LPARAM)buffer);
					SendDlgItemMessage(hwnd, IDC_LANGUAGE, CB_SETITEMDATA, i + 1, (LPARAM)ui_lcid[i]);

					if(ui_lcid[i] == _r_lcid)
					{
						SendDlgItemMessage(hwnd, IDC_LANGUAGE, CB_SETCURSEL, i + 1, 0);
					}
				}
			}
			else if((INT)lparam == IDD_SETTINGS_2)
			{
				SetDlgItemText(hwnd, IDC_MODULE_CUSTOM, _r_cfg_read(APP_NAME_SHORT, L"ModuleCustom", LPCWSTR(NULL)));
			}

			break;
		}

		case WM_DESTROY:
		{
			// save settings if this property is empty
			if(GetProp(GetParent(hwnd), L"is_save"))
			{
				if(INT(GetProp(hwnd, L"id")) == IDD_SETTINGS_1)
				{
					BOOL top = (IsDlgButtonChecked(hwnd, IDC_ALWAYSONTOP_CHK) == BST_CHECKED) ? TRUE : FALSE;

					_r_windowtotop(_r_hwnd, top);

					_r_cfg_write(APP_NAME_SHORT, L"AlwaysOnTop", INT(top));
					_r_cfg_write(APP_NAME_SHORT, L"InsertBufferAtStartup", INT((IsDlgButtonChecked(hwnd, IDC_INSERTBUFFER_CHK) == BST_CHECKED) ? TRUE : FALSE));
					_r_cfg_write(APP_NAME_SHORT, L"CheckUpdates", INT((IsDlgButtonChecked(hwnd, IDC_CHECKUPDATES_CHK) == BST_CHECKED) ? TRUE : FALSE));

					if(IsDlgButtonChecked(hwnd, IDC_TYPE_DEC) == BST_CHECKED)
					{
						_r_cfg_write(APP_NAME_SHORT, L"InputType", 1);
					}
					else if(IsDlgButtonChecked(hwnd, IDC_TYPE_HEX) == BST_CHECKED)
					{
						_r_cfg_write(APP_NAME_SHORT, L"InputType", 2);
					}
					else
					{
						_r_cfg_write(APP_NAME_SHORT, L"InputType", DWORD(0));
					}

					LCID lang = (LCID)SendDlgItemMessage(hwnd, IDC_LANGUAGE, CB_GETITEMDATA, SendDlgItemMessage(hwnd, IDC_LANGUAGE, CB_GETCURSEL, 0, NULL), NULL);

					if(lang <= 0)
					{
						lang = NULL;
					}

					_r_locale_set(lang);
				}
				else if(INT(GetProp(hwnd, L"id")) == IDD_SETTINGS_2)
				{
					WCHAR buffer[MAX_PATH] = {0};

					GetDlgItemText(hwnd, IDC_MODULE_CUSTOM, buffer, MAX_PATH);

					_r_cfg_write(APP_NAME_SHORT, L"ModuleCustom", buffer);
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
			_r_treeviewsetstyle(hwnd, IDC_NAV, TVS_EX_DOUBLEBUFFER, 18);

			_r_treeviewinsertitem(hwnd, IDC_NAV, (LPWSTR)_r_locale(IDS_SETTINGS_1), -1, (LPARAM)CreateDialogParam(NULL, MAKEINTRESOURCE(IDD_SETTINGS_1), hwnd, PagesDlgProc, IDD_SETTINGS_1));
			_r_treeviewinsertitem(hwnd, IDC_NAV, (LPWSTR)_r_locale(IDS_SETTINGS_2), -1, (LPARAM)CreateDialogParam(NULL, MAKEINTRESOURCE(IDD_SETTINGS_2), hwnd, PagesDlgProc, IDD_SETTINGS_2));

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
					SetProp(hwnd, L"is_save", (HANDLE)TRUE); // save settings indicator

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
			_r_listviewsetstyle(hwnd, IDC_LISTVIEW, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP, FALSE);

			_r_listviewinsertcolumn(hwnd, IDC_LISTVIEW, _r_locale(IDS_COLUMN_1), _r_cfg_read(APP_NAME_SHORT, L"Column1", INT(190)), 0, LVCFMT_LEFT);
			_r_listviewinsertcolumn(hwnd, IDC_LISTVIEW, _r_locale(IDS_COLUMN_2), _r_cfg_read(APP_NAME_SHORT, L"Column2", INT(370)), 1, LVCFMT_LEFT);

			_r_windowtotop(hwnd, _r_cfg_read(APP_NAME_SHORT, L"AlwaysOnTop", 0));

			_Errlib_Clear(hwnd);

			if(_r_cfg_read(APP_NAME_SHORT, L"InsertBufferAtStartup", 1))
			{
				SetDlgItemText(hwnd, IDC_CODE, _r_clipboard_get());
				SendMessage(hwnd, WM_COMMAND, MAKELPARAM(IDC_GET, 0), NULL);
			}

			break;
		}

		case WM_DESTROY:
		{
			_r_cfg_write(APP_NAME_SHORT, L"Column1", (DWORD)SendDlgItemMessage(hwnd, IDC_LISTVIEW, LVM_GETCOLUMNWIDTH, 0, 0));
			_r_cfg_write(APP_NAME_SHORT, L"Column2", (DWORD)SendDlgItemMessage(hwnd, IDC_LISTVIEW, LVM_GETCOLUMNWIDTH, 1, 0));

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

					_Errlib_PrintCode(hwnd, code);
					_Errlib_PrintDescription(hwnd, IDC_LISTVIEW, code);

					break;
				}

				case IDM_CLEAR:
				{
					_Errlib_Clear(hwnd);
					break;
				}

				case IDM_WEBSITE:
				{
					ShellExecute(hwnd, NULL, APP_WEBSITE L"/product/" APP_NAME_SHORT, NULL, NULL, 0);
					break;
				}

				case IDM_CHECKUPDATES:
				{
					_r_updatecheck(FALSE);
					break;
				}

				case IDM_ABOUT:
				{
					_r_aboutbox(hwnd);
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
	if(_r_initialize((DLGPROC)DlgProc))
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

	_r_uninitialize();

	return ERROR_SUCCESS;
}