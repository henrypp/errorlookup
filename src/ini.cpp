/************************************
*	Ini Wrapper
*	Copyright © 2012 Henry++
*
*	GNU General Public License v2
*	http://www.gnu.org/licenses/
*
*	http://www.henrypp.org/
*************************************/

/*
	Example:
		Ini ini(L"C:\\test.cfg");
		ini.write(L"section", L"key", L"text");
		
		wcout << ini.read(L"section", L"key", MAX_PATH);
*/

#include "ini.h"

BOOL INI::make_write(wchar_t *szSection, wchar_t *szKey, wchar_t *szValue)
{
	return WritePrivateProfileString(szSection, szKey, szValue, szIni);
}
		
DWORD INI::make_read(wchar_t *szSection, wchar_t *szKey, wchar_t *sReturned, int iLenght, wchar_t *szDefault)
{
	return GetPrivateProfileString(szSection, szKey, szDefault, sReturned, iLenght, szIni);
}

// Load
void INI::load(wchar_t *szPath)
{
	wcsncpy(szIni, szPath, MAX_PATH);
}

// Write Int
void INI::write(wchar_t *szSection, wchar_t *szKey, int iValue)
{
	wsprintf(szBuffer, L"%i\0", iValue);
	make_write(szSection, szKey, szBuffer);
}

// Write String
void INI::write(wchar_t *szSection, wchar_t *szKey, wchar_t *szValue)
{
	make_write(szSection, szKey, szValue);
}

// Read Int
int INI::read(wchar_t *szSection, wchar_t *szKey, int iDefault)
{
	return GetPrivateProfileInt(szSection, szKey, iDefault, szIni);
}

// Read String
wchar_t* INI::read(wchar_t* szSection, wchar_t* szKey, int iLenght, wchar_t* szDefault)
{
	DWORD dwRet = make_read(szSection, szKey, szBuffer, iLenght, szDefault);

	return dwRet ? szBuffer : 0;
}