//
//	MaskedEdit.c 
//	Copyright J Brown (c) 2001
//
//	Usage:
//	 To accepy only numeric characters and the space character:
//		MaskEditControl(hwndEdit, "0123456789 \b", TRUE)
//
//	

#include <windows.h>


//
//	Replacement window procedure for a masked edit control
//
LRESULT CALLBACK MaskedEditProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WNDPROC oldwndproc;
	BOOL *mask = (BOOL * )GetWindowLong(hwnd, GWL_USERDATA);
	oldwndproc = (WNDPROC)mask[256];

	switch(uMsg)
	{
	//WM_NCDESTROY is the LAST message that a window will receive - 
	//therefore we must finally remove the old wndproc here
	case WM_NCDESTROY:
		HeapFree(GetProcessHeap(), 0, mask);
		break;

	case WM_CHAR:
		if(HIWORD(GetKeyState(VK_LCONTROL)) || HIWORD(GetKeyState(VK_RCONTROL)))
			break;

		if(mask[wParam & 0xff])
			break;
		else
			return 0;
	}

	return CallWindowProc(oldwndproc, hwnd, uMsg, wParam, lParam);
}

//
//	Apply a mask to the specified edit control
//
BOOL MaskEditControl(HWND hwndEdit, char* szMask, DWORD nOptions)
{
	BOOL *mask;
	WNDPROC oldproc;
	int i;

	if(hwndEdit == 0)
		return FALSE;

	// don't make a new mask if there is already one available
	oldproc = (WNDPROC)GetWindowLong(hwndEdit, GWL_WNDPROC);
	
	if(oldproc != MaskedEditProc)
		mask = (BOOL *)HeapAlloc(GetProcessHeap(), 0, sizeof(WNDPROC) + 256 * sizeof(BOOL));
	else
		mask = (BOOL *)GetWindowLong(hwndEdit, GWL_USERDATA);

	// build the mask lookup table. The method varies depending
	// on whether we want to allow or disallow the specified szMask characters
	if(nOptions == TRUE)
	{
		for(i = 0; i < 256; i++)
			mask[i] = FALSE;

		for(i = 0; szMask[i] != '\0'; i++)
			mask[szMask[i]] = TRUE;
	}
	else
	{
		for(i = 0; i < 256; i++)
			mask[i] = TRUE;

		for(i = 0; szMask[i] != '\0'; i++)
			mask[szMask[i]] = FALSE;
	}

	//don't update the user data if it is already in place
	if(oldproc != MaskedEditProc)
	{
		mask[256] = (BOOL)GetWindowLong(hwndEdit, GWL_WNDPROC);
		SetWindowLong(hwndEdit, GWL_WNDPROC, (LONG)MaskedEditProc);
		SetWindowLong(hwndEdit, GWL_USERDATA, (LONG)mask);
	}

	return TRUE;
}