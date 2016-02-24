// Error Lookup
// Copyright (c) 2011-2016 Henry++

#include <windows.h>

#include "main.h"
#include "rapp.h"
#include "routine.h"
#include "dxerr.h"

#include "resource.h"

rapp app (APP_NAME, APP_NAME_SHORT, APP_VERSION, APP_COPYRIGHT);

LCID lcid = 0;

#define SYSTEM_MODULES_DEFAULT L"kernel32.dll; ntdll.dll; ntoskrnl.exe; crypt32.dll; kerberos.dll; adtschema.dll; mpssvc.dll; msimsg.dll; wmerror.dll; mferror.dll; netevent.dll; netmsg.dll; ntshrui.dll; qmgr.dll; winhttp.dll; wininet.dll; wsock32.dll; rpcrt4.dll; dhcpsapi.dll; dhcpcore.dll; dhcpcore6.dll; p2p.dll; iphlpapi.dll; ipnathlp.dll; winbio.dll; pdh.dll; loadperf.dll; pshed.dll; ole32.dll; schedsvc.dll; twinui.dll; ddputils.dll; efscore.dll; msxml3r.dll; msxml6r.dll; comres.dll; blbres.dll; dmutil.dll; imapi2.dll; imapi2fs.dll; mprmsg.dll; msobjs.dll; mswsock.dll; ntprint.dll;"

#define FORMAT_DEC L"%lu"
#define FORMAT_HEX L"0x%08lx"

DWORD _app_getcode (HWND hwnd)
{
	rstring buffer = _r_ctrl_gettext (hwnd, IDC_CODE);
	DWORD result = 0;

	if (!buffer.IsEmpty ())
	{
		if ((result = buffer.AsUlong (10)) == 0)
		{
			result = buffer.AsUlong (16);
		}
	}

	return result;
}

rstring _app_formatmessage (DWORD code, LPCWSTR module, BOOL is_localized = TRUE)
{
	rstring result;
	HMODULE h = LoadLibraryEx (module, nullptr, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);

	if (h)
	{
		HLOCAL buffer = nullptr;

		if (FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS, h, code, is_localized ? lcid : 0, (LPWSTR)&buffer, 0, nullptr))
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
			{
				result = _app_formatmessage (code, module, FALSE);
			}
		}

		LocalFree (buffer);
		FreeLibrary (h);
	}

	return result.Trim (L"\r\n ");
}

VOID _app_showdescription (HWND hwnd, INT item)
{
	if (_r_listview_getitemcount (hwnd, IDC_LISTVIEW) && item >= 0)
	{
		_r_ctrl_settext (hwnd, IDC_DESCRIPTION, L"%s: %s", _r_listview_gettext (hwnd, IDC_LISTVIEW, item, 0), _r_listview_getlparam (hwnd, IDC_LISTVIEW, item));
	}
	else
	{
		SetDlgItemText (hwnd, IDC_DESCRIPTION, nullptr);
	}
}

rstring _app_getdescription (rstring module)
{
	rstring result;

	// hardcoded descriptions
	if (module.CompareNoCase (L"kernel32.dll") == 0)
	{
		result = L"Windows (User-Mode)";
	}
	else if (module.CompareNoCase (L"ntdll.dll") == 0)
	{
		result = L"Windows (Kernel-Mode)";
	}
	else if (module.CompareNoCase (L"ntoskrnl.exe") == 0)
	{
		result = L"Blue Screen of Death (BSOD)";
	}
	else if (module.CompareNoCase (L"crypt32.dll") == 0)
	{
		result = L"Cryptographic API";
	}
	else if (module.CompareNoCase (L"kerberos.dll") == 0)
	{
		result = L"Kerberos";
	}
	else if (module.CompareNoCase (L"adtschema.dll") == 0)
	{
		result = L"Security Audit";
	}
	else if (module.CompareNoCase (L"mpssvc.dll") == 0)
	{
		result = L"Microsoft Protection Service";
	}
	else if (module.CompareNoCase (L"msimsg.dll") == 0)
	{
		result = L"Windows Installer";
	}
	else if (module.CompareNoCase (L"wmerror.dll") == 0)
	{
		result = L"Windows Media";
	}
	else if (module.CompareNoCase (L"mferror.dll") == 0)
	{
		result = L"Media Foundation";
	}
	else if (module.CompareNoCase (L"netevent.dll") == 0)
	{
		result = "Network Events";
	}
	else if (module.CompareNoCase (L"netmsg.dll") == 0)
	{
		result = L"Network Messages";
	}
	else if (module.CompareNoCase (L"ntshrui.dll") == 0)
	{
		result = L"Network Share";
	}
	else if (module.CompareNoCase (L"qmgr.dll") == 0)
	{
		result = L"Background Intelligent Transfer Service (BITS)";
	}
	else if (module.CompareNoCase (L"winhttp.dll") == 0)
	{
		result = L"HTTP Services";
	}
	else if (module.CompareNoCase (L"wininet.dll") == 0)
	{
		result = L"Windows Internet";
	}
	else if (module.CompareNoCase (L"wsock32.dll") == 0)
	{
		result = L"Socket Library";
	}
	else if (module.CompareNoCase (L"rpcrt4.dll") == 0)
	{
		result = L"Remote Procedure Call (RPC)";
	}
	else if (module.CompareNoCase (L"dhcpsapi.dll") == 0)
	{
		result = L"DHCP Server API";
	}
	else if (module.CompareNoCase (L"dhcpcore.dll") == 0)
	{
		result = L"DHCP v4";
	}
	else if (module.CompareNoCase (L"dhcpcore6.dll") == 0)
	{
		result = L"DHCP v6";
	}
	else if (module.CompareNoCase (L"p2p.dll") == 0)
	{
		result = L"Peer-to-peer (P2P)";
	}
	else if (module.CompareNoCase (L"iphlpapi.dll") == 0)
	{
		result = L"IP Helper API";
	}
	else if (module.CompareNoCase (L"ipnathlp.dll") == 0)
	{
		result = L"NAT Helper API";
	}
	else if (module.CompareNoCase (L"winbio.dll") == 0)
	{
		result = L"Biometric API";
	}
	else if (module.CompareNoCase (L"pdh.dll") == 0)
	{
		result = L"Performance Data Helper (PDH)";
	}
	else if (module.CompareNoCase (L"loadperf.dll") == 0)
	{
		result = L"Performance Counter Library";
	}
	else if (module.CompareNoCase (L"pshed.dll") == 0)
	{
		result = L"Platform Specific Hardware Error (PSHED)";
	}
	else if (module.CompareNoCase (L"comres.dll") == 0)
	{
		result = L"COM+ Library";
	}
	else if (module.CompareNoCase (L"ole32.dll") == 0)
	{
		result = L"Object Linking and Embedding (OLE)";
	}
	else if (module.CompareNoCase (L"schedsvc.dll") == 0)
	{
		result = L"Task Scheduler Service";
	}
	else if (module.CompareNoCase (L"twinui.dll") == 0)
	{
		result = L"Twin UI";
	}
	else if (module.CompareNoCase (L"ddputil.dll") == 0)
	{
		result = L"Data Deduplication Library";
	}
	else if (module.CompareNoCase (L"efscore.dll") == 0)
	{
		result = L"Encrypting File System (EFS)";
	}
	else if (module.CompareNoCase (L"msxml3r.dll") == 0)
	{
		result = L"Microsoft XML v3";
	}
	else if (module.CompareNoCase (L"msxml6r.dll") == 0)
	{
		result = L"Microsoft XML v6";
	}

	if (result.IsEmpty ())
	{
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
						result = (LPCWSTR)buffer;
					}
				}
			}

			GlobalFree ((HGLOBAL)info);
		}
	}

	if (result.IsEmpty ())
		result = PathFindFileName (module);

	return result;
}

VOID _app_insert (HWND hwnd, LPCWSTR description, rstring text, BOOL is_internal)
{
	size_t length = text.GetLength () + 1;
	LPWSTR lparam = (LPWSTR)malloc (length * sizeof (WCHAR));

	if (lparam)
	{
		StringCchCopy (lparam, length, text);
	}

	_r_listview_additem (hwnd, IDC_LISTVIEW, description, -1, 0, -1, is_internal ? 1 : 0, (LPARAM)lparam);
	_r_listview_additem (hwnd, IDC_LISTVIEW, text.Mid (0, 70), -1, 1);
}

VOID _app_print (HWND hwnd)
{
	DWORD code = _app_getcode (hwnd);

	rstring buffer;

	// clear first
	_r_listview_deleteallitems (hwnd, IDC_LISTVIEW);

	// dec.
	_r_ctrl_settext (hwnd, IDC_CODE10_CTL, FORMAT_DEC, code);

	// hex.
	_r_ctrl_settext (hwnd, IDC_CODE16_CTL, FORMAT_HEX, code);

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
		// fac. codes (win7.1 sdk)
		SWITCH_ROUTINE (FACILITY_XPS);
		SWITCH_ROUTINE (FACILITY_WINRM);
		SWITCH_ROUTINE (FACILITY_WINDOWSUPDATE);
		SWITCH_ROUTINE (FACILITY_WINDOWS_DEFENDER);
		SWITCH_ROUTINE (FACILITY_WINDOWS_CE);
		SWITCH_ROUTINE (FACILITY_WINDOWS);
		SWITCH_ROUTINE (FACILITY_USERMODE_VOLMGR);
		SWITCH_ROUTINE (FACILITY_USERMODE_VIRTUALIZATION);
		SWITCH_ROUTINE (FACILITY_USERMODE_VHD);
		SWITCH_ROUTINE (FACILITY_URT);
		SWITCH_ROUTINE (FACILITY_UMI);
		SWITCH_ROUTINE (FACILITY_UI);
		SWITCH_ROUTINE (FACILITY_TPM_SOFTWARE);
		SWITCH_ROUTINE (FACILITY_TPM_SERVICES);
		SWITCH_ROUTINE (FACILITY_SXS);
		SWITCH_ROUTINE (FACILITY_STORAGE);
		SWITCH_ROUTINE (FACILITY_STATE_MANAGEMENT);
		SWITCH_ROUTINE (FACILITY_SCARD);
		SWITCH_ROUTINE (FACILITY_SHELL); // FACILITY_NAP
		SWITCH_ROUTINE (FACILITY_SETUPAPI);
		SWITCH_ROUTINE (FACILITY_SECURITY); // FACILITY_SSPI
		SWITCH_ROUTINE (FACILITY_SDIAG);
		SWITCH_ROUTINE (FACILITY_RPC);
		SWITCH_ROUTINE (FACILITY_RAS);
		SWITCH_ROUTINE (FACILITY_PLA);
		SWITCH_ROUTINE (FACILITY_OPC);
		SWITCH_ROUTINE (FACILITY_WIN32);
		SWITCH_ROUTINE (FACILITY_CONTROL);
		SWITCH_ROUTINE (FACILITY_WEBSERVICES); // FACILITY_WINPE
		SWITCH_ROUTINE (FACILITY_NULL);
		SWITCH_ROUTINE (FACILITY_NDIS);
		SWITCH_ROUTINE (FACILITY_METADIRECTORY);
		SWITCH_ROUTINE (FACILITY_MSMQ);
		SWITCH_ROUTINE (FACILITY_MEDIASERVER);
		SWITCH_ROUTINE (FACILITY_INTERNET);
		SWITCH_ROUTINE (FACILITY_ITF);
		SWITCH_ROUTINE (FACILITY_USERMODE_HYPERVISOR);
		SWITCH_ROUTINE (FACILITY_HTTP);
		SWITCH_ROUTINE (FACILITY_GRAPHICS);
		SWITCH_ROUTINE (FACILITY_FWP);
		SWITCH_ROUTINE (FACILITY_FVE);
		SWITCH_ROUTINE (FACILITY_USERMODE_FILTER_MANAGER);
		SWITCH_ROUTINE (FACILITY_DPLAY);
		SWITCH_ROUTINE (FACILITY_DISPATCH);
		SWITCH_ROUTINE (FACILITY_DIRECTORYSERVICE);
		SWITCH_ROUTINE (FACILITY_CONFIGURATION); // FACILITY_WIA
		SWITCH_ROUTINE (FACILITY_COMPLUS);
		SWITCH_ROUTINE (FACILITY_USERMODE_COMMONLOG);
		SWITCH_ROUTINE (FACILITY_CMI);
		SWITCH_ROUTINE (FACILITY_CERT);
		SWITCH_ROUTINE (FACILITY_BCD);
		SWITCH_ROUTINE (FACILITY_BACKGROUNDCOPY);
		SWITCH_ROUTINE (FACILITY_ACS);
		SWITCH_ROUTINE (FACILITY_AAF);

		// fac. codes (win8 sdk)
		SWITCH_ROUTINE (FACILITY_XAML);
		SWITCH_ROUTINE (FACILITY_USN);
		SWITCH_ROUTINE (FACILITY_BLBUI);
		SWITCH_ROUTINE (FACILITY_WSB_ONLINE);
		SWITCH_ROUTINE (FACILITY_DLS);
		SWITCH_ROUTINE (FACILITY_BLB_CLI);
		SWITCH_ROUTINE (FACILITY_BLB);
		SWITCH_ROUTINE (FACILITY_WSBAPP);
		SWITCH_ROUTINE (FACILITY_WPN);
		SWITCH_ROUTINE (FACILITY_WMAAECMA);
		SWITCH_ROUTINE (FACILITY_WINCODEC_DWRITE_DWM);
		SWITCH_ROUTINE (FACILITY_WER);
		SWITCH_ROUTINE (FACILITY_WEB_SOCKET);
		SWITCH_ROUTINE (FACILITY_WEB);
		SWITCH_ROUTINE (FACILITY_USERMODE_VOLSNAP);
		SWITCH_ROUTINE (FACILITY_VISUALCPP);
		SWITCH_ROUTINE (FACILITY_USERMODE_SPACES);
		SWITCH_ROUTINE (FACILITY_SOS);
		SWITCH_ROUTINE (FACILITY_RESTORE); // FACILITY_SPP & FACILITY_DMSERVER
		SWITCH_ROUTINE (FACILITY_SCRIPT);
		SWITCH_ROUTINE (FACILITY_PARSE);
		SWITCH_ROUTINE (FACILITY_POWERSHELL); // FACILITY_MBN
		SWITCH_ROUTINE (FACILITY_PIDGENX);
		SWITCH_ROUTINE (FACILITY_P2P_INT);
		SWITCH_ROUTINE (FACILITY_P2P);
		SWITCH_ROUTINE (FACILITY_ONLINE_ID);
		SWITCH_ROUTINE (FACILITY_MOBILE);
		SWITCH_ROUTINE (FACILITY_LINGUISTIC_SERVICES);
		SWITCH_ROUTINE (FACILITY_LEAP);
		SWITCH_ROUTINE (FACILITY_ACCELERATOR);
		SWITCH_ROUTINE (FACILITY_EAS);
		SWITCH_ROUTINE (FACILITY_EAP);
		SWITCH_ROUTINE (FACILITY_DXGI_DDI);
		SWITCH_ROUTINE (FACILITY_DXGI);
		SWITCH_ROUTINE (FACILITY_DIRECTMUSIC);
		SWITCH_ROUTINE (FACILITY_DIRECT3D11);
		SWITCH_ROUTINE (FACILITY_DIRECT3D10);
		SWITCH_ROUTINE (FACILITY_DIRECT2D);
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
		SWITCH_ROUTINE (FACILITY_AUDCLNT);
		SWITCH_ROUTINE (FACILITY_AUDIO);

		// fac. codes (win8.1 sdk)
		SWITCH_ROUTINE (FACILITY_WEP);
		SWITCH_ROUTINE (FACILITY_TIERING);
		SWITCH_ROUTINE (FACILITY_SYNCENGINE);
		SWITCH_ROUTINE (FACILITY_USERMODE_SDBUS);
		SWITCH_ROUTINE (FACILITY_JSCRIPT);
		SWITCH_ROUTINE (FACILITY_ACTION_QUEUE);

		// fac. codes (win10 sdk)
		SWITCH_ROUTINE (FACILITY_XBOX);
		SWITCH_ROUTINE (FACILITY_UTC);
		SWITCH_ROUTINE (FACILITY_STATEREPOSITORY);
		SWITCH_ROUTINE (FACILITY_SQLITE);
		SWITCH_ROUTINE (FACILITY_USER_MODE_SECURITY_CORE);
		SWITCH_ROUTINE (FACILITY_USERMODE_LICENSING);
		SWITCH_ROUTINE (FACILITY_DIRECT3D12_DEBUG);
		SWITCH_ROUTINE (FACILITY_DIRECT3D12);
		SWITCH_ROUTINE (FACILITY_DIRECT3D11_DEBUG);
		SWITCH_ROUTINE (FACILITY_DELIVERY_OPTIMIZATION);

		// fac. codes (directx2010 sdk)
		SWITCH_ROUTINE (FACILITY_AUDIO_KERNEL);
		//SWITCH_ROUTINE (FACILITY_BTH_ATT);
		SWITCH_ROUTINE (FACILITY_CODCLASS_ERROR_CODE);
		SWITCH_ROUTINE (FACILITY_FILTER_MANAGER);
		SWITCH_ROUTINE (FACILITY_GRAPHICS_KERNEL);
		//SWITCH_ROUTINE (FACILITY_INTERIX);
		SWITCH_ROUTINE (FACILITY_MONITOR);
		SWITCH_ROUTINE (FACILITY_WIN32K_NTGDI);
		//SWITCH_ROUTINE (FACILITY_WIN32K_NTUSER);
		SWITCH_ROUTINE (FACILITY_RDBSS);
		SWITCH_ROUTINE (FACILITY_RESUME_KEY_FILTER);
		SWITCH_ROUTINE (FACILITY_SECUREBOOT);
		SWITCH_ROUTINE (FACILITY_SHARED_VHDX);
		//SWITCH_ROUTINE (FACILITY_SPACES);
		//SWITCH_ROUTINE (FACILITY_VIDEO);
		SWITCH_ROUTINE (FACILITY_D3D);
		SWITCH_ROUTINE (FACILITY_D3DX);
		//SWITCH_ROUTINE (FACILITY_DSOUND_DMUSIC);
		//SWITCH_ROUTINE (FACILITY_D3D10);
		SWITCH_ROUTINE (FACILITY_XAUDIO2);
		SWITCH_ROUTINE (FACILITY_XAPO);
		SWITCH_ROUTINE (FACILITY_XACTENGINE);
		//SWITCH_ROUTINE (FACILITY_D3D11_OR_AE);
		//SWITCH_ROUTINE (FACILITY_D2D);
		//SWITCH_ROUTINE (FACILITY_DWRITE);
		//SWITCH_ROUTINE (FACILITY_APO);
		//SWITCH_ROUTINE (FACILITY_WSAPI);
		//SWITCH_ROUTINE (FACILITY_EAP_MESSAGE);

		default:
		{
			buffer.Format (L"0x%02x", HRESULT_FACILITY (code));
			break;
		}
	}

	SetDlgItemText (hwnd, IDC_FACILITY_CTL, buffer);

	// print (system)
	rstring::vector vc = app.ConfigGet (L"SystemModule", SYSTEM_MODULES_DEFAULT).AsVector (L";");

	for (size_t i = 0; i < vc.size (); i++)
	{
		buffer = _app_formatmessage (code, vc.at (i));

		if (!buffer.IsEmpty ())
		{
			_app_insert (hwnd, _app_getdescription (vc.at (i)), buffer, FALSE);
		}
	}

	// print (internal)
	if (app.ConfigGet (L"InternalModuleCPP", 1).AsBool ())
	{
		_wcserror_s (buffer.GetBuffer (4096), 4096, code);
		buffer.ReleaseBuffer ();

		if (buffer.CompareNoCase (L"Unknown error") != 0)
		{
			_app_insert (hwnd, I18N (&app, IDS_MODULE_INTERNAL_CPP, 0), buffer, TRUE);
		}
	}

	if (app.ConfigGet (L"InternalModuleDX", 1).AsBool ())
	{
		buffer = DXGetErrorDescription (HRESULT (code));

		if (buffer.CompareNoCase (L"n/a") != 0)
		{
			_app_insert (hwnd, I18N (&app, IDS_MODULE_INTERNAL_DX, 0), buffer, TRUE);
		}
	}

	// show description for first item
	_app_showdescription (hwnd, 0);
}

BOOL settings_callback (HWND hwnd, DWORD msg, LPVOID lpdata1, LPVOID lpdata2)
{
	PAPPLICATION_PAGE page = (PAPPLICATION_PAGE)lpdata2;

	switch (msg)
	{
		case _RM_INITIALIZE:
		{
			switch (page->dlg_id)
			{
				case IDD_SETTINGS_1:
				{
					// localize
					SetDlgItemText (hwnd, IDC_TITLE_1, I18N (&app, IDS_TITLE_1, 0));
					SetDlgItemText (hwnd, IDC_TITLE_2, I18N (&app, IDS_TITLE_2, 0));

					SetDlgItemText (hwnd, IDC_ALWAYSONTOP_CHK, I18N (&app, IDS_ALWAYSONTOP_CHK, 0));
					SetDlgItemText (hwnd, IDC_INSERTBUFFER_CHK, I18N (&app, IDS_INSERTBUFFER_CHK, 0));
					SetDlgItemText (hwnd, IDC_CHECKUPDATES_CHK, I18N (&app, IDS_CHECKUPDATES_CHK, 0));

					SetDlgItemText (hwnd, IDC_LANGUAGE_HINT, I18N (&app, IDS_LANGUAGE_HINT, 0));

					// set checks
					CheckDlgButton (hwnd, IDC_ALWAYSONTOP_CHK, app.ConfigGet (L"AlwaysOnTop", 0).AsBool () ? BST_CHECKED : BST_UNCHECKED);
					CheckDlgButton (hwnd, IDC_INSERTBUFFER_CHK, app.ConfigGet (L"InsertBufferAtStartup", 1).AsBool () ? BST_CHECKED : BST_UNCHECKED);
					CheckDlgButton (hwnd, IDC_CHECKUPDATES_CHK, app.ConfigGet (L"CheckUpdates", 1).AsBool () ? BST_CHECKED : BST_UNCHECKED);

					app.LocaleEnum (hwnd, IDC_LANGUAGE);

					SetWindowLongPtr (hwnd, GWLP_USERDATA, (LONG_PTR)SendDlgItemMessage (hwnd, IDC_LANGUAGE, CB_GETCURSEL, 0, 0)); // check on save

					break;
				}

				case IDD_SETTINGS_2:
				{
					_r_listview_setstyle (hwnd, IDC_MODULES, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP | LVS_EX_CHECKBOXES);

					_r_listview_deleteallitems (hwnd, IDC_MODULES);
					_r_listview_deleteallcolumns (hwnd, IDC_MODULES);

					_r_listview_addcolumn (hwnd, IDC_MODULES, I18N (&app, IDS_COLUMN_1, 0), 30, 0, LVCFMT_LEFT);
					_r_listview_addcolumn (hwnd, IDC_MODULES, I18N (&app, IDS_COLUMN_2, 0), 65, 1, LVCFMT_LEFT);

					rstring str = app.ConfigGet (L"SystemModule", SYSTEM_MODULES_DEFAULT);

					INT item = 0;
					rstring::vector vc = str.AsVector (L";");

					for (size_t i = 0; i < vc.size (); i++)
					{
						BOOL is_enabled = vc.at (i).At (0) == L'~' ? FALSE : TRUE;

						if (!is_enabled)
						{
							vc.at (i) = vc.at (i).Mid (1);
						}

						if (!vc.at (i).IsEmpty ())
						{
							_r_listview_additem (hwnd, IDC_MODULES, vc.at (i), item, 0);
							_r_listview_additem (hwnd, IDC_MODULES, _app_getdescription (vc.at (i)), item, 1);

							ListView_SetCheckState (GetDlgItem (hwnd, IDC_MODULES), item++, is_enabled);
						}
					}

					break;
				}

				case IDD_SETTINGS_3:
				{
					// localize
					SetDlgItemText (hwnd, IDC_TITLE_3, I18N (&app, IDS_TITLE_3, 0));
					SetDlgItemText (hwnd, IDC_MODULE_INTERNAL_CPP, I18N (&app, IDS_MODULE_INTERNAL_CPP, 0));
					SetDlgItemText (hwnd, IDC_MODULE_INTERNAL_DX, I18N (&app, IDS_MODULE_INTERNAL_DX, 0));

					// set checks
					CheckDlgButton (hwnd, IDC_MODULE_INTERNAL_CPP, app.ConfigGet (L"InternalModuleCPP", 1).AsBool () ? BST_CHECKED : BST_UNCHECKED);
					CheckDlgButton (hwnd, IDC_MODULE_INTERNAL_DX, app.ConfigGet (L"InternalModuleDX", 1).AsBool () ? BST_CHECKED : BST_UNCHECKED);

					break;
				}
			}

			break;
		}

		case _RM_SAVE:
		{
			switch (page->dlg_id)
			{
				case IDD_SETTINGS_1:
				{
					app.ConfigSet (L"AlwaysOnTop", (IsDlgButtonChecked (hwnd, IDC_ALWAYSONTOP_CHK) == BST_CHECKED) ? TRUE : FALSE);
					app.ConfigSet (L"InsertBufferAtStartup", (IsDlgButtonChecked (hwnd, IDC_INSERTBUFFER_CHK) == BST_CHECKED) ? TRUE : FALSE);
					app.ConfigSet (L"CheckUpdates", (IsDlgButtonChecked (hwnd, IDC_CHECKUPDATES_CHK) == BST_CHECKED) ? TRUE : FALSE);

					// set language
					rstring buffer;

					if (SendDlgItemMessage (hwnd, IDC_LANGUAGE, CB_GETCURSEL, 0, 0) >= 1)
					{
						buffer = _r_ctrl_gettext (hwnd, IDC_LANGUAGE);
					}

					app.ConfigSet (L"Language", buffer);

					if (GetWindowLongPtr (hwnd, GWLP_USERDATA) != (INT)SendDlgItemMessage (hwnd, IDC_LANGUAGE, CB_GETCURSEL, 0, 0))
					{
						return TRUE; // for restart
					}

					break;
				}

				case IDD_SETTINGS_2:
				{
					INT item = -1;

					rstring buffer;

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

				case IDD_SETTINGS_3:
				{
					app.ConfigSet (L"InternalModuleCPP", (IsDlgButtonChecked (hwnd, IDC_MODULE_INTERNAL_CPP) == BST_CHECKED) ? TRUE : FALSE);
					app.ConfigSet (L"InternalModuleDX", (IsDlgButtonChecked (hwnd, IDC_MODULE_INTERNAL_DX) == BST_CHECKED) ? TRUE : FALSE);

					break;
				}
			}

			break;
		}

		case _RM_MESSAGE:
		{
			LPMSG pmsg = (LPMSG)lpdata1;

			INT iHeight = 0;
			POINT p;
			static HIMAGELIST hDragImageList;
			HIMAGELIST hOneImageList, hTempImageList;
			static BOOL bDragging;
			LVHITTESTINFO lvhti;
			BOOL bFirst;
			IMAGEINFO imf;

			switch (pmsg->message)
			{
				case WM_MOUSEMOVE:
				{
					if (bDragging && GetDlgItem (hwnd, IDC_MODULES))
					{
						lvhti.pt.x = LOWORD (pmsg->lParam);
						lvhti.pt.y = HIWORD (pmsg->lParam);

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
						lvhti.pt.x = LOWORD (pmsg->lParam);
						lvhti.pt.y = HIWORD (pmsg->lParam);

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
					if (GetDlgCtrlID ((HWND)pmsg->wParam) == IDC_MODULES)
					{
						HMENU menu = LoadMenu (nullptr, MAKEINTRESOURCE (IDM_MODULES)), submenu = GetSubMenu (menu, 0);

						// localize
						app.LocaleMenu (submenu, I18N (&app, IDS_ADD, 0), IDM_ADD, FALSE);
						app.LocaleMenu (submenu, I18N (&app, IDS_DELETE, 0), IDM_DELETE, FALSE);

						if (!SendDlgItemMessage (hwnd, IDC_MODULES, LVM_GETSELECTEDCOUNT, 0, 0))
						{
							EnableMenuItem (submenu, IDM_DELETE, MF_BYCOMMAND | MF_DISABLED);
						}

						TrackPopupMenuEx (submenu, TPM_RIGHTBUTTON | TPM_LEFTBUTTON, LOWORD (pmsg->lParam), HIWORD (pmsg->lParam), hwnd, nullptr);

						DestroyMenu (menu);
						DestroyMenu (submenu);
					}

					break;
				}

				case WM_NOTIFY:
				{
					if (((LPNMHDR)pmsg->lParam)->idFrom == IDC_MODULES)
					{
						switch (((LPNMHDR)pmsg->lParam)->code)
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

								INT iPos = ListView_GetNextItem (((LPNMHDR)pmsg->lParam)->hwndFrom, -1, LVNI_SELECTED);

								while (iPos != -1)
								{
									if (bFirst)
									{
										// For the first selected item,
										// we simply create a single-line drag image
										hDragImageList = ListView_CreateDragImage (((LPNMHDR)pmsg->lParam)->hwndFrom, iPos, &p);
										ImageList_GetImageInfo (hDragImageList, 0, &imf);
										iHeight = imf.rcImage.bottom;
										bFirst = FALSE;
									}
									else
									{
										// For the rest selected items,
										// we create a single-line drag image, then
										// append it to the bottom of the complete drag image
										hOneImageList = ListView_CreateDragImage (((LPNMHDR)pmsg->lParam)->hwndFrom, iPos, &p);
										hTempImageList = ImageList_Merge (hDragImageList, 0, hOneImageList, 0, 0, iHeight);
										ImageList_Destroy (hDragImageList);
										ImageList_Destroy (hOneImageList);
										hDragImageList = hTempImageList;
										ImageList_GetImageInfo (hDragImageList, 0, &imf);
										iHeight = imf.rcImage.bottom;
									}

									iPos = ListView_GetNextItem (((LPNMHDR)pmsg->lParam)->hwndFrom, iPos, LVNI_SELECTED);
								}

								// Now we can initialize then start the drag action
								ImageList_BeginDrag (hDragImageList, 0, 0, 0);

								POINT pt = ((NM_LISTVIEW*)((LPNMHDR)pmsg->lParam))->ptAction;
								ClientToScreen (((LPNMHDR)pmsg->lParam)->hwndFrom, &pt);

								ImageList_DragEnter (GetDesktopWindow (), pt.x, pt.y);

								bDragging = TRUE;

								// Don't forget to capture the mouse
								SetCapture (hwnd);

								break;
							}

							case NM_CUSTOMDRAW:
							{
								LONG result = CDRF_DODEFAULT;
								LPNMLVCUSTOMDRAW lpnmlv = (LPNMLVCUSTOMDRAW)pmsg->lParam;

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
											HMODULE h = LoadLibraryEx (_r_listview_gettext (hwnd, (INT)lpnmlv->nmcd.hdr.idFrom, (INT)lpnmlv->nmcd.dwItemSpec, 0), nullptr, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);

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

					break;
				}

				case WM_COMMAND:
				{
					switch (LOWORD (pmsg->wParam))
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
								INT item = (INT)SendDlgItemMessage (hwnd, IDC_MODULES, LVM_GETNEXTITEM, static_cast<WPARAM>(-1), LVNI_SELECTED) + 1;

								_r_listview_additem (hwnd, IDC_MODULES, file, item, 0);
								_r_listview_additem (hwnd, IDC_MODULES, _app_getdescription (file), item, 1);

								ListView_SetCheckState (GetDlgItem (hwnd, IDC_MODULES), item, TRUE);
							}

							break;
						}

						case IDM_DELETE:
						{
							INT count = (INT)SendDlgItemMessage (hwnd, IDC_MODULES, LVM_GETITEMCOUNT, 0, 0) - 1;

							for (INT i = count; i >= 0; i--)
							{
								if (ListView_GetItemState (GetDlgItem (hwnd, IDC_MODULES), i, LVNI_SELECTED))
								{
									SendDlgItemMessage (hwnd, IDC_MODULES, LVM_DELETEITEM, i, 0);
								}
							}

							break;
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

BOOL initializer_callback (HWND hwnd, DWORD msg, LPVOID, LPVOID)
{
	switch (msg)
	{
		case _RM_INITIALIZE:
		{
			// get locale id
			lcid = I18N (&app, IDS_LCID, 0).AsUlong (16);

			// clear
			_r_listview_deleteallitems (hwnd, IDC_LISTVIEW);
			_r_listview_deleteallgroups (hwnd, IDC_LISTVIEW);
			_r_listview_deleteallcolumns (hwnd, IDC_LISTVIEW);

			// localize
			_r_listview_addcolumn (hwnd, IDC_LISTVIEW, I18N (&app, IDS_COLUMN_1, 0), app.ConfigGet (L"Column1", 40).AsInt (), 0, LVCFMT_LEFT);
			_r_listview_addcolumn (hwnd, IDC_LISTVIEW, I18N (&app, IDS_COLUMN_2, 0), app.ConfigGet (L"Column2", 60).AsInt (), 1, LVCFMT_LEFT);

			_r_listview_addgroup (hwnd, IDC_LISTVIEW, 0, I18N (&app, IDS_GROUP_1, 0), 0, 0);
			_r_listview_addgroup (hwnd, IDC_LISTVIEW, 1, I18N (&app, IDS_GROUP_2, 0), 0, 0);

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

			_app_print (hwnd);

			break;
		}
	}

	return FALSE;
}

INT_PTR CALLBACK DlgProc (HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			// configure listview
			_r_listview_setstyle (hwnd, IDC_LISTVIEW, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);

			// configure controls
			SendDlgItemMessage (hwnd, IDC_CODE_UD, UDM_SETRANGE32, 0, INT32_MAX);

			if (app.ConfigGet (L"InsertBufferAtStartup", 1).AsBool ())
			{
				SetDlgItemText (hwnd, IDC_CODE, _r_clipboard_get (hwnd));
			}
			else
			{
				SetDlgItemText (hwnd, IDC_CODE, app.ConfigGet (L"LatestCode", 0));
			}

			// settings
			app.AddSettingsPage (nullptr, IDD_SETTINGS_1, IDS_SETTINGS_1, L"IDS_SETTINGS_1", &settings_callback);
			app.AddSettingsPage (nullptr, IDD_SETTINGS_2, IDS_SETTINGS_2, L"IDS_SETTINGS_2", &settings_callback);
			app.AddSettingsPage (nullptr, IDD_SETTINGS_3, IDS_SETTINGS_3, L"IDS_SETTINGS_3", &settings_callback);

			break;
		}

		case WM_DESTROY:
		{
			app.ConfigSet (L"LatestCode", _r_ctrl_gettext (hwnd, IDC_CODE));

			app.ConfigSet (L"Column1", _r_listview_getcolumnwidth (hwnd, IDC_LISTVIEW, 0));
			app.ConfigSet (L"Column2", _r_listview_getcolumnwidth (hwnd, IDC_LISTVIEW, 1));

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

				if (!SendDlgItemMessage (hwnd, IDC_LISTVIEW, LVM_GETSELECTEDCOUNT, 0, 0))
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
					if (hdr->idFrom == IDC_LISTVIEW)
					{
						_app_showdescription (hwnd, LPNMITEMACTIVATE (lparam)->iItem);
					}

					break;
				}

				case LVN_DELETEITEM:
				{
					if (hdr->idFrom == IDC_LISTVIEW)
					{
						LPNMLISTVIEW pnmlv = (LPNMLISTVIEW)lparam;

						free ((LPVOID)pnmlv->lParam);
					}

					break;
				}

				case UDN_DELTAPOS:
				{
					if (hdr->idFrom == IDC_CODE_UD)
					{
						_r_ctrl_settext (hwnd, IDC_CODE, FORMAT_DEC, _app_getcode (hwnd) + LPNMUPDOWN (lparam)->iDelta);
						_app_print(hwnd);

						return TRUE;
					}

					break;
				}
			}

			break;
		}

		case WM_COMMAND:
		{
			if (LOWORD (wparam) == IDC_CODE && HIWORD (wparam) == EN_CHANGE)
			{
				_app_print (hwnd);
				break;
			}

			switch (LOWORD (wparam))
			{
				case IDM_SETTINGS:
				{
					app.CreateSettingsWindow ();
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
					ShellExecute (hwnd, nullptr, _APP_WEBSITE_URL, nullptr, nullptr, SW_SHOWDEFAULT);
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
					rstring buffer;

					INT item = -1;

					while ((item = (INT)SendDlgItemMessage (hwnd, IDC_LISTVIEW, LVM_GETNEXTITEM, item, LVNI_SELECTED)) != -1)
					{
						buffer.Append ((LPCWSTR)_r_listview_getlparam (hwnd, IDC_LISTVIEW, item));
						buffer.Append (L"\r\n");
					}

					if (!buffer.IsEmpty ())
					{
						buffer.Trim (L"\r\n");

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
	if (app.CreateMainWindow (&DlgProc, &initializer_callback))
	{
		MSG msg = {0};

		while (GetMessage (&msg, nullptr, 0, 0) > 0)
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
