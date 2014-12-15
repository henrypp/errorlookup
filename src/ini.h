/************************************
*	Ini Wrapper
*	Copyright © 2012 Henry++
*
*	GNU General Public License v2
*	http://www.gnu.org/licenses/
*
*	http://www.henrypp.org/
*************************************/

#ifndef __INI_H__
#define __INI_H__

#include <windows.h>

/*
	Example:
		Ini ini(L"C:\\test.cfg");
		ini.write(L"section", L"key", L"text");
		
		wcout << ini.read(L"section", L"key", MAX_PATH);
*/

class INI
{
	protected:
		wchar_t szIni[MAX_PATH], szBuffer[MAX_PATH];
				
		BOOL make_write(wchar_t *szSection, wchar_t *szKey, wchar_t *szValue);
		DWORD make_read(wchar_t *szSection, wchar_t *szKey, wchar_t *sReturned, int iLenght, wchar_t *szDefault);

	public:
		// Load
		void load(wchar_t *szPath);

		// Write Int
		void write(wchar_t *szSection, wchar_t *szKey, int iValue);

		// Write String
		void write(wchar_t *szSection, wchar_t *szKey, wchar_t *szValue);

		// Read Int
		int read(wchar_t *szSection, wchar_t *szKey, int iDefault);

		// Read String
		wchar_t* read(wchar_t *szSection, wchar_t *szKey, int iLenght, wchar_t *szDefault);
};

#endif // __INI_H__