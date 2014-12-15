/************************************
*  	Error Lookup
*	Copyright © 2012 Henry++
*
*	GNU General Public License v2
*	http://www.gnu.org/licenses/
*
*	http://www.henrypp.org/
*************************************/

// Include
#include <windows.h>
#include <commctrl.h>
#include <wininet.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <iphlpapi.h>
#include <ras.h>
#include <process.h>
#include <dxerr.h> // DXSDK Needed

#include "errorlookup.h"
#include "extended.h" // extended codes
#include "resource.h"
#include "about.h"
#include "ini.h"

INI cfg;

// Check Updates
void CheckUpdates(LPVOID lpParam)
{
	unsigned long ulReaded = 0;
	char szBufferA[50] = {0};
    wchar_t szBufferW[50] = {0};

	DWORD dwStatus = 0, dwStatusSize = sizeof(dwStatus);

	// Disable Menu
	EnableMenuItem(GetMenu(hMainDlg), IDM_CHECK_UPDATES, MF_BYCOMMAND | MF_DISABLED);

	// Connect
	HINTERNET hInternet = InternetOpen(APP_NAME L" " APP_VERSION, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	HINTERNET hConnect = InternetOpenUrl(hInternet, APP_WEBSITE L"/update.php?product=" APP_NAME_SHORT, NULL, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);

	// Get Status
	HttpQueryInfo(hConnect, HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE, &dwStatus, &dwStatusSize, NULL);

	// Reading
	InternetReadFile(hConnect, szBufferA, 50, &ulReaded);

	// Check Errors
	if(!hInternet || !hConnect || dwStatus != HTTP_STATUS_OK)
	{
		if(!lpParam)
			MessageBox(hMainDlg, L"Ошибка подключения к серверу обновления", APP_NAME, MB_OK | MB_ICONSTOP);
	}
	else
	{
		// Convert to Unicode
		MultiByteToWideChar(CP_UTF8, 0, szBufferA, 50, szBufferW, 50);

		// If NEWVER == CURVER
		if(lstrcmpi(szBufferW, APP_VERSION) == 0)
		{
			if(!lpParam)
				MessageBox(hMainDlg, L"Вы используте последнюю версию программы", APP_NAME, MB_OK | MB_ICONINFORMATION);
		}
		else
		{
			wchar_t szBuffer[MAX_PATH] = {0};
			StringCchPrintf(szBuffer, MAX_PATH, L"Доступна новая версия программы: %s\r\nВы хотите открыть страницу загрузки новой версии?\0", szBufferW);

			if(MessageBox(hMainDlg, szBuffer, APP_NAME, MB_YESNO | MB_ICONQUESTION) == IDYES)
				ShellExecute(hMainDlg, L"open", APP_WEBSITE, NULL, NULL, SW_SHOW);
		}
	}

	// Enable Menu
	EnableMenuItem(GetMenu(hMainDlg), IDM_CHECK_UPDATES, MF_BYCOMMAND | MF_ENABLED);

	// Clear Memory
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hInternet);
}

// Hex to Int
int _httoi(const wchar_t *value)
{
	struct CHexMap
	{
		wchar_t chr;
		int value;
	};

	CHexMap HexMap[16] =
	{
		{'0', 0}, {'1', 1},
		{'2', 2}, {'3', 3},
		{'4', 4}, {'5', 5},
		{'6', 6}, {'7', 7},
		{'8', 8}, {'9', 9},
		{'A', 10}, {'B', 11},
		{'C', 12}, {'D', 13},
		{'E', 14}, {'F', 15}
	};

	wchar_t *mstr = _wcsupr(_wcsdup(value)), *s = mstr;;

	int result = 0;
	if (*s == '0' && *(s + 1) == 'X') s += 2;
	bool firsttime = 1;

	while (*s != '\0')
	{
		bool found = 0;
		for (int i = 0; i < 16; i++)
		{
			if (*s == HexMap[i].chr)
			{
				if(!firsttime)
					result <<= 4;

				result |= HexMap[i].value;
				found = 1;

				break;
			}
		}

		if (!found)
			break;

		s++;
		firsttime = 0;
	}

	free(mstr);

	return result;
}

// Get Text From Clipboard and Put to Input Control
void InsertClipboardText(HWND hWnd, int idCtrl)
{
	HGLOBAL hGlb = NULL;
	
	wchar_t* szRet = {0};

	if(OpenClipboard(hWnd))
	{
		hGlb = GetClipboardData(CF_UNICODETEXT);

		if(hGlb)
		{
			szRet = (wchar_t*)GlobalLock(hGlb);

			if(szRet)
				GlobalUnlock(hGlb);

			// Insert Text From Clipboard
			for(unsigned int i = 0; i < wcslen(szRet); i++)
				SendDlgItemMessage(hWnd, idCtrl, WM_CHAR, szRet[i], 1);
		}
	}

	CloseClipboard();
}

// Get Error Index from Table
int GetErrorIndex(ERROR_TABLE* table, int iCode, size_t cbStruct)
{
	for(size_t i = 0; i < cbStruct; i++)
	{
		if(table[i].code == iCode)
			return i;
	}

	return -1;
}

// Error Engine
bool ErrorOut(HWND hWnd, DWORD dwError, int iMode)
{
	HMODULE hModule = NULL;
	HRESULT hError = (HRESULT)dwError;

	wchar_t szError[MAX_PATH] = {0}, szBuffer[MAX_PATH] = {0}, *szFormat = L"Код (DEC): %d\r\nКод (HEX): 0x%08X\r\nИмя: %s\r\nSeverity: %s\r\nFacility: %s (%d)\0";
	wchar_t szFacility[100] = {0}, szSeverity[100] = {0};

	bool bRet = 0;
	int iBuffer = 0;

	// Severity Codes
    switch((dwError & 0xC0000000) >> 30)
    {
		case STATUS_SEVERITY_SUCCESS:
			wcsncpy_s(szSeverity, L"Success", 100);
			break;
			
		case STATUS_SEVERITY_INFORMATIONAL:
			wcsncpy_s(szSeverity, L"Informational", 100);
			break;
			
		case STATUS_SEVERITY_WARNING:
			wcsncpy_s(szSeverity, L"Warning", 100);
			break;

		case STATUS_SEVERITY_ERROR:
			wcsncpy_s(szSeverity, L"Error", 100);
			break;

        default:
			wcsncpy_s(szSeverity, L"n/a", 100);
			break;
	}

	// Facility Codes
    switch((dwError & 0x0FFF0000) >> 16)
    {
        FACILITY(FACILITY_WINRM)
        FACILITY(FACILITY_WINDOWSUPDATE)
        FACILITY(FACILITY_WINDOWS_DEFENDER)
        FACILITY(FACILITY_WINDOWS_CE)
        FACILITY(FACILITY_WINDOWS)
        FACILITY(FACILITY_USERMODE_VOLMGR)
        FACILITY(FACILITY_USERMODE_VIRTUALIZATION)
        FACILITY(FACILITY_URT)
        FACILITY(FACILITY_UMI)
        FACILITY(FACILITY_TPM_SOFTWARE)
        FACILITY(FACILITY_TPM_SERVICES)
        FACILITY(FACILITY_SXS)
        FACILITY(FACILITY_STORAGE)
        FACILITY(FACILITY_STATE_MANAGEMENT)
        FACILITY(FACILITY_SCARD)
        FACILITY(FACILITY_SHELL)
        FACILITY(FACILITY_SETUPAPI)
        FACILITY(FACILITY_SECURITY)
        FACILITY(FACILITY_RPC)
        FACILITY(FACILITY_PLA)
        FACILITY(FACILITY_WIN32)
        FACILITY(FACILITY_CONTROL)
        FACILITY(FACILITY_NULL)
        FACILITY(FACILITY_NDIS)
        FACILITY(FACILITY_METADIRECTORY)
        FACILITY(FACILITY_MSMQ)
        FACILITY(FACILITY_MEDIASERVER)
        FACILITY(FACILITY_INTERNET)
        FACILITY(FACILITY_ITF)
        FACILITY(FACILITY_USERMODE_HYPERVISOR)
        FACILITY(FACILITY_HTTP)
        FACILITY(FACILITY_GRAPHICS)
        FACILITY(FACILITY_FWP)
        FACILITY(FACILITY_FVE)
        FACILITY(FACILITY_USERMODE_FILTER_MANAGER)
        FACILITY(FACILITY_DPLAY)
        FACILITY(FACILITY_DISPATCH)
        FACILITY(FACILITY_DIRECTORYSERVICE)
        FACILITY(FACILITY_CONFIGURATION)
        FACILITY(FACILITY_COMPLUS)
        FACILITY(FACILITY_USERMODE_COMMONLOG)
        FACILITY(FACILITY_CMI)
        FACILITY(FACILITY_CERT)
        FACILITY(FACILITY_BCD)
        FACILITY(FACILITY_BACKGROUNDCOPY)
        FACILITY(FACILITY_ACS)
        FACILITY(FACILITY_AAF)
        FACILITY(FACILITY_D3D)
        FACILITY(FACILITY_D3DX)
        FACILITY(FACILITY_DSOUND_DMUSIC)
        FACILITY(FACILITY_D3D10)
        FACILITY(FACILITY_DXGI)
        FACILITY(FACILITY_XAUDIO2)
        FACILITY(FACILITY_XAPO)
        FACILITY(FACILITY_XACTENGINE)
        FACILITY(FACILITY_D3D11_OR_AE)
        FACILITY(FACILITY_D2D)
        FACILITY(FACILITY_DWRITE)
        FACILITY(FACILITY_APO)
        FACILITY(FACILITY_LEAP)
        FACILITY(FACILITY_WSAPI)

        default:
			wcsncpy_s(szFacility, L"n/a", 100);
			break;
    }

	switch(iMode)
	{
		// Windows
		case 0:
			if(FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwError, 0, szBuffer, MAX_PATH, NULL))
			{
				SetDlgItemText(hWnd, IDC_ERROR_TEXT, szBuffer);
				StringCchPrintf(szError, MAX_PATH, szFormat, dwError, hError, L"n/a", szSeverity, szFacility, (dwError & 0x0FFF0000) >> 16);

				bRet = 1;
			}

			break;

		// DirectX
		case 1:
			SetDlgItemText(hWnd, IDC_ERROR_TEXT, DXGetErrorDescription(hError));
			StringCchPrintf(szError, MAX_PATH, szFormat, dwError, hError, DXGetErrorString(hError), szSeverity, szFacility, (dwError & 0x0FFF0000) >> 16);
			bRet = 1;

			break;

		// NTSTATUS
		case 2:
			hModule = LoadLibrary(L"ntdll.dll");

			if(FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS, hModule, dwError, 0, szBuffer, MAX_PATH, NULL))
			{
				SetDlgItemText(hWnd, IDC_ERROR_TEXT, szBuffer);
				StringCchPrintf(szError, MAX_PATH, szFormat, dwError, hError, L"n/a", szSeverity, szFacility, (dwError & 0x0FFF0000) >> 16);

				bRet = 1;
			}
			
			break;
		
		// Wininet
		case 3:
			hModule = LoadLibrary(L"wininet.dll");

			if(FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS, hModule, dwError, 0, szBuffer, MAX_PATH, NULL))
			{
				SetDlgItemText(hWnd, IDC_ERROR_TEXT, szBuffer);
				StringCchPrintf(szError, MAX_PATH, szFormat, dwError, hError, L"n/a", szSeverity, szFacility, (dwError & 0x0FFF0000) >> 16);

				bRet = 1;
			}

            break;
			
		// Dev Manager
		case 4:
			iBuffer = GetErrorIndex(devmgr_table, dwError, sizeof(devmgr_table) / sizeof(devmgr_table[0]));

			if(iBuffer != -1)
			{
				SetDlgItemText(hWnd, IDC_ERROR_TEXT, devmgr_table[iBuffer].description ? devmgr_table[iBuffer].description : L"n/a");
				StringCchPrintf(szError, MAX_PATH, szFormat, dwError, hError, devmgr_table[iBuffer].name ? devmgr_table[iBuffer].name : L"n/a", L"n/a", L"n/a", 0);

				bRet = 1;
			}

            break;
			
		// STOP
		case 5:
			iBuffer = GetErrorIndex(stop_table, dwError, sizeof(stop_table) / sizeof(stop_table[0]));

			if(iBuffer != -1)
			{
				SetDlgItemText(hWnd, IDC_ERROR_TEXT, stop_table[iBuffer].description ? stop_table[iBuffer].description : L"n/a");
				StringCchPrintf(szError, MAX_PATH, szFormat, dwError, hError, stop_table[iBuffer].name ? stop_table[iBuffer].name : L"n/a", L"n/a", L"n/a", 0);

				bRet = 1;
			}

            break;
			
		// RAS
		case 6:
			if(RasGetErrorString(dwError, szBuffer, MAX_PATH) == ERROR_SUCCESS)
			{
				SetDlgItemText(hWnd, IDC_ERROR_TEXT, szBuffer);
				StringCchPrintf(szError, MAX_PATH, szFormat, dwError, hError, L"n/a", L"n/a", L"n/a", 0);

				bRet = 1;
			}
			
            break;

		// IP Helper
		case 7:
			DWORD dwSize = MAX_PATH;
			if(GetIpErrorString(dwError, szBuffer, &dwSize) == NO_ERROR)
			{
				SetDlgItemText(hWnd, IDC_ERROR_TEXT, szBuffer);
				StringCchPrintf(szError, MAX_PATH, szFormat, dwError, hError, L"n/a", szSeverity, szFacility, (dwError & 0x0FFF0000) >> 16);

				bRet = 1;
			}
			
            break;
	}

	// Show Result
	if(bRet)
		SetDlgItemText(hWnd, IDC_ERROR_ADVANCED, szError);

	// Clear memory
	if(hModule)
		FreeLibrary(hModule);

	return bRet;
}

LRESULT CALLBACK DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	wchar_t szBuffer[MAX_PATH] = {0};
	int iBuffer = 0;

	switch(uMsg)
	{
		case WM_INITDIALOG:
		{
			// Check Mutex
			HANDLE hMutex = CreateMutex(NULL, TRUE, APP_NAME_SHORT);

			if(GetLastError() == ERROR_ALREADY_EXISTS)
			{
				CloseHandle(hMutex);
				ExitProcess(0);
			}

			// Set Title
			SetWindowText(hwndDlg, APP_NAME L" " APP_VERSION);

			// Handles
			hMainDlg = hwndDlg;

			// Modify System Menu
			HMENU hMenu = GetSystemMenu(hwndDlg, 0);
			InsertMenu(hMenu, -1, MF_BYCOMMAND | MF_SEPARATOR, 0, 0);
			InsertMenu(hMenu, -1, MF_BYCOMMAND | MF_STRING, IDM_ABOUT, L"О программе");

			// Load Settings
			GetModuleFileName(0, szBuffer, MAX_PATH);
			PathRenameExtension(szBuffer, L".cfg");
			cfg.load(szBuffer);

			// Set Icons
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, 32, 32, 0));
			SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, 16, 16, 0));
			
			// Insert Data Types
			SendDlgItemMessage(hwndDlg, IDC_ERROR_DATA, CB_INSERTSTRING, 0, (LPARAM)L"DEC");
			SendDlgItemMessage(hwndDlg, IDC_ERROR_DATA, CB_INSERTSTRING, 1, (LPARAM)L"HEX");

			SendDlgItemMessage(hwndDlg, IDC_ERROR_DATA, CB_SETCURSEL, cfg.read(APP_NAME_SHORT, L"DataType", 0), 0);

			// Insert Error Modes
			SendDlgItemMessage(hwndDlg, IDC_ERROR_MODE, CB_INSERTSTRING, 0, (LPARAM)L"WINDOWS");
			SendDlgItemMessage(hwndDlg, IDC_ERROR_MODE, CB_INSERTSTRING, 1, (LPARAM)L"DIRECTX");
			SendDlgItemMessage(hwndDlg, IDC_ERROR_MODE, CB_INSERTSTRING, 2, (LPARAM)L"NTSTATUS");
			SendDlgItemMessage(hwndDlg, IDC_ERROR_MODE, CB_INSERTSTRING, 3, (LPARAM)L"WININET");
			SendDlgItemMessage(hwndDlg, IDC_ERROR_MODE, CB_INSERTSTRING, 4, (LPARAM)L"ДИСПЕТЧЕР УСТРОЙСТВ");
			SendDlgItemMessage(hwndDlg, IDC_ERROR_MODE, CB_INSERTSTRING, 5, (LPARAM)L"STOP");
			SendDlgItemMessage(hwndDlg, IDC_ERROR_MODE, CB_INSERTSTRING, 6, (LPARAM)L"RAS");
			SendDlgItemMessage(hwndDlg, IDC_ERROR_MODE, CB_INSERTSTRING, 7, (LPARAM)L"IP HELPER");

			SendDlgItemMessage(hwndDlg, IDC_ERROR_MODE, CB_SETCURSEL, cfg.read(APP_NAME_SHORT, L"ErrorMode", 0), 0);

			// Configure Edit Control
			MaskEditControl(GetDlgItem(hwndDlg, IDC_ERROR_ID), "0123456789abcdefxABCDEFX-\b\0", 1);
			SendDlgItemMessage(hwndDlg, IDC_ERROR_ID, EM_SETLIMITTEXT, 12, 0);

			// Check Updates
			iBuffer = cfg.read(APP_NAME_SHORT, L"CheckUpdateAtStartup", 1);
			CheckMenuItem(GetMenu(hwndDlg), IDM_CFG_CHECKUPDATES, MF_BYCOMMAND | (iBuffer) ? MF_CHECKED : MF_UNCHECKED);

			if(iBuffer)
				_beginthread(CheckUpdates, 0, (LPVOID)1);

			// Insert From Clipboard
			iBuffer = cfg.read(APP_NAME_SHORT, L"InsertBufferAtStartup", 1);
			CheckMenuItem(GetMenu(hwndDlg), IDM_CFG_INSERTBUFFER, MF_BYCOMMAND | (iBuffer) ? MF_CHECKED : MF_UNCHECKED);

			if(iBuffer)
				InsertClipboardText(hwndDlg, IDC_ERROR_ID);

			break;
		}

		case WM_CLOSE:
		{
			// Save Settings
			cfg.write(APP_NAME_SHORT, L"DataType", SendDlgItemMessage(hwndDlg, IDC_ERROR_DATA, CB_GETCURSEL, 0, 0));
			cfg.write(APP_NAME_SHORT, L"ErrorMode", SendDlgItemMessage(hwndDlg, IDC_ERROR_MODE, CB_GETCURSEL, 0, 0));

			DestroyWindow(hwndDlg);
			PostQuitMessage(0);

			break;
		}

		case WM_SYSCOMMAND:
		{
			if(wParam == IDM_ABOUT)
				SendMessage(hwndDlg, WM_COMMAND, MAKELPARAM(IDM_ABOUT, 0), 0);

			break;
		}

		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case IDM_EXIT:
				case IDC_EXIT:
				case IDCANCEL:
					SendMessage(hwndDlg, WM_CLOSE, 0, 0);
					break;
					
				case IDM_WEBSITE:
					ShellExecute(hwndDlg, L"open", APP_WEBSITE, NULL, NULL, SW_SHOW);
					break;

				case IDM_CHECK_UPDATES:
					_beginthread(CheckUpdates, 0, 0);
					break;

				case IDM_ABOUT:
					CAboutBox about;
					about.Create(hwndDlg);

					break;
					
				case IDM_CFG_CHECKUPDATES:
					iBuffer = cfg.read(APP_NAME_SHORT, L"CheckUpdateAtStartup", 1);
					CheckMenuItem(GetMenu(hwndDlg), IDM_CFG_CHECKUPDATES, MF_BYCOMMAND | (iBuffer) ? MF_UNCHECKED : MF_CHECKED);
					cfg.write(APP_NAME_SHORT, L"CheckUpdateAtStartup", iBuffer ? 0 : 1);

					break;

				case IDM_CFG_INSERTBUFFER:
					iBuffer = cfg.read(APP_NAME_SHORT, L"InsertBufferAtStartup", 1);
					CheckMenuItem(GetMenu(hwndDlg), IDM_CFG_INSERTBUFFER, MF_BYCOMMAND | (iBuffer) ? MF_UNCHECKED : MF_CHECKED);
					cfg.write(APP_NAME_SHORT, L"InsertBufferAtStartup", iBuffer ? 0 : 1);

					break;

				case IDC_GET:
				{
					BOOL bResult = 0;
					EDITBALLOONTIP ebt = {0};
					ebt.cbStruct = sizeof(ebt);

					// DEC or HEX mode
					switch(SendDlgItemMessage(hwndDlg, IDC_ERROR_DATA, CB_GETCURSEL, 0, 0))
					{
						// Dec Mode
						case 0:
							iBuffer = GetDlgItemInt(hwndDlg, IDC_ERROR_ID, &bResult, 1);
							break;

						// Hex Mode
						case 1:
							bResult = GetDlgItemText(hwndDlg, IDC_ERROR_ID, szBuffer, MAX_PATH);
							iBuffer = _httoi(szBuffer);
							break;

						// Else
						default:
							return 0;
					}

					// Validate Input
					if(!bResult)
					{
						ebt.pszText = L"Необходимо ввести код ошибки";
						SendDlgItemMessage(hwndDlg, IDC_ERROR_ID, EM_SHOWBALLOONTIP, 0, (LPARAM)&ebt);
						SendMessage(hwndDlg, WM_COMMAND, MAKELPARAM(IDC_CLEAR, 0), 0);

						return 0;
					}

					int iErrorMode = SendDlgItemMessage(hwndDlg, IDC_ERROR_MODE, CB_GETCURSEL, 0, 0);

					if(iErrorMode == CB_ERR)
						return 0;

					// Get Error Text
					if(!ErrorOut(hwndDlg, iBuffer, iErrorMode))
					{
						ebt.pszText = L"Код ошибки не найден в базе";
						SendDlgItemMessage(hwndDlg, IDC_ERROR_ID, EM_SHOWBALLOONTIP, 0, (LPARAM)&ebt);
						SendMessage(hwndDlg, WM_COMMAND, MAKELPARAM(IDC_CLEAR, 0), 0);
					}

					break;
				}

				case IDC_CLEAR:
					SetDlgItemText(hwndDlg, IDC_ERROR_TEXT, 0);
					SetDlgItemText(hwndDlg, IDC_ERROR_ADVANCED, 0);

					break;
			}

			break;
		}
	}

	return 0;
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
	MSG msg = {0};
	INITCOMMONCONTROLSEX icex = {0};

	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES;

	if(!InitCommonControlsEx(&icex))
		return 0;

	if(!CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, (DLGPROC)DlgProc))
		return 0;

	while(GetMessage(&msg, NULL, 0, 0))
	{
		if(!IsDialogMessage(hMainDlg, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	
	return msg.wParam;
}