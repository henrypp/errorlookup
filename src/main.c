// Error Lookup
// Copyright © 2011, 2012, 2015 Henry++
#include <windows.h>

#include "main.h"

#include "resource.h"
#include "routine.h"

#include <dxerr.h>

#define NULL_STRING L"(null)"
#define CUSTOM_DEFAULT L"asferror.dll; blbres.dll; comres.dll; crypt32.dll; ddputils.dll; dhcpcore.dll; dhcpcore6.dll; dhcpsapi.dll; dmutil.dll; efscore.dll; imapi2.dll; imapi2fs.dll; iphlpapi.dll; ipnathlp.dll; kerberos.dll; loadperf.dll; mferror.dll; mprmsg.dll; msobjs.dll; mswsock.dll; msxml3r.dll; msxml6r.dll; ntprint.dll; ntshrui.dll; ole32.dll; p2p.dll; pdh.dll; pshed.dll; qmgr.dll; rpcrt4.dll; schedsvc.dll; twinui.dll; winbio.dll; winhttp.dll; wininet.dll; wsock32.dll;"

DWORD _Errlib_ParseCode(HWND hwnd)
{
	WCHAR buffer[MAX_PATH] = {0};
	DWORD result = 0;

	if(GetDlgItemText(hwnd, IDC_CODE, buffer, MAX_PATH))
	{
		if((result = wcstoul(buffer, NULL, 10)) == NULL)
		{
			result = wcstoul(buffer, NULL, 16);
		}
	}

	// dec.
#ifdef _WIN64
	StringCchPrintf(buffer, MAX_PATH, L"%lld\0", result);
#else
	StringCchPrintf(buffer, MAX_PATH, L"%u\0", result);
#endif

	SetDlgItemText(hwnd, IDC_DESCRIPTION_1, buffer);

	// hex.
#ifdef _WIN64
	StringCchPrintf(buffer, MAX_PATH, L"0x%08llx\0", result);
#else
	StringCchPrintf(buffer, MAX_PATH, L"0x%08x\0", result);
#endif

	SetDlgItemText(hwnd, IDC_DESCRIPTION_2, buffer);

	// sev.
	switch(HRESULT_SEVERITY(result))
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

	// fac.
	switch(HRESULT_FACILITY(result))
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

	return result;
}

VOID _Errlib_Insert(HWND hwnd, INT i18n, LPCWSTR description, LPCWSTR module)
{
	WCHAR buffer[70] = {0};

	StringCchCopy(buffer, 70, description);

	buffer[66] = L'.';
	buffer[67] = L'.';
	buffer[68] = L'.';
	buffer[69] = L'\0';

	INT item = (INT)SendDlgItemMessage(hwnd, IDC_LISTVIEW, LVM_GETITEMCOUNT, 0, NULL);
	size_t length = wcslen(description);

	LPWSTR lparam = (LPWSTR)malloc((length + 1) * sizeof(WCHAR));

	if(lparam != NULL)
	{
		StringCchCopy(lparam, length + sizeof(WCHAR), description);
	}

	size_t pos = 0;

	std::wstring result = description;

	while((pos = result.find(L"\r\n")) != std::wstring::npos)
	{
		if(result[pos + 2] == L'\0' || result[pos + 2] == L' ')
		{
			result.erase(pos, 2);
		}
		else
		{
			result.replace(pos, 2, L" ");
		}
	}

	_r_listview_additem(hwnd, IDC_LISTVIEW, i18n ? _r_locale(i18n) : module, item, 0, -1, module ? 1 : 0, (LPARAM)lparam);
	_r_listview_additem(hwnd, IDC_LISTVIEW, buffer, item, 1);
}

VOID _Errlib_FormatMessage(HWND hwnd, DWORD code, LPCWSTR library, INT i18n, BOOL localized = TRUE)
{
	HMODULE h = LoadLibraryEx(library, NULL, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);
	HLOCAL buffer = NULL;

	if(h)
	{
		if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS, h, code, localized ? _r_lcid : 0, (LPWSTR)&buffer, 0, NULL))
		{
			std::wstring result = (LPCWSTR)buffer;

			if(wcsncmp(result.c_str(), L"%1", 2) != 0)
			{
				_Errlib_Insert(hwnd, i18n, result.c_str(), i18n ? NULL : library);
			}
		}
		else
		{
			if(localized)
			{
				_Errlib_FormatMessage(hwnd, code, library, i18n, FALSE);
			}
		}

		LocalFree(buffer);
		FreeLibrary(h);
	}
}

VOID _Errlib_Print(HWND hwnd)
{
	DWORD code = _Errlib_ParseCode(hwnd);

	// clear first
	SendDlgItemMessage(hwnd, IDC_LISTVIEW, LVM_DELETEALLITEMS, 0, NULL);

	//_wcserror_s

	// print (internal)
	if(_r_cfg_read(APP_NAME_SHORT, L"ModuleKernel32", 1))
	{
		_Errlib_FormatMessage(hwnd, code, L"kernel32.dll", IDS_MODULE_KERNEL32);
	}

	if(_r_cfg_read(APP_NAME_SHORT, L"ModuleNtdll", 1))
	{
		_Errlib_FormatMessage(hwnd, code, L"ntdll.dll", IDS_MODULE_NTDLL);
	}
	
	if(_r_cfg_read(APP_NAME_SHORT, L"ModuleNtoskrnl", 1))
	{
		_Errlib_FormatMessage(hwnd, code, L"ntoskrnl.exe", IDS_MODULE_NTOSKRNL);
	}

	if(_r_cfg_read(APP_NAME_SHORT, L"ModuleMsimsg", 1))
	{
		_Errlib_FormatMessage(hwnd, code, L"msimsg.dll", IDS_MODULE_MSIMSG);
	}

	if(_r_cfg_read(APP_NAME_SHORT, L"ModuleWmerror", 1))
	{
		_Errlib_FormatMessage(hwnd, code, L"wmerror.dll", IDS_MODULE_WMERROR);
	}

	if(_r_cfg_read(APP_NAME_SHORT, L"ModuleDirectX", 1))
	{
		std::wstring buffer = DXGetErrorDescription(HRESULT(code));

		if(wcsncmp(buffer.c_str(), L"n/a", 2) != 0)
		{
			_Errlib_Insert(hwnd, IDS_MODULE_DIRECTX, buffer.c_str(), NULL);
		}
	}

	if(_r_cfg_read(APP_NAME_SHORT, L"ModuleMpssvc", 1))
	{
		_Errlib_FormatMessage(hwnd, code, L"mpssvc.dll", IDS_MODULE_MPSSVC);
	}

	if(_r_cfg_read(APP_NAME_SHORT, L"ModuleAdtschema", 1))
	{
		_Errlib_FormatMessage(hwnd, code, L"adtschema.dll", IDS_MODULE_ADTSCHEMA);
	}
	
	if(_r_cfg_read(APP_NAME_SHORT, L"ModuleNetmsg", 1))
	{
		_Errlib_FormatMessage(hwnd, code, L"netmsg.dll", IDS_MODULE_NETMSG);
	}

	if(_r_cfg_read(APP_NAME_SHORT, L"ModuleNetevent", 1))
	{
		_Errlib_FormatMessage(hwnd, code, L"netevent.dll", IDS_MODULE_NETEVENT);
	}

	// print (custom)
	std::wstring modules = _r_cfg_read(APP_NAME_SHORT, L"Modules", CUSTOM_DEFAULT);
	std::wstring module;

	LPWSTR next = NULL, modules_dup = _wcsdup(modules.c_str()), token = wcstok_s(modules_dup, L";", &next);

	while(token != NULL)
	{
		module = token;

		module.erase(0, module.find_first_not_of(L' '));
		module.erase(module.find_last_not_of(L' ') + 1);

		_Errlib_FormatMessage(hwnd, code, module.c_str(), 0);

		token = wcstok_s(NULL, L";", &next);
	}

	free(token);
	free(modules_dup);
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

				SendDlgItemMessage(hwnd, IDC_LANGUAGE, CB_INSERTSTRING, 0, (LPARAM)L"System default");
				SendDlgItemMessage(hwnd, IDC_LANGUAGE, CB_SETCURSEL, 0, 0);

				EnumResourceLanguages(NULL, RT_STRING, MAKEINTRESOURCE(626), _r_locale_enum, (LONG_PTR)GetDlgItem(hwnd, IDC_LANGUAGE));

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
				SetDlgItemText(hwnd, IDC_MODULES, _r_cfg_read(APP_NAME_SHORT, L"Modules", CUSTOM_DEFAULT).c_str());
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
					INT length = (INT)SendDlgItemMessage(hwnd, IDC_MODULES, WM_GETTEXTLENGTH, 0, NULL) + sizeof(WCHAR);

					LPWSTR buffer = (LPWSTR)malloc(length * sizeof(WCHAR));

					if(buffer)
					{
						GetDlgItemText(hwnd, IDC_MODULES, buffer, length);

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
			_r_treeview_setstyle(hwnd, IDC_NAV, TVS_EX_DOUBLEBUFFER, 22);

			for(INT i = 0; i < APP_SETTINGS_COUNT; i++)
			{
				_r_treeview_additem(hwnd, IDC_NAV, (LPWSTR)_r_locale(IDS_SETTINGS_1 + i), -1, (LPARAM)CreateDialogParam(NULL, MAKEINTRESOURCE(IDD_SETTINGS_1 + i), hwnd, PagesDlgProc, IDD_SETTINGS_1 + i));
			}

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
				case IDC_OK:
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
			_r_listview_setstyle(hwnd, IDC_LISTVIEW, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP, TRUE);

			_r_listview_addcolumn(hwnd, IDC_LISTVIEW, _r_locale(IDS_COLUMN_1), _r_cfg_read(APP_NAME_SHORT, L"Column1", INT(190)), 0, LVCFMT_LEFT);
			_r_listview_addcolumn(hwnd, IDC_LISTVIEW, _r_locale(IDS_COLUMN_2), _r_cfg_read(APP_NAME_SHORT, L"Column2", INT(355)), 1, LVCFMT_LEFT);

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

		case WM_NOTIFY:
		{
			LPNMHDR hdr = (LPNMHDR)lparam;

			switch(hdr->code)
			{	
				case LVN_GETINFOTIP:
				{
					if(wparam == IDC_LISTVIEW)
					{
						LPNMLVGETINFOTIP pnmlv = (LPNMLVGETINFOTIP)lparam;

						StringCchCopy(pnmlv->pszText, pnmlv->cchTextMax, (LPCWSTR)_r_listview_getlparam(hwnd, IDC_LISTVIEW, pnmlv->iItem));
					}

					break;
				}

				case NM_DBLCLK:
				{
					if(wparam == IDC_LISTVIEW)
					{
						LPNMITEMACTIVATE pnmlv = (LPNMITEMACTIVATE)lparam;

						if(pnmlv->iItem >= 0)
						{
							_r_msg(MB_ICONINFORMATION, L"%s: %s", _r_listview_gettext(hwnd, IDC_LISTVIEW, pnmlv->iItem, 0, MAX_PATH).c_str(), _r_listview_getlparam(hwnd, IDC_LISTVIEW, pnmlv->iItem));
						}
					}

					break;
				}

				case LVN_DELETEITEM:
				{
					if(wparam == IDC_LISTVIEW)
					{
						LPNMLISTVIEW pnmlv = (LPNMLISTVIEW)lparam;

						free((LPVOID)pnmlv->lParam);
					}

					break;
				}
			}

			break;
		}

		case WM_COMMAND:
		{
			if(HIWORD(wparam) == EN_CHANGE && LOWORD(wparam) == IDC_CODE)
			{
				_Errlib_Print(hwnd);
				break;
			}

			switch(LOWORD(wparam))
			{
				case IDOK: // process Enter key
				{
					_Errlib_Print(hwnd);
					break;
				}

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
					std::wstring result;

					INT item = -1;

					while((item = (INT)SendDlgItemMessage(hwnd, IDC_LISTVIEW, LVM_GETNEXTITEM, item, LVNI_SELECTED)) != -1)
					{
						result.append((LPCWSTR)_r_listview_getlparam(hwnd, IDC_LISTVIEW, item));
						result.append(L"\r\n");
					}

					if(!result.empty())
					{
						result.erase(result.find_last_not_of(L"\r\n") + 2);

						_r_clipboard_set(result.c_str(), result.length());
					}
 
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