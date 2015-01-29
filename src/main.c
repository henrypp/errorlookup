// Error Lookup
// Copyright © 2011, 2012, 2015 Henry++

#include <windows.h>

#include "main.h"

#include "resource.h"
#include "routine.h"

#include <dxerr.h> // direct x

#define NULL_STRING L"(null)"
#define CUSTOM_DEFAULT L"asferror.dll; blbres.dll; comres.dll; crypt32.dll; ddputils.dll; dhcpcore.dll; dhcpcore6.dll; dhcpsapi.dll; dmutil.dll; efscore.dll; imapi2.dll; imapi2fs.dll; iphlpapi.dll; ipnathlp.dll; kerberos.dll; loadperf.dll; mferror.dll; mprmsg.dll; msobjs.dll; mswsock.dll; msxml3r.dll; msxml6r.dll; ntprint.dll; ntshrui.dll; ole32.dll; p2p.dll; pdh.dll; pshed.dll; qmgr.dll; rpcrt4.dll; schedsvc.dll; twinui.dll; winbio.dll; winhttp.dll; wininet.dll; wsock32.dll;"

DWORD _Errlib_GetCode(HWND hwnd, INT ctrl)
{
	WCHAR buffer[100] = {0};
	DWORD result = 0;

	if(GetDlgItemText(hwnd, ctrl, buffer, 100))
	{
		if((result = wcstoul(buffer, NULL, 10)) == NULL)
		{
			result = wcstoul(buffer, NULL, 16);
		}
	}

	return result;
}

BOOL _Errlib_FormatMessage(DWORD code, LPCWSTR library, LPWSTR buffer, const DWORD length, BOOL localized = TRUE)
{
	BOOL result = FALSE;

	HMODULE h = LoadLibraryEx(library, NULL, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);
	HLOCAL out = NULL;

	if(h)
	{
		if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS, h, code, localized ? _r_lcid : 0, (LPWSTR)&out, 0, NULL))
		{
			StringCchCopy(buffer, length, (LPCWSTR)out);

			if(wcsncmp(buffer, L"%1", 2) != 0)
			{
				if(buffer[wcslen(buffer) - 1] == 10)
				{
					buffer[wcslen(buffer) - sizeof(WCHAR)] = 0;
				}

				result = TRUE;
			}
		}
		else
		{
			if(localized)
			{
				result = _Errlib_FormatMessage(code, library, buffer, length, FALSE);
			}
		}

		LocalFree(out);
		FreeLibrary(h);
	}

	return result;
}

VOID _Errlib_Insert(HWND hwnd, INT ctrl, INT i18n, LPCWSTR description, LPCWSTR module = NULL, INT group_id = 0)
{
	INT item = (INT)SendDlgItemMessage(hwnd, ctrl, LVM_GETITEMCOUNT, 0, NULL);

	_r_listview_additem(hwnd, ctrl, i18n ? _r_locale(i18n) : module, item, 0, -1, group_id);
	_r_listview_additem(hwnd, ctrl, description, item, 1);
}

VOID _Errlib_PrintDescription(HWND hwnd, INT ctrl, DWORD code)
{
	WCHAR buffer[1024] = {0};
	const DWORD length = 1024;

	// Internal modules
	if(_r_cfg_read(APP_NAME_SHORT, L"ModuleKernel32", 1))
	{
		if(_Errlib_FormatMessage(code, L"kernel32.dll", buffer, length))
		{
			_Errlib_Insert(hwnd, ctrl, IDS_MODULE_KERNEL32, buffer);
		}
	}

	if(_r_cfg_read(APP_NAME_SHORT, L"ModuleNtdll", 1))
	{
		if(_Errlib_FormatMessage(code, L"ntdll.dll", buffer, length))
		{
			_Errlib_Insert(hwnd, ctrl, IDS_MODULE_NTDLL, buffer);
		}
	}
	
	if(_r_cfg_read(APP_NAME_SHORT, L"ModuleNtoskrnl", 1))
	{
		if(_Errlib_FormatMessage(code, L"ntoskrnl.exe", buffer, length))
		{
			_Errlib_Insert(hwnd, ctrl, IDS_MODULE_NTOSKRNL, buffer);
		}
	}

	if(_r_cfg_read(APP_NAME_SHORT, L"ModuleMsimsg", 1))
	{
		if(_Errlib_FormatMessage(code, L"msimsg.dll", buffer, length))
		{
			_Errlib_Insert(hwnd, ctrl, IDS_MODULE_MSIMSG, buffer);
		}
	}

	if(_r_cfg_read(APP_NAME_SHORT, L"ModuleWmerror", 1))
	{
		if(_Errlib_FormatMessage(code, L"wmerror.dll", buffer, length))
		{
			_Errlib_Insert(hwnd, ctrl, IDS_MODULE_WMERROR, buffer);
		}
	}

	if(_r_cfg_read(APP_NAME_SHORT, L"ModuleDirectX", 1))
	{
		StringCchCopy(buffer, (size_t)length, DXGetErrorDescription(HRESULT(code)));

		if(wcscmp(buffer, L"n/a\0") != 0)
		{
			_Errlib_Insert(hwnd, ctrl, IDS_MODULE_DIRECTX, buffer);
		}
	}
	
	if(_r_cfg_read(APP_NAME_SHORT, L"ModuleMpssvc", 1))
	{
		if(_Errlib_FormatMessage(code, L"mpssvc.dll", buffer, length))
		{
			_Errlib_Insert(hwnd, ctrl, IDS_MODULE_MPSSVC, buffer);
		}
	}
	
	if(_r_cfg_read(APP_NAME_SHORT, L"ModuleAdtschema", 1))
	{
		if(_Errlib_FormatMessage(code, L"adtschema.dll", buffer, length))
		{
			_Errlib_Insert(hwnd, ctrl, IDS_MODULE_ADTSCHEMA, buffer);
		}
	}
	
	if(_r_cfg_read(APP_NAME_SHORT, L"ModuleNetmsg", 1))
	{
		if(_Errlib_FormatMessage(code, L"netmsg.dll", buffer, length))
		{
			_Errlib_Insert(hwnd, ctrl, IDS_MODULE_NETMSG, buffer);
		}
	}

	if(_r_cfg_read(APP_NAME_SHORT, L"ModuleNetevent", 1))
	{
		if(_Errlib_FormatMessage(code, L"netevent.dll", buffer, length))
		{
			_Errlib_Insert(hwnd, ctrl, IDS_MODULE_NETEVENT, buffer);
		}
	}

	// Custom modules
	std::wstring modules = _r_cfg_read(APP_NAME_SHORT, L"Modules", CUSTOM_DEFAULT);
	std::wstring module;

	LPWSTR next = NULL, modules_dup = _wcsdup(modules.c_str()), token = wcstok_s(modules_dup, L";", &next);

	while(token != NULL)
	{
		SecureZeroMemory(buffer, MAX_PATH * sizeof(WCHAR));

		module = token;

		module.erase(0, module.find_first_not_of(L' '));
		module.erase(module.find_last_not_of(L' ') + 1);

		if(_Errlib_FormatMessage(code, module.c_str(), buffer, length))
		{
			_Errlib_Insert(hwnd, ctrl, NULL, buffer, module.c_str(), 1);
		}

		token = wcstok_s(NULL, L";", &next);
	}

	free(token);
	free(modules_dup);
}

VOID _Errlib_PrintCode(HWND hwnd)
{
	WCHAR buffer[MAX_PATH] = {0};
	DWORD code = _Errlib_GetCode(hwnd, IDC_CODE);

	SendDlgItemMessage(hwnd, IDC_LISTVIEW, LVM_DELETEALLITEMS, 0, NULL);

	_Errlib_PrintDescription(hwnd, IDC_LISTVIEW, code);

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

BOOL CALLBACK EnumResourceLanguagesCallback(HMODULE, LPCWSTR, LPCWSTR, WORD language, LONG_PTR lparam)
{
	WCHAR buffer[MAX_PATH] = {0};

	INT item = max(0, (INT)SendMessage((HWND)lparam, CB_GETCOUNT, 0, NULL));

	if(GetLocaleInfo(language, _r_system_validversion(6, 1) ? LOCALE_SENGLISHDISPLAYNAME : LOCALE_SLANGUAGE, buffer, MAX_PATH))
	{
		SendMessage((HWND)lparam, CB_INSERTSTRING, item, (LPARAM)buffer);
		SendMessage((HWND)lparam, CB_SETITEMDATA, item, (LPARAM)language);

		if(language == _r_lcid)
		{
			SendMessage((HWND)lparam, CB_SETCURSEL, item, 0);
		}
	}

	return TRUE;
}

INT_PTR WINAPI PagesDlgProc(HWND hwnd, UINT msg, WPARAM, LPARAM lparam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			SetProp(hwnd, L"id", (HANDLE)lparam);

			RECT rc = {0};

			GetWindowRect(GetDlgItem(GetParent(hwnd), IDC_NAV), &rc);
			MapWindowPoints(NULL, GetParent(hwnd), (LPPOINT)&rc, 2);

			SetWindowPos(hwnd, HWND_TOP, (rc.right - rc.left) + rc.left * 2, rc.top, 0, 0, SWP_NOSIZE);

			if((INT)lparam == IDD_SETTINGS_1)
			{
				CheckDlgButton(hwnd, IDC_ALWAYSONTOP_CHK, _r_cfg_read(APP_NAME_SHORT, L"AlwaysOnTop", 0) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwnd, IDC_INSERTBUFFER_CHK, _r_cfg_read(APP_NAME_SHORT, L"InsertBufferAtStartup", 1) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwnd, IDC_CHECKUPDATES_CHK, _r_cfg_read(APP_NAME_SHORT, L"CheckUpdates", 1) ? BST_CHECKED : BST_UNCHECKED);

				SendDlgItemMessage(hwnd, IDC_LANGUAGE, CB_INSERTSTRING, 0, (LPARAM)L"System default");
				SendDlgItemMessage(hwnd, IDC_LANGUAGE, CB_SETCURSEL, 0, 0);

				EnumResourceLanguages(NULL, RT_STRING, MAKEINTRESOURCE(626), EnumResourceLanguagesCallback, (LONG_PTR)GetDlgItem(hwnd, IDC_LANGUAGE));

			}
			else if((INT)lparam == IDD_SETTINGS_2)
			{
				for (INT i = IDC_MODULE_KERNEL32, j = IDS_MODULE_KERNEL32; i < IDC_MODULE_NETEVENT + 1; i++, j++)
				{
					SetDlgItemText(hwnd, i, _r_locale(j));
				}

				CheckDlgButton(hwnd, IDC_MODULE_KERNEL32, _r_cfg_read(APP_NAME_SHORT, L"ModuleKernel32", 1) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwnd, IDC_MODULE_NTDLL, _r_cfg_read(APP_NAME_SHORT, L"ModuleNtdll", 1) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwnd, IDC_MODULE_NTOSKRNL, _r_cfg_read(APP_NAME_SHORT, L"ModuleNtoskrnl", 1) ? BST_CHECKED : BST_UNCHECKED);

				CheckDlgButton(hwnd, IDC_MODULE_MSIMSG, _r_cfg_read(APP_NAME_SHORT, L"ModuleMsimsg", 1) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwnd, IDC_MODULE_WMERROR, _r_cfg_read(APP_NAME_SHORT, L"ModuleWmerror", 1) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwnd, IDC_MODULE_DIRECTX, _r_cfg_read(APP_NAME_SHORT, L"ModuleDirectX", 1) ? BST_CHECKED : BST_UNCHECKED);

				CheckDlgButton(hwnd, IDC_MODULE_MPSSVC, _r_cfg_read(APP_NAME_SHORT, L"ModuleMpssvc", 1) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwnd, IDC_MODULE_ADTSCHEMA, _r_cfg_read(APP_NAME_SHORT, L"ModuleAdtschema", 1) ? BST_CHECKED : BST_UNCHECKED);

				CheckDlgButton(hwnd, IDC_MODULE_NETMSG, _r_cfg_read(APP_NAME_SHORT, L"ModuleNetmsg", 1) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwnd, IDC_MODULE_NETEVENT, _r_cfg_read(APP_NAME_SHORT, L"ModuleNetevent", 1) ? BST_CHECKED : BST_UNCHECKED);
			}
			else if((INT)lparam == IDD_SETTINGS_3)
			{
				SetDlgItemText(hwnd, IDC_MODULE_LIST, _r_cfg_read(APP_NAME_SHORT, L"Modules", CUSTOM_DEFAULT).c_str());
			}

			break;
		}

		case WM_DESTROY:
		{
			if(GetProp(GetParent(hwnd), L"is_save"))
			{
				if(INT(GetProp(hwnd, L"id")) == IDD_SETTINGS_1)
				{
					_r_windowtotop(_r_hwnd, (IsDlgButtonChecked(hwnd, IDC_ALWAYSONTOP_CHK) == BST_CHECKED) ? TRUE : FALSE);

					// general
					_r_cfg_write(APP_NAME_SHORT, L"AlwaysOnTop", INT((IsDlgButtonChecked(hwnd, IDC_ALWAYSONTOP_CHK) == BST_CHECKED) ? TRUE : FALSE));
					_r_cfg_write(APP_NAME_SHORT, L"InsertBufferAtStartup", INT((IsDlgButtonChecked(hwnd, IDC_INSERTBUFFER_CHK) == BST_CHECKED) ? TRUE : FALSE));
					_r_cfg_write(APP_NAME_SHORT, L"CheckUpdates", INT((IsDlgButtonChecked(hwnd, IDC_CHECKUPDATES_CHK) == BST_CHECKED) ? TRUE : FALSE));

					// language
					LCID lang = (LCID)SendDlgItemMessage(hwnd, IDC_LANGUAGE, CB_GETITEMDATA, SendDlgItemMessage(hwnd, IDC_LANGUAGE, CB_GETCURSEL, 0, NULL), NULL);

					if(lang <= 0)
					{
						lang = NULL;
					}

					SetProp(_r_hwnd, L"is_restart", (HANDLE)((lang != _r_lcid) ? TRUE : FALSE));

					_r_locale_set(lang);
				}
				else if(INT(GetProp(hwnd, L"id")) == IDD_SETTINGS_2)
				{
					_r_cfg_write(APP_NAME_SHORT, L"ModuleKernel32", INT((IsDlgButtonChecked(hwnd, IDC_MODULE_KERNEL32) == BST_CHECKED) ? TRUE : FALSE));
					_r_cfg_write(APP_NAME_SHORT, L"ModuleNtdll", INT((IsDlgButtonChecked(hwnd, IDC_MODULE_NTDLL) == BST_CHECKED) ? TRUE : FALSE));
					_r_cfg_write(APP_NAME_SHORT, L"ModuleNtoskrnl", INT((IsDlgButtonChecked(hwnd, IDC_MODULE_NTOSKRNL) == BST_CHECKED) ? TRUE : FALSE));

					_r_cfg_write(APP_NAME_SHORT, L"ModuleMsimsg", INT((IsDlgButtonChecked(hwnd, IDC_MODULE_MSIMSG) == BST_CHECKED) ? TRUE : FALSE));
					_r_cfg_write(APP_NAME_SHORT, L"ModuleWmerror", INT((IsDlgButtonChecked(hwnd, IDC_MODULE_WMERROR) == BST_CHECKED) ? TRUE : FALSE));
					_r_cfg_write(APP_NAME_SHORT, L"ModuleDirectX", INT((IsDlgButtonChecked(hwnd, IDC_MODULE_DIRECTX) == BST_CHECKED) ? TRUE : FALSE));

					_r_cfg_write(APP_NAME_SHORT, L"ModuleMpssvc", INT((IsDlgButtonChecked(hwnd, IDC_MODULE_MPSSVC) == BST_CHECKED) ? TRUE : FALSE));
					_r_cfg_write(APP_NAME_SHORT, L"ModuleAdtschema", INT((IsDlgButtonChecked(hwnd, IDC_MODULE_ADTSCHEMA) == BST_CHECKED) ? TRUE : FALSE));

					_r_cfg_write(APP_NAME_SHORT, L"ModuleNetmsg", INT((IsDlgButtonChecked(hwnd, IDC_MODULE_NETMSG) == BST_CHECKED) ? TRUE : FALSE));
					_r_cfg_write(APP_NAME_SHORT, L"ModuleNetevent", INT((IsDlgButtonChecked(hwnd, IDC_MODULE_NETEVENT) == BST_CHECKED) ? TRUE : FALSE));
				}
				else if(INT(GetProp(hwnd, L"id")) == IDD_SETTINGS_3)
				{
					INT length = (INT)SendDlgItemMessage(hwnd, IDC_MODULE_LIST, WM_GETTEXTLENGTH, 0, NULL) + sizeof(WCHAR);

					LPWSTR buffer = (LPWSTR)malloc(length * sizeof(WCHAR));

					if(buffer)
					{
						GetDlgItemText(hwnd, IDC_MODULE_LIST, buffer, length);

						_r_cfg_write(APP_NAME_SHORT, L"Modules", buffer);
					}

					free(buffer);
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
			_r_treeview_setstyle(hwnd, IDC_NAV, TVS_EX_DOUBLEBUFFER, 18);

			_r_treeview_additem(hwnd, IDC_NAV, (LPWSTR)_r_locale(IDS_SETTINGS_1), -1, (LPARAM)CreateDialogParam(NULL, MAKEINTRESOURCE(IDD_SETTINGS_1), hwnd, PagesDlgProc, IDD_SETTINGS_1));
			_r_treeview_additem(hwnd, IDC_NAV, (LPWSTR)_r_locale(IDS_SETTINGS_2), -1, (LPARAM)CreateDialogParam(NULL, MAKEINTRESOURCE(IDD_SETTINGS_2), hwnd, PagesDlgProc, IDD_SETTINGS_2));
			_r_treeview_additem(hwnd, IDC_NAV, (LPWSTR)_r_locale(IDS_SETTINGS_3), -1, (LPARAM)CreateDialogParam(NULL, MAKEINTRESOURCE(IDD_SETTINGS_3), hwnd, PagesDlgProc, IDD_SETTINGS_3));

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
			_r_listview_setstyle(hwnd, IDC_LISTVIEW, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP, TRUE);

			_r_listview_addcolumn(hwnd, IDC_LISTVIEW, _r_locale(IDS_COLUMN_1), _r_cfg_read(APP_NAME_SHORT, L"Column1", INT(190)), 0, LVCFMT_LEFT);
			_r_listview_addcolumn(hwnd, IDC_LISTVIEW, _r_locale(IDS_COLUMN_2), _r_cfg_read(APP_NAME_SHORT, L"Column2", INT(370)), 1, LVCFMT_LEFT);

			_r_listview_addgroup(hwnd, IDC_LISTVIEW, 0, _r_locale(IDS_GROUP_1), 0, 0);
			_r_listview_addgroup(hwnd, IDC_LISTVIEW, 1, _r_locale(IDS_GROUP_2), 0, 0);

			_r_windowtotop(hwnd, _r_cfg_read(APP_NAME_SHORT, L"AlwaysOnTop", INT(0)));

			SendDlgItemMessage(hwnd, IDC_CODE_UD, UDM_SETRANGE32, 0, 0xfffffff);

			if(_r_cfg_read(APP_NAME_SHORT, L"InsertBufferAtStartup", INT(1)))
			{
				SetDlgItemText(hwnd, IDC_CODE, _r_clipboard_get().c_str());
			}
			else
			{
				SetDlgItemInt(hwnd, IDC_CODE, 0, TRUE);
			}

			_Errlib_PrintCode(hwnd);

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
			if(HIWORD(wparam) == EN_CHANGE && LOWORD(wparam) == IDC_CODE)
			{
				_Errlib_PrintCode(hwnd);
				break;
			}

			switch(LOWORD(wparam))
			{
				case IDM_SETTINGS:
				{
					DialogBox(NULL, MAKEINTRESOURCE(IDD_SETTINGS), hwnd, SettingsDlgProc);

					if(GetProp(hwnd, L"is_restart"))
					{
						_r_uninitialize(TRUE);
					}

					break;
				}

				case IDCANCEL: // process Esc key
				case IDM_EXIT:
				{
					DestroyWindow(hwnd);
					break;
				}

				case IDOK: // process Enter key
				{
					_Errlib_PrintCode(hwnd);
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

				case IDM_COPY:
				{
					WCHAR buffer[1024] = {0};
					std::wstring text;

					LVITEM lvi = {0};
					
					lvi.iSubItem = 1;
					lvi.cchTextMax = 1024;

					INT item = -1;

					while((item = (INT)SendDlgItemMessage(hwnd, IDC_LISTVIEW, LVM_GETNEXTITEM, item, LVNI_SELECTED)) != -1)
					{
						lvi.pszText = buffer;

						SendDlgItemMessage(hwnd, IDC_LISTVIEW, LVM_GETITEMTEXT, item, (LPARAM)&lvi);

						text.append(buffer);
						text.append(L"\r\n");
					}

					text.erase(text.find_last_not_of(L"\r\n") + 2);

					_r_clipboard_set(text.c_str(), text.length());
 
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

	_r_uninitialize(FALSE);

	return ERROR_SUCCESS;
}