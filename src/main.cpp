// Error Lookup
// Copyright (c) 2011-2015 Henry++

#include <windows.h>
#include <stdint.h>
#include "dxerr.h"

#include "main.h"
#include "application.h"
#include "routine.h"

#include "resource.h"

CApplication app (APP_NAME, APP_NAME_SHORT, APP_VERSION, APP_AUTHOR);

LCID lcid;

#define SYSTEM_MODULES_DEFAULT L"kernel32.dll; ntdll.dll; ntoskrnl.exe; crypt32.dll; kerberos.dll; adtschema.dll; mpssvc.dll; msimsg.dll; wmerror.dll; mferror.dll; netevent.dll; netmsg.dll; ntshrui.dll; qmgr.dll; winhttp.dll; wininet.dll; wsock32.dll; rpcrt4.dll; dhcpsapi.dll; dhcpcore.dll; dhcpcore6.dll; p2p.dll; iphlpapi.dll; ipnathlp.dll; winbio.dll; pdh.dll; loadperf.dll; pshed.dll; ole32.dll; schedsvc.dll; twinui.dll; ddputils.dll; efscore.dll; msxml3r.dll; msxml6r.dll; comres.dll; blbres.dll;  dmutil.dll; imapi2.dll; imapi2fs.dll; mprmsg.dll; msobjs.dll; mswsock.dll; ntprint.dll;"

#ifdef _WIN64
#define FORMAT_DEC L"%lld"
#define FORMAT_HEX L"0x%08llx"
#else
#define FORMAT_DEC L"%u"
#define FORMAT_HEX L"0x%08x"
#endif // _WIN64

DWORD _Errlib_GetCode (HWND hwnd)
{
	WCHAR buffer[MAX_PATH] = {0};
	DWORD result = 0;

	if (GetDlgItemText (hwnd, IDC_CODE, buffer, _countof (buffer)))
	{
		if ((result = wcstoul (buffer, nullptr, 10)) == 0)
		{
			result = wcstoul (buffer, nullptr, 16);
		}
	}

	return result;
}

CString _Errlib_FormatMessage (DWORD code, LPCWSTR module, BOOL is_localized = TRUE)
{
	CString result;
	HMODULE h = LoadLibraryEx (module, nullptr, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);

	if (h)
	{
		HLOCAL buffer = nullptr;

		if (FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS, h, code, is_localized ? lcid : 0, (LPWSTR)&buffer, 0, nullptr))
		{
			result = (LPCWSTR)buffer;

			if (wcsncmp (result, L"%1", 2) == 0)
			{
				result.Empty (); // clear
			}
		}
		else
		{
			if (is_localized)
			{
				result = _Errlib_FormatMessage (code, module, FALSE);
			}
		}

		LocalFree (buffer);
		FreeLibrary (h);
	}

	return result.Trim (L"\r\n ");
}

VOID _Errlib_ShowDescription (HWND hwnd, INT item)
{
	CString buffer;

	if (item >= 0)
	{
		buffer = _r_listview_gettext (hwnd, IDC_LISTVIEW, item, 0);

		if (buffer.GetLength ())
		{
			buffer.Append (L": ");
			buffer.Append ((LPCWSTR)_r_listview_getlparam (hwnd, IDC_LISTVIEW, item));
		}
	}

	SetDlgItemText (hwnd, IDC_DESCRIPTION, buffer);
}

CString _Errlib_GetModuleDescription (CString module)
{
	// hardcoded descriptions
	if (module.CompareNoCase (L"kernel32.dll") == 0)
	{
		return L"Windows (User-Mode)";
	}
	else if (module.CompareNoCase (L"ntdll.dll") == 0)
	{
		return L"Windows (Kernel-Mode)";
	}
	else if (module.CompareNoCase (L"ntoskrnl.exe") == 0)
	{
		return L"Blue Screen of Death (BSOD)";
	}
	else if (module.CompareNoCase (L"crypt32.dll") == 0)
	{
		return L"Cryptographic API";
	}
	else if (module.CompareNoCase (L"kerberos.dll") == 0)
	{
		return L"Kerberos";
	}
	else if (module.CompareNoCase (L"adtschema.dll") == 0)
	{
		return L"Security Audit";
	}
	else if (module.CompareNoCase (L"mpssvc.dll") == 0)
	{
		return L"Microsoft Protection Service";
	}
	else if (module.CompareNoCase (L"msimsg.dll") == 0)
	{
		return L"Windows Installer";
	}
	else if (module.CompareNoCase (L"wmerror.dll") == 0)
	{
		return L"Windows Media";
	}
	else if (module.CompareNoCase (L"mferror.dll") == 0)
	{
		return L"Media Foundation";
	}
	else if (module.CompareNoCase (L"netevent.dll") == 0)
	{
		return "Network Events";
	}
	else if (module.CompareNoCase (L"netmsg.dll") == 0)
	{
		return L"Network Messages";
	}
	else if (module.CompareNoCase (L"ntshrui.dll") == 0)
	{
		return L"Network Share";
	}
	else if (module.CompareNoCase (L"qmgr.dll") == 0)
	{
		return L"Background Intelligent Transfer Service (BITS)";
	}
	else if (module.CompareNoCase (L"winhttp.dll") == 0)
	{
		return L"HTTP Services";
	}
	else if (module.CompareNoCase (L"wininet.dll") == 0)
	{
		return L"Windows Internet";
	}
	else if (module.CompareNoCase (L"wsock32.dll") == 0)
	{
		return L"Socket Library";
	}
	else if (module.CompareNoCase (L"rpcrt4.dll") == 0)
	{
		return L"Remote Procedure Call (RPC)";
	}
	else if (module.CompareNoCase (L"dhcpsapi.dll") == 0)
	{
		return L"DHCP Server API";
	}
	else if (module.CompareNoCase (L"dhcpcore.dll") == 0)
	{
		return L"DHCP v4";
	}
	else if (module.CompareNoCase (L"dhcpcore6.dll") == 0)
	{
		return L"DHCP v6";
	}
	else if (module.CompareNoCase (L"p2p.dll") == 0)
	{
		return L"Peer-to-peer (P2P)";
	}
	else if (module.CompareNoCase (L"iphlpapi.dll") == 0)
	{
		return L"IP Helper API";
	}
	else if (module.CompareNoCase (L"ipnathlp.dll") == 0)
	{
		return L"NAT Helper API";
	}
	else if (module.CompareNoCase (L"winbio.dll") == 0)
	{
		return L"Biometric API";
	}
	else if (module.CompareNoCase (L"pdh.dll") == 0)
	{
		return L"Performance Data Helper (PDH)";
	}
	else if (module.CompareNoCase (L"loadperf.dll") == 0)
	{
		return L"Performance Counter Library";
	}
	else if (module.CompareNoCase (L"pshed.dll") == 0)
	{
		return L"Platform Specific Hardware Error (PSHED)";
	}
	else if (module.CompareNoCase (L"comres.dll") == 0)
	{
		return L"COM+ Library";
	}
	else if (module.CompareNoCase (L"ole32.dll") == 0)
	{
		return L"Object Linking and Embedding (OLE)";
	}
	else if (module.CompareNoCase (L"schedsvc.dll") == 0)
	{
		return L"Task Scheduler Service";
	}
	else if (module.CompareNoCase (L"twinui.dll") == 0)
	{
		return L"Twin UI";
	}
	else if (module.CompareNoCase (L"ddputil.dll") == 0)
	{
		return L"Data Deduplication Library";
	}
	else if (module.CompareNoCase (L"efscore.dll") == 0)
	{
		return L"Encrypting File System (EFS)";
	}
	else if (module.CompareNoCase (L"msxml3r.dll") == 0)
	{
		return L"Microsoft XML v3";
	}
	else if (module.CompareNoCase (L"msxml6r.dll") == 0)
	{
		return L"Microsoft XML v6";
	}

	// version information
	DWORD size = GetFileVersionInfoSize (module, nullptr);

	if (size)
	{
		LPTSTR info = (LPTSTR)GlobalAlloc (GPTR, size);

		if (info)
		{
			WORD* language = nullptr;
			PVOID buffer = nullptr;

			UINT len = 0;

			if (GetFileVersionInfo (module, 0, size, info))
			{
				VerQueryValue (info, L"\\VarFileInfo\\Translation", (LPVOID*)&language, &len);

				if (VerQueryValue (info, _r_fmt (L"\\StringFileInfo\\%04x%04x\\FileDescription", language[0], language[1]), &buffer, &len))
				{
					return (LPCWSTR)buffer;
				}
			}
		}

		GlobalFree ((HGLOBAL)info);
	}

	return PathFindFileName (module);
}

VOID _Errlib_InsertItem (HWND hwnd, LPCWSTR description, CString text, BOOL is_internal)
{
	SIZE_T length = text.GetLength () + 1;
	LPWSTR lparam = (LPWSTR)malloc (length * sizeof (WCHAR));

	if (lparam != nullptr)
	{
		StringCchCopy (lparam, length, text);
	}

	_r_listview_additem (hwnd, IDC_LISTVIEW, description, -1, 0, -1, is_internal ? 1 : 0, (LPARAM)lparam);
	_r_listview_additem (hwnd, IDC_LISTVIEW, text.Mid (0, 70), -1, 1);
}

VOID _Errlib_PrintInformation (HWND hwnd)
{
	DWORD code = _Errlib_GetCode (hwnd);

	CString buffer;

	// clear first
	SendDlgItemMessage (hwnd, IDC_LISTVIEW, LVM_DELETEALLITEMS, 0, NULL);

	// dec.
	SetDlgItemText (hwnd, IDC_CODE10_CTL, _r_fmt (FORMAT_DEC, code));

	// hex.
	SetDlgItemText (hwnd, IDC_CODE16_CTL, _r_fmt (FORMAT_HEX, code));

	// sev.
	switch (HRESULT_SEVERITY (code))
	{
		SWITCH_ROUTINE (STATUS_SEVERITY_SUCCESS);
		SWITCH_ROUTINE (STATUS_SEVERITY_INFORMATIONAL);
		SWITCH_ROUTINE (STATUS_SEVERITY_WARNING);
		SWITCH_ROUTINE (STATUS_SEVERITY_ERROR);

		default:
		{
			buffer.Format (L"0x%02x", HRESULT_SEVERITY (code));
			break;
		}
	}

	SetDlgItemText (hwnd, IDC_SEVERITY_CTL, buffer);

	// fac.
	switch (HRESULT_FACILITY (code))
	{
		SWITCH_ROUTINE (FACILITY_NULL);
		SWITCH_ROUTINE (FACILITY_RPC);
		SWITCH_ROUTINE (FACILITY_DISPATCH);
		SWITCH_ROUTINE (FACILITY_STORAGE);
		SWITCH_ROUTINE (FACILITY_ITF);
		SWITCH_ROUTINE (FACILITY_WIN32);
		SWITCH_ROUTINE (FACILITY_WINDOWS);
		SWITCH_ROUTINE (FACILITY_SSPI); // FACILITY_SECURITY
		SWITCH_ROUTINE (FACILITY_CONTROL);
		SWITCH_ROUTINE (FACILITY_CERT);
		SWITCH_ROUTINE (FACILITY_INTERNET);
		SWITCH_ROUTINE (FACILITY_MEDIASERVER);
		SWITCH_ROUTINE (FACILITY_MSMQ);
		SWITCH_ROUTINE (FACILITY_SETUPAPI);
		SWITCH_ROUTINE (FACILITY_SCARD);
		SWITCH_ROUTINE (FACILITY_COMPLUS);
		SWITCH_ROUTINE (FACILITY_AAF);
		SWITCH_ROUTINE (FACILITY_URT);
		SWITCH_ROUTINE (FACILITY_ACS);
		SWITCH_ROUTINE (FACILITY_DPLAY);
		SWITCH_ROUTINE (FACILITY_UMI);
		SWITCH_ROUTINE (FACILITY_SXS);
		SWITCH_ROUTINE (FACILITY_WINDOWS_CE);
		SWITCH_ROUTINE (FACILITY_HTTP);
		SWITCH_ROUTINE (FACILITY_USERMODE_COMMONLOG);
		SWITCH_ROUTINE (FACILITY_USERMODE_FILTER_MANAGER);
		SWITCH_ROUTINE (FACILITY_BACKGROUNDCOPY);
		SWITCH_ROUTINE (FACILITY_CONFIGURATION);
		SWITCH_ROUTINE (FACILITY_STATE_MANAGEMENT);
		SWITCH_ROUTINE (FACILITY_METADIRECTORY);
		SWITCH_ROUTINE (FACILITY_WINDOWSUPDATE);
		SWITCH_ROUTINE (FACILITY_DIRECTORYSERVICE);
		SWITCH_ROUTINE (FACILITY_GRAPHICS);
		SWITCH_ROUTINE (FACILITY_SHELL);
		SWITCH_ROUTINE (FACILITY_TPM_SERVICES);
		SWITCH_ROUTINE (FACILITY_TPM_SOFTWARE);
		SWITCH_ROUTINE (FACILITY_PLA);
		SWITCH_ROUTINE (FACILITY_FVE);
		SWITCH_ROUTINE (FACILITY_FWP);
		SWITCH_ROUTINE (FACILITY_WINRM);
		SWITCH_ROUTINE (FACILITY_NDIS);
		SWITCH_ROUTINE (FACILITY_USERMODE_HYPERVISOR);
		SWITCH_ROUTINE (FACILITY_CMI);
		SWITCH_ROUTINE (FACILITY_USERMODE_VIRTUALIZATION);
		SWITCH_ROUTINE (FACILITY_USERMODE_VOLMGR);
		SWITCH_ROUTINE (FACILITY_BCD);
		SWITCH_ROUTINE (FACILITY_USERMODE_VHD);
		SWITCH_ROUTINE (FACILITY_SDIAG);
		SWITCH_ROUTINE (FACILITY_WEBSERVICES);
		SWITCH_ROUTINE (FACILITY_AUDIO_KERNEL);
		SWITCH_ROUTINE (FACILITY_WINDOWS_DEFENDER);
		SWITCH_ROUTINE (FACILITY_OPC);
		SWITCH_ROUTINE (FACILITY_BTH_ATT);
		SWITCH_ROUTINE (FACILITY_CODCLASS_ERROR_CODE);
		SWITCH_ROUTINE (FACILITY_FILTER_MANAGER);
		SWITCH_ROUTINE (FACILITY_GRAPHICS_KERNEL);
		SWITCH_ROUTINE (FACILITY_INTERIX);
		SWITCH_ROUTINE (FACILITY_MAXIMUM_VALUE);
		SWITCH_ROUTINE (FACILITY_MONITOR);
		SWITCH_ROUTINE (FACILITY_WIN32K_NTGDI);
		SWITCH_ROUTINE (FACILITY_WIN32K_NTUSER);
		SWITCH_ROUTINE (FACILITY_RDBSS);
		SWITCH_ROUTINE (FACILITY_RESUME_KEY_FILTER);
		SWITCH_ROUTINE (FACILITY_SECUREBOOT);
		SWITCH_ROUTINE (FACILITY_SHARED_VHDX);
		SWITCH_ROUTINE (FACILITY_SPACES);
		SWITCH_ROUTINE (FACILITY_VIDEO);
		SWITCH_ROUTINE (FACILITY_MBN);
		SWITCH_ROUTINE (FACILITY_RAS);
		SWITCH_ROUTINE (FACILITY_UI);
		SWITCH_ROUTINE (FACILITY_XPS);
		SWITCH_ROUTINE (FACILITY_D3D);
		SWITCH_ROUTINE (FACILITY_D3DX);
		SWITCH_ROUTINE (FACILITY_DSOUND_DMUSIC);
		SWITCH_ROUTINE (FACILITY_D3D10);
		SWITCH_ROUTINE (FACILITY_XAUDIO2);
		SWITCH_ROUTINE (FACILITY_XAPO);
		SWITCH_ROUTINE (FACILITY_XACTENGINE);
		SWITCH_ROUTINE (FACILITY_D3D11_OR_AE);
		SWITCH_ROUTINE (FACILITY_D2D);
		SWITCH_ROUTINE (FACILITY_DWRITE);
		SWITCH_ROUTINE (FACILITY_APO);
		SWITCH_ROUTINE (FACILITY_LEAP);
		SWITCH_ROUTINE (FACILITY_WSAPI);
		SWITCH_ROUTINE (FACILITY_DXGI);
		SWITCH_ROUTINE (FACILITY_XAML);
		SWITCH_ROUTINE (FACILITY_USN);
		SWITCH_ROUTINE (FACILITY_BLBUI);
		SWITCH_ROUTINE (FACILITY_SPP);
		SWITCH_ROUTINE (FACILITY_WSB_ONLINE);
		SWITCH_ROUTINE (FACILITY_BLB_CLI);
		SWITCH_ROUTINE (FACILITY_BLB);
		SWITCH_ROUTINE (FACILITY_WSBAPP);
		SWITCH_ROUTINE (FACILITY_WMAAECMA);
		SWITCH_ROUTINE (FACILITY_WEP);
		SWITCH_ROUTINE (FACILITY_WEB_SOCKET);
		SWITCH_ROUTINE (FACILITY_WEB);
		SWITCH_ROUTINE (FACILITY_USERMODE_VOLSNAP);
		SWITCH_ROUTINE (FACILITY_VISUALCPP);
		SWITCH_ROUTINE (FACILITY_TIERING);
		SWITCH_ROUTINE (FACILITY_SYNCENGINE);
		SWITCH_ROUTINE (FACILITY_SOS);
		SWITCH_ROUTINE (FACILITY_USERMODE_SDBUS);
		SWITCH_ROUTINE (FACILITY_SCRIPT);
		SWITCH_ROUTINE (FACILITY_PARSE);
		SWITCH_ROUTINE (FACILITY_PIDGENX);
		SWITCH_ROUTINE (FACILITY_P2P_INT);
		SWITCH_ROUTINE (FACILITY_P2P);
		SWITCH_ROUTINE (FACILITY_ONLINE_ID);
		SWITCH_ROUTINE (FACILITY_MOBILE);
		SWITCH_ROUTINE (FACILITY_LINGUISTIC_SERVICES);
		SWITCH_ROUTINE (FACILITY_JSCRIPT);
		SWITCH_ROUTINE (FACILITY_ACCELERATOR);
		SWITCH_ROUTINE (FACILITY_EAS);
		SWITCH_ROUTINE (FACILITY_DXGI_DDI);
		SWITCH_ROUTINE (FACILITY_DAF);
		SWITCH_ROUTINE (FACILITY_DEPLOYMENT_SERVICES_UTIL);
		SWITCH_ROUTINE (FACILITY_DEPLOYMENT_SERVICES_TRANSPORT_MANAGEMENT);
		SWITCH_ROUTINE (FACILITY_DEPLOYMENT_SERVICES_TFTP);
		SWITCH_ROUTINE (FACILITY_DEPLOYMENT_SERVICES_PXE);
		SWITCH_ROUTINE (FACILITY_DEPLOYMENT_SERVICES_MULTICAST_SERVER);
		SWITCH_ROUTINE (FACILITY_DEPLOYMENT_SERVICES_MULTICAST_CLIENT);
		SWITCH_ROUTINE (FACILITY_DEPLOYMENT_SERVICES_MANAGEMENT);
		SWITCH_ROUTINE (FACILITY_DEPLOYMENT_SERVICES_IMAGING);
		SWITCH_ROUTINE (FACILITY_DEPLOYMENT_SERVICES_DRIVER_PROVISIONING);
		SWITCH_ROUTINE (FACILITY_DEPLOYMENT_SERVICES_SERVER);
		SWITCH_ROUTINE (FACILITY_DEPLOYMENT_SERVICES_CONTENT_PROVIDER);
		SWITCH_ROUTINE (FACILITY_DEPLOYMENT_SERVICES_BINLSVC);
		SWITCH_ROUTINE (FACILITY_DEFRAG);
		SWITCH_ROUTINE (FACILITY_DEBUGGERS);
		SWITCH_ROUTINE (FACILITY_BLUETOOTH_ATT);
		SWITCH_ROUTINE (FACILITY_AUDIOSTREAMING);
		SWITCH_ROUTINE (FACILITY_AUDIO);
		SWITCH_ROUTINE (FACILITY_ACTION_QUEUE);
		SWITCH_ROUTINE (FACILITY_EAP_MESSAGE);

		default:
		{
			buffer.Format (L"0x%02x", HRESULT_FACILITY (code));
			break;
		}
	}

	SetDlgItemText (hwnd, IDC_FACILITY_CTL, buffer);

	// print (system)
	INT pos = 0;
	CString modules = app.ConfigGet (L"SystemModule", SYSTEM_MODULES_DEFAULT), token = modules.Tokenize (L";", pos);

	while (!token.IsEmpty ())
	{
		token = token.Trim (L"\r\n ");

		buffer = _Errlib_FormatMessage (code, token);

		if (buffer.GetLength ())
		{
			_Errlib_InsertItem (hwnd, _Errlib_GetModuleDescription (token), buffer, FALSE);
		}

		token = modules.Tokenize (L";", pos);
	}

	// print (internal)
	if (app.ConfigGet (L"InternalModuleCPP", 1))
	{
		_wcserror_s (buffer.GetBuffer (4096), 4096, code);
		buffer.ReleaseBuffer ();

		if (wcsncmp (buffer, L"Unknown error", 13) != 0)
		{
			_Errlib_InsertItem (hwnd, I18N (&app, IDS_MODULE_INTERNAL_CPP, 0), buffer, TRUE);
		}
	}

	if (app.ConfigGet (L"InternalModuleDX", 1))
	{
		buffer = DXGetErrorDescription (HRESULT (code));

		if (buffer.CompareNoCase (L"n/a") != 0)
		{
			_Errlib_InsertItem (hwnd, I18N (&app, IDS_MODULE_INTERNAL_DX, 0), buffer, TRUE);
		}
	}

	// show description for first item
	_Errlib_ShowDescription (hwnd, 0);
}

INT_PTR WINAPI PagesDlgProc (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	INT iHeight = 0;
	POINT p;
	static HIMAGELIST hDragImageList;
	HIMAGELIST hOneImageList, hTempImageList;
	static BOOL bDragging;
	LVHITTESTINFO lvhti;
	BOOL bFirst;
	IMAGEINFO imf;

	switch (msg)
	{
		case WM_INITDIALOG:
		{
			// localize
			SetDlgItemText (hwnd, IDC_TITLE_1, I18N (&app, IDS_TITLE_1, 0));
			SetDlgItemText (hwnd, IDC_TITLE_2, I18N (&app, IDS_TITLE_2, 0));
			SetDlgItemText (hwnd, IDC_TITLE_3, I18N (&app, IDS_TITLE_3, 0));

			SetDlgItemText (hwnd, IDC_ALWAYSONTOP_CHK, I18N (&app, IDS_ALWAYSONTOP_CHK, 0));
			SetDlgItemText (hwnd, IDC_INSERTBUFFER_CHK, I18N (&app, IDS_INSERTBUFFER_CHK, 0));
			SetDlgItemText (hwnd, IDC_CHECKUPDATES_CHK, I18N (&app, IDS_CHECKUPDATES_CHK, 0));

			SetDlgItemText (hwnd, IDC_MODULE_INTERNAL_CPP, I18N (&app, IDS_MODULE_INTERNAL_CPP, 0));
			SetDlgItemText (hwnd, IDC_MODULE_INTERNAL_DX, I18N (&app, IDS_MODULE_INTERNAL_DX, 0));

			// set configuration
			if (lparam == 0)
			{
				CheckDlgButton (hwnd, IDC_ALWAYSONTOP_CHK, app.ConfigGet (L"AlwaysOnTop", 0) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton (hwnd, IDC_INSERTBUFFER_CHK, app.ConfigGet (L"InsertBufferAtStartup", 1) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton (hwnd, IDC_CHECKUPDATES_CHK, app.ConfigGet (L"CheckUpdates", 1) ? BST_CHECKED : BST_UNCHECKED);

				SendDlgItemMessage (hwnd, IDC_LANGUAGE, CB_INSERTSTRING, 0, (LPARAM)L"English (default)");
				SendDlgItemMessage (hwnd, IDC_LANGUAGE, CB_SETCURSEL, 0, NULL);

				app.LocaleEnum (hwnd, IDC_LANGUAGE);

				SetProp (app.GetHWND (), L"language", (HANDLE)SendDlgItemMessage (hwnd, IDC_LANGUAGE, CB_GETCURSEL, 0, NULL)); // check on save
			}
			else if (lparam == 1)
			{
				_r_listview_setstyle (hwnd, IDC_MODULES, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP | LVS_EX_CHECKBOXES);

				_r_listview_addcolumn (hwnd, IDC_MODULES, I18N (&app, IDS_COLUMN_1, 0), 30, 0, LVCFMT_LEFT);
				_r_listview_addcolumn (hwnd, IDC_MODULES, I18N (&app, IDS_COLUMN_2, 0), 65, 1, LVCFMT_LEFT);

				CString str = app.ConfigGet (L"SystemModule", SYSTEM_MODULES_DEFAULT);

				INT pos = 0;
				INT item = 0;
				CString token = str.Tokenize (L";", pos);

				while (!token.IsEmpty ())
				{
					CString module = token.Trim ();
					BOOL is_enabled = module.GetAt (0) == L'~' ? FALSE : TRUE;

					if (!is_enabled)
					{
						module = module.Mid (1, module.GetLength ());
					}

					if (!module.IsEmpty ())
					{
						_r_listview_additem (hwnd, IDC_MODULES, module, item, 0);
						_r_listview_additem (hwnd, IDC_MODULES, _Errlib_GetModuleDescription (module), item, 1);

						ListView_SetCheckState (GetDlgItem (hwnd, IDC_MODULES), item++, is_enabled);
					}

					token = str.Tokenize (L";", pos);
				}
			}
			else if (lparam == 2)
			{
				CheckDlgButton (hwnd, IDC_MODULE_INTERNAL_CPP, app.ConfigGet (L"InternalModuleCPP", 1) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton (hwnd, IDC_MODULE_INTERNAL_DX, app.ConfigGet (L"InternalModuleDX", 1) ? BST_CHECKED : BST_UNCHECKED);
			}

			break;
		}

		case WM_MOUSEMOVE:
		{
			if (bDragging && GetDlgItem (hwnd, IDC_MODULES))
			{
				lvhti.pt.x = LOWORD (lparam);
				lvhti.pt.y = HIWORD (lparam);

				ListView_SetItemState (GetDlgItem (hwnd, IDC_MODULES), -1, 0, LVIS_DROPHILITED);

				if (SendDlgItemMessage (hwnd, IDC_MODULES, LVM_HITTEST, 0, (LPARAM)&lvhti) >= 0)
				{
					ListView_SetItemState (GetDlgItem (hwnd, IDC_MODULES), lvhti.iItem, LVIS_DROPHILITED, LVIS_DROPHILITED);
				}

				ClientToScreen (hwnd, &lvhti.pt);
				ImageList_DragMove (lvhti.pt.x, lvhti.pt.y);
			}

			break;
		}

		case WM_LBUTTONUP:
		{
			if (GetDlgItem (hwnd, IDC_MODULES))
			{
				// End the drag-and-drop process
				bDragging = FALSE;

				ImageList_DragLeave (GetDlgItem (hwnd, IDC_MODULES));
				ImageList_EndDrag ();
				ImageList_Destroy (hDragImageList);

				ReleaseCapture ();

				// Determine the dropped item
				lvhti.pt.x = LOWORD (lparam);
				lvhti.pt.y = HIWORD (lparam);

				ClientToScreen (hwnd, &lvhti.pt);
				ScreenToClient (GetDlgItem (hwnd, IDC_MODULES), &lvhti.pt);

				SendDlgItemMessage (hwnd, IDC_MODULES, LVM_HITTEST, 0, (LPARAM)&lvhti);

				ListView_SetItemState (GetDlgItem (hwnd, IDC_MODULES), -1, 0, LVIS_DROPHILITED);

				// Out of the ListView?
				if (lvhti.iItem >= 0)
				{
					LVITEM lvi = {0};

					WCHAR buf[MAX_PATH];

					// Rearrange the items
					INT item = -1;

					while ((item = (INT)SendDlgItemMessage (hwnd, IDC_MODULES, LVM_GETNEXTITEM, item, LVNI_SELECTED)) != -1)
					{
						// First, copy one item
						lvi.iItem = item;
						lvi.iSubItem = 0;
						lvi.cchTextMax = MAX_PATH;
						lvi.pszText = buf;
						lvi.mask = LVIF_TEXT | LVIF_STATE;
						lvi.stateMask = LVIS_STATEIMAGEMASK;

						ListView_GetItem (GetDlgItem (hwnd, IDC_MODULES), &lvi);

						//lvi.state = INDEXTOSTATEIMAGEMASK(2);
						lvi.iItem = lvhti.iItem;

						// Insert the main item
						INT iRet = ListView_InsertItem (GetDlgItem (hwnd, IDC_MODULES), &lvi);
						//ListView_SetCheckState(GetDlgItem(hwnd, IDC_MODULES), iRet, TRUE);

						//if(lvi.iItem < item)
						//	lvhti.iItem++;

						if (iRet <= item)
							item++;

						// Set the subitem text
						for (INT i = 1; i < 2; i++)
						{
							ListView_GetItemText (GetDlgItem (hwnd, IDC_MODULES), item, i, buf, MAX_PATH);
							ListView_SetItemText (GetDlgItem (hwnd, IDC_MODULES), iRet, i, buf);
						}

						// Delete from original position
						ListView_DeleteItem (GetDlgItem (hwnd, IDC_MODULES), item);
					}
				}
			}

			break;
		}

		case WM_CONTEXTMENU:
		{
			if (GetDlgCtrlID ((HWND)wparam) == IDC_MODULES)
			{
				HMENU menu = LoadMenu (nullptr, MAKEINTRESOURCE (IDM_MODULES)), submenu = GetSubMenu (menu, 0);

				// localize
				app.LocaleMenu (submenu, I18N (&app, IDS_ADD, 0), IDM_ADD, FALSE);
				app.LocaleMenu (submenu, I18N (&app, IDS_DELETE, 0), IDM_DELETE, FALSE);

				if (!SendDlgItemMessage (hwnd, IDC_MODULES, LVM_GETSELECTEDCOUNT, 0, NULL))
				{
					EnableMenuItem (submenu, IDM_DELETE, MF_BYCOMMAND | MF_DISABLED);
				}

				TrackPopupMenuEx (submenu, TPM_RIGHTBUTTON | TPM_LEFTBUTTON, LOWORD (lparam), HIWORD (lparam), hwnd, nullptr);

				DestroyMenu (menu);
				DestroyMenu (submenu);
			}

			break;
		}

		case WM_NOTIFY:
		{
			if (((LPNMHDR)lparam)->idFrom == IDC_MODULES)
			{
				switch (((LPNMHDR)lparam)->code)
				{
					case LVN_BEGINLABELEDIT:
					{
						return FALSE;
					}

					case LVN_ENDLABELEDIT:
					{
						return TRUE;
					}

					case LVN_BEGINDRAG:
					{
						// You can set your customized cursor here
						p.x = p.y = -10;

						// Ok, now we create a drag-image for all selected items
						bFirst = TRUE;

						INT iPos = ListView_GetNextItem (((LPNMHDR)lparam)->hwndFrom, -1, LVNI_SELECTED);

						while (iPos != -1)
						{
							if (bFirst)
							{
								// For the first selected item,
								// we simply create a single-line drag image
								hDragImageList = ListView_CreateDragImage (((LPNMHDR)lparam)->hwndFrom, iPos, &p);
								ImageList_GetImageInfo (hDragImageList, 0, &imf);
								iHeight = imf.rcImage.bottom;
								bFirst = FALSE;
							}
							else
							{
								// For the rest selected items,
								// we create a single-line drag image, then
								// append it to the bottom of the complete drag image
								hOneImageList = ListView_CreateDragImage (((LPNMHDR)lparam)->hwndFrom, iPos, &p);
								hTempImageList = ImageList_Merge (hDragImageList, 0, hOneImageList, 0, 0, iHeight);
								ImageList_Destroy (hDragImageList);
								ImageList_Destroy (hOneImageList);
								hDragImageList = hTempImageList;
								ImageList_GetImageInfo (hDragImageList, 0, &imf);
								iHeight = imf.rcImage.bottom;
							}

							iPos = ListView_GetNextItem (((LPNMHDR)lparam)->hwndFrom, iPos, LVNI_SELECTED);
						}

						// Now we can initialize then start the drag action
						ImageList_BeginDrag (hDragImageList, 0, 0, 0);

						POINT pt = ((NM_LISTVIEW*)((LPNMHDR)lparam))->ptAction;
						ClientToScreen (((LPNMHDR)lparam)->hwndFrom, &pt);

						ImageList_DragEnter (GetDesktopWindow (), pt.x, pt.y);

						bDragging = TRUE;

						// Don't forget to capture the mouse
						SetCapture (hwnd);

						break;
					}

					case NM_CUSTOMDRAW:
					{
						LONG result = CDRF_DODEFAULT;
						LPNMLVCUSTOMDRAW lpnmlv = (LPNMLVCUSTOMDRAW)lparam;

						switch (lpnmlv->nmcd.dwDrawStage)
						{
							case CDDS_PREPAINT:
							{
								result = (CDRF_NOTIFYPOSTPAINT | CDRF_NOTIFYITEMDRAW);
								break;
							}

							case CDDS_ITEMPREPAINT:
							{
								if ((UINT)lpnmlv->nmcd.lItemlParam == 0 && ListView_GetCheckState (lpnmlv->nmcd.hdr.hwndFrom, lpnmlv->nmcd.dwItemSpec))
								{
									HMODULE h = LoadLibraryEx (_r_listview_gettext (hwnd, lpnmlv->nmcd.hdr.idFrom, lpnmlv->nmcd.dwItemSpec, 0), nullptr, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);

									if (!h)
									{
										lpnmlv->clrText = RGB (255, 0, 0);
										result = (CDRF_NOTIFYPOSTPAINT | CDRF_NEWFONT);
									}

									FreeLibrary (h);
								}

								break;
							}
						}

						SetWindowLongPtr (hwnd, DWLP_MSGRESULT, result);
						return TRUE;
					}
				}
			}
		}

		case WM_COMMAND:
		{
			switch (LOWORD (wparam))
			{
				case IDM_ADD:
				{
					WCHAR file[MAX_PATH] = {0};

					OPENFILENAME ofn = {0};

					ofn.lStructSize = sizeof (ofn);
					ofn.hwndOwner = hwnd;
					ofn.lpstrFile = file;
					ofn.lpstrFile[0] = '\0';
					ofn.nMaxFile = _countof (file);
					ofn.lpstrFilter = L"*.dll; *.exe\0*.dll;*.exe\0All files (*.*)\0*.*\0";
					ofn.nFilterIndex = 1;
					ofn.lpstrFileTitle = nullptr;
					ofn.nMaxFileTitle = 0;
					ofn.lpstrInitialDir = nullptr;
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

					if (GetOpenFileName (&ofn))
					{
						INT item = (INT)SendDlgItemMessage (hwnd, IDC_MODULES, LVM_GETNEXTITEM, -1, LVNI_SELECTED) + 1;

						_r_listview_additem (hwnd, IDC_MODULES, file, item, 0);
						_r_listview_additem (hwnd, IDC_MODULES, _Errlib_GetModuleDescription (file), item, 1);

						ListView_SetCheckState (GetDlgItem (hwnd, IDC_MODULES), item, TRUE);
					}

					break;
				}

				case IDM_DELETE:
				{
					INT count = (INT)SendDlgItemMessage (hwnd, IDC_MODULES, LVM_GETITEMCOUNT, 0, NULL) - 1;

					for (INT i = count; i >= 0; i--)
					{
						if (ListView_GetItemState (GetDlgItem (hwnd, IDC_MODULES), i, LVNI_SELECTED))
						{
							SendDlgItemMessage (hwnd, IDC_MODULES, LVM_DELETEITEM, i, NULL);
						}
					}

					break;
				}
			}

			break;
		}
	}

	return FALSE;
}

BOOL SettingsSaveCallback (HWND hwnd, DWORD page_id)
{
	CString buffer;
	BOOL result = FALSE;

	switch (page_id)
	{
		case 0:
		{
			app.ConfigSet (L"AlwaysOnTop", INT ((IsDlgButtonChecked (hwnd, IDC_ALWAYSONTOP_CHK) == BST_CHECKED) ? TRUE : FALSE));
			app.ConfigSet (L"InsertBufferAtStartup", INT ((IsDlgButtonChecked (hwnd, IDC_INSERTBUFFER_CHK) == BST_CHECKED) ? TRUE : FALSE));
			app.ConfigSet (L"CheckUpdates", INT ((IsDlgButtonChecked (hwnd, IDC_CHECKUPDATES_CHK) == BST_CHECKED) ? TRUE : FALSE));

			// set language
			if (SendDlgItemMessage (hwnd, IDC_LANGUAGE, CB_GETCURSEL, 0, NULL) >= 1)
			{
				GetDlgItemText (hwnd, IDC_LANGUAGE, buffer.GetBuffer (MAX_PATH), MAX_PATH);
				buffer.ReleaseBuffer ();
			}

			app.LocaleSet (buffer);

			if (((INT)GetProp (app.GetHWND (), L"language") != (INT)SendDlgItemMessage (hwnd, IDC_LANGUAGE, CB_GETCURSEL, 0, NULL)))
			{
				result = TRUE;
			}

			break;
		}

		case 1:
		{
			INT item = -1;

			while ((item = (INT)SendDlgItemMessage (hwnd, IDC_MODULES, LVM_GETNEXTITEM, item, LVNI_ALL)) != -1)
			{
				if (!ListView_GetCheckState (GetDlgItem (hwnd, IDC_MODULES), item))
				{
					buffer.Append (L"~");
				}

				buffer.Append (_r_listview_gettext (hwnd, IDC_MODULES, item, 0));
				buffer.Append (L";");
			}

			app.ConfigSet (L"SystemModule", buffer);

			break;
		}

		case 2:
		{
			app.ConfigSet (L"InternalModuleCPP", INT ((IsDlgButtonChecked (hwnd, IDC_MODULE_INTERNAL_CPP) == BST_CHECKED) ? TRUE : FALSE));
			app.ConfigSet (L"InternalModuleDX", INT ((IsDlgButtonChecked (hwnd, IDC_MODULE_INTERNAL_DX) == BST_CHECKED) ? TRUE : FALSE));

			// on finish fix
			_Errlib_PrintInformation (app.GetHWND ());

			break;
		}

		case (DWORD)-1:
		{
			_r_windowtotop (app.GetHWND (), app.ConfigGet (L"AlwaysOnTop", 0));
			break;
		}
	}

	return result;
}

LRESULT CALLBACK DlgProc (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			// configure listview
			_r_listview_setstyle (hwnd, IDC_LISTVIEW, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);

			_r_listview_addcolumn (hwnd, IDC_LISTVIEW, I18N (&app, IDS_COLUMN_1, 0), app.ConfigGet (L"Column1", 40), 0, LVCFMT_LEFT);
			_r_listview_addcolumn (hwnd, IDC_LISTVIEW, I18N (&app, IDS_COLUMN_2, 0), app.ConfigGet (L"Column2", 60), 1, LVCFMT_LEFT);

			_r_listview_addgroup (hwnd, IDC_LISTVIEW, 0, I18N (&app, IDS_GROUP_1, 0), 0, 0);
			_r_listview_addgroup (hwnd, IDC_LISTVIEW, 1, I18N (&app, IDS_GROUP_2, 0), 0, 0);

			// configure controls
			SendDlgItemMessage (hwnd, IDC_CODE_UD, UDM_SETRANGE32, 0, INT32_MAX);

			if (app.ConfigGet (L"InsertBufferAtStartup", 1))
			{
				SetDlgItemText (hwnd, IDC_CODE, _r_clipboard_get (hwnd));
			}
			else
			{
				SetDlgItemInt (hwnd, IDC_CODE, 0, TRUE);
			}

			// get locale id
			lcid = wcstol (I18N (&app, IDS_LCID, 0), nullptr, 16);

			// localize
			HMENU menu = GetMenu (hwnd);

			app.LocaleMenu (menu, I18N (&app, IDS_FILE, 0), 0, TRUE);
			app.LocaleMenu (menu, I18N (&app, IDS_SETTINGS, 0), IDM_SETTINGS, FALSE);
			app.LocaleMenu (menu, I18N (&app, IDS_EXIT, 0), IDM_EXIT, FALSE);
			app.LocaleMenu (menu, I18N (&app, IDS_HELP, 0), 1, TRUE);
			app.LocaleMenu (menu, I18N (&app, IDS_WEBSITE, 0), IDM_WEBSITE, FALSE);
			app.LocaleMenu (menu, I18N (&app, IDS_CHECKUPDATES, 0), IDM_CHECKUPDATES, FALSE);
			app.LocaleMenu (menu, I18N (&app, IDS_ABOUT, 0), IDM_ABOUT, FALSE);

			DrawMenuBar (hwnd); // redraw menu

			SetDlgItemText (hwnd, IDC_CODE10, I18N (&app, IDS_CODE10, 0));
			SetDlgItemText (hwnd, IDC_CODE16, I18N (&app, IDS_CODE16, 0));
			SetDlgItemText (hwnd, IDC_SEVERITY, I18N (&app, IDS_SEVERITY, 0));
			SetDlgItemText (hwnd, IDC_FACILITY, I18N (&app, IDS_FACILITY, 0));

			break;
		}

		case WM_DESTROY:
		{
			app.ConfigSet (L"Column1", (DWORD)_r_listview_getcolumnwidth (hwnd, IDC_LISTVIEW, 0));
			app.ConfigSet (L"Column2", (DWORD)_r_listview_getcolumnwidth (hwnd, IDC_LISTVIEW, 1));

			PostQuitMessage (0);

			break;
		}

		case WM_QUERYENDSESSION:
		{
			SetWindowLongPtr (hwnd, DWLP_MSGRESULT, TRUE);
			return TRUE;
		}

		case WM_CONTEXTMENU:
		{
			if (GetDlgCtrlID ((HWND)wparam) == IDC_LISTVIEW)
			{
				HMENU menu = LoadMenu (nullptr, MAKEINTRESOURCE (IDM_LISTVIEW)), submenu = GetSubMenu (menu, 0);

				// localize
				app.LocaleMenu (submenu, I18N (&app, IDS_COPY, 0), IDM_COPY, FALSE);

				if (!SendDlgItemMessage (hwnd, IDC_LISTVIEW, LVM_GETSELECTEDCOUNT, 0, NULL))
				{
					EnableMenuItem (submenu, IDM_COPY, MF_BYCOMMAND | MF_DISABLED);
				}

				TrackPopupMenuEx (submenu, TPM_RIGHTBUTTON | TPM_LEFTBUTTON, LOWORD (lparam), HIWORD (lparam), hwnd, nullptr);

				DestroyMenu (menu);
				DestroyMenu (submenu);
			}

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
					if (wparam == IDC_LISTVIEW)
					{
						_Errlib_ShowDescription (hwnd, LPNMITEMACTIVATE (lparam)->iItem);
					}

					break;
				}

				case LVN_DELETEITEM:
				{
					if (wparam == IDC_LISTVIEW)
					{
						LPNMLISTVIEW pnmlv = (LPNMLISTVIEW)lparam;

						free ((LPVOID)pnmlv->lParam);
					}

					break;
				}

				case UDN_DELTAPOS:
				{
					if (wparam == IDC_CODE_UD)
					{
						SetDlgItemText (hwnd, IDC_CODE, _r_fmt (FORMAT_DEC, _Errlib_GetCode (hwnd) + LPNMUPDOWN (lparam)->iDelta));
						return TRUE;
					}

					break;
				}
			}

			break;
		}

		case WM_COMMAND:
		{
			if (HIWORD (wparam) == EN_CHANGE && LOWORD (wparam) == IDC_CODE)
			{
				_Errlib_PrintInformation (hwnd);
				break;
			}

			switch (LOWORD (wparam))
			{
				case IDM_SETTINGS:
				{
					app.CreateSettingsWindow (APP_SETTINGS_COUNT, PagesDlgProc, SettingsSaveCallback);
					break;
				}

				case IDCANCEL: // process Esc key
				case IDM_EXIT:
				{
					DestroyWindow (hwnd);
					break;
				}

				case IDM_WEBSITE:
				{
					ShellExecute (hwnd, nullptr, APP_WEBSITE, nullptr, nullptr, SW_SHOWDEFAULT);
					break;
				}

				case IDM_CHECKUPDATES:
				{
					app.CheckForUpdates (FALSE);
					break;
				}

				case IDM_ABOUT:
				{
					app.CreateAboutWindow ();
					break;
				}

				case IDM_COPY:
				{
					CString buffer;

					INT item = -1;

					while ((item = (INT)SendDlgItemMessage (hwnd, IDC_LISTVIEW, LVM_GETNEXTITEM, item, LVNI_SELECTED)) != -1)
					{
						buffer.Append ((LPCWSTR)_r_listview_getlparam (hwnd, IDC_LISTVIEW, item));
						buffer.Append (L"\r\n");
					}

					if (!buffer.IsEmpty ())
					{
						buffer.TrimRight (L"\r\n");

						_r_clipboard_set (hwnd, buffer, buffer.GetLength ());
					}

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
	app.SetCopyright (APP_COPYRIGHT);
	app.SetLinks (APP_WEBSITE, APP_GITHUB);

	if (app.CreateMainWindow ((DLGPROC)DlgProc))
	{
		MSG msg = {0};

#ifndef _WIN64
		if (_r_system_iswow64 ())
		{
			_r_msg (nullptr, MB_OK | MB_ICONEXCLAMATION, APP_NAME, L"WARNING! 32-bit version may incompatible with 64-bit operating system version!");
		}
#endif // _WIN64

		while (GetMessage (&msg, nullptr, 0, 0))
		{
			if (!IsDialogMessage (app.GetHWND (), &msg))
			{
				TranslateMessage (&msg);
				DispatchMessage (&msg);
			}
		}
	}

	return ERROR_SUCCESS;
}
