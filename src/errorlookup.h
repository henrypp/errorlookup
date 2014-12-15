/************************************
*  	Error Lookup
*	Copyright © 2012 Henry++
*
*	GNU General Public License v2
*	http://www.gnu.org/licenses/
*
*	http://www.henrypp.org/
*************************************/

#ifndef __ERRORLOOKUP_H__
#define __ERRORLOOKUP_H__

// Define
#define APP_NAME L"Error Lookup"
#define APP_NAME_SHORT L"errorlookup"
#define APP_VERSION L"1.6"
#define APP_VERSION_RES 1,6
#define APP_HOST L"www.henrypp.org"
#define APP_WEBSITE L"http://" APP_HOST

// Variables
HWND hMainDlg = NULL;

// Prototypes
LRESULT CALLBACK MaskedEditProc(HWND, UINT, WPARAM, LPARAM);
BOOL MaskEditControl(HWND, char*, DWORD);

// Error Table Struct
struct ERROR_TABLE
{
	int code;
	wchar_t* description;
	wchar_t* name;
};

// Severity Status Codes
#define STATUS_SEVERITY_SUCCESS			0x0
#define STATUS_SEVERITY_INFORMATIONAL	0x1
#define STATUS_SEVERITY_WARNING			0x2
#define STATUS_SEVERITY_ERROR			0x3

// Facility Codes (Undefined)
#define FACILITY_D3D					0x876
#define FACILITY_D3DX					0x877
#define FACILITY_DSOUND_DMUSIC			0x878
#define FACILITY_D3D10					0x879
#define FACILITY_DXGI					0x87a
#define FACILITY_XAUDIO2				0x896
#define FACILITY_XAPO					0x897
#define FACILITY_XACTENGINE				0xac7
#define FACILITY_D2D					0x899
#define FACILITY_DWRITE					0x898
#define FACILITY_D3D11_OR_AE			0x87c
#define FACILITY_APO					0x87d
#define FACILITY_LEAP					0x888
#define FACILITY_WSAPI					0x889

// Facility Switch Routine
#define FACILITY(x)  case x: wcsncpy_s(szFacility, L#x, 100); break;

// Centering Window by Parent
void CenterDialog(HWND hwndWindow)
{
     HWND hParent = GetParent(hwndWindow);
	 RECT rcChild = {0}, rcParent = {0};

	 // If Parent Doesn't Exists - Use Desktop
	 if(!hParent)
		 hParent = GetDesktopWindow();

    GetWindowRect(hwndWindow, &rcChild);
    GetWindowRect(hParent, &rcParent);
 
    int nWidth = rcChild.right - rcChild.left;
    int nHeight = rcChild.bottom - rcChild.top;
 
    int nX = ((rcParent.right - rcParent.left) - nWidth) / 2 + rcParent.left;
    int nY = ((rcParent.bottom - rcParent.top) - nHeight) / 2 + rcParent.top;
 
    int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);

    if (nX < 0) nX = 0;
    if (nY < 0) nY = 0;
    if (nX + nWidth > nScreenWidth) nX = nScreenWidth - nWidth;
    if (nY + nHeight > nScreenHeight) nY = nScreenHeight - nHeight;
 
    MoveWindow(hwndWindow, nX, nY, nWidth, nHeight, FALSE);
}

#endif
