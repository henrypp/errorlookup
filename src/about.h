/************************************
*  	About Dialog Module
*	Copyright © 2012 Henry++
*
*	GNU General Public License v2
*	http://www.gnu.org/licenses/
*
*	http://www.henrypp.org/
*************************************/

#define ABOUT_CLASS L"AboutBox"

class CAboutBox
{
	protected:
		static LRESULT CALLBACK AboutBoxProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			switch (uMsg)
			{
				case WM_CREATE:
				{
					EnableWindow(GetParent(hwndDlg), 0);
					CenterDialog(hwndDlg);

					break;
				}

				case WM_LBUTTONDBLCLK:
				case WM_DESTROY:
				{
					SetActiveWindow(GetParent(hwndDlg));
					EnableWindow(GetParent(hwndDlg), 1);

					DestroyWindow(hwndDlg);
					PostQuitMessage(0);

					break;
				}

				case WM_PAINT:
				{
					PAINTSTRUCT ps = {0};
					RECT rc = {0};

					HDC hDC = BeginPaint(hwndDlg, &ps);

					GetClientRect(hwndDlg, &rc);
					rc.top = rc.bottom - 43;

					// Instead FillRect
					COLORREF clrOld = SetBkColor(hDC, GetSysColor(COLOR_BTNFACE));
					ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
					SetBkColor(hDC, clrOld);

					// Draw Line
					for(int i = 0; i < rc.right; i++)
						SetPixel(hDC, i, rc.top, GetSysColor(COLOR_BTNSHADOW));

					EndPaint(hwndDlg, &ps);

					return 0;
				}

				case WM_LBUTTONDOWN:
				{
					SendMessage(hwndDlg, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, 0);
					break;
				}

				case WM_CTLCOLORSTATIC:
				{
					return (LPARAM)CreateSolidBrush(GetSysColor(COLOR_WINDOW));
				}

				case WM_KEYDOWN:
				{
					if(wParam == VK_ESCAPE || wParam == VK_RETURN)
						SendMessage(hwndDlg, WM_CLOSE, 0, 0);

					break;
				}

				case WM_NOTIFY:
				{
					switch (((LPNMHDR)lParam)->code)
					{
						case NM_CLICK:
						case NM_RETURN:
						{
							NMLINK* nmlink = (NMLINK*)lParam;
							LITEM item = nmlink->item;
            
							if(item.szUrl)
								ShellExecute(hwndDlg, L"open", item.szUrl, 0, 0, SW_SHOW);
            
							break;
						}
					}

					break;
				}

				case WM_COMMAND:
				{
					if(LOWORD(wParam) == 100)
						SendMessage(hwndDlg, WM_CLOSE, 0, 0);

					break;
				}
			}

			return DefWindowProc(hwndDlg, uMsg, wParam, lParam);
		}

	public:
		int Create(HWND hParent)
		{
			MSG msg = {0};
			WNDCLASSEX wcex = {0};
			HINSTANCE hInstance = GetModuleHandle(0);

			if(!GetClassInfoEx(hInstance, ABOUT_CLASS, &wcex))
			{
				wcex.cbSize = sizeof(WNDCLASSEX);
				wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
				wcex.lpfnWndProc = (WNDPROC)AboutBoxProc;
				wcex.hInstance = hInstance;
				wcex.hCursor = LoadCursor(0, IDC_ARROW);
				wcex.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
				wcex.lpszClassName = ABOUT_CLASS;

				if(!RegisterClassEx(&wcex))
					return 0;
			}

			HWND hDlg = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST, ABOUT_CLASS, L"О программе", WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 353, 270, hParent, 0, hInstance, 0);

			if(!hDlg)
				return 0;

			HFONT hFont = CreateFont(-11, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, FF_DONTCARE, L"Segoe UI");
			HFONT hTitle = CreateFont(-12, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, FF_DONTCARE, L"Segoe UI");

			HWND hWnd = CreateWindowEx(0, WC_STATIC, 0, WS_VISIBLE | WS_CHILD | SS_ICON, 13, 13, 32, 32, hDlg, 0, hInstance, 0);
			SendMessage(hWnd, STM_SETICON, (WPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN)), 0);
			
			hWnd = CreateWindowEx(0, WC_STATIC, APP_NAME L" " APP_VERSION, WS_VISIBLE | WS_CHILD | WS_GROUP | SS_LEFT | SS_CENTERIMAGE, 56, 13, 280, 32, hDlg, 0, hInstance, 0);
			SendMessage(hWnd, WM_SETFONT, (WPARAM)hTitle, 0);

			hWnd = CreateWindowEx(0, WC_LINK, L"Copyright © 2012 Henry++\r\nВсе права защищены\r\n\r\nПрограмма распространяется под лицензией\r\nGNU General Public License v2.\r\n\r\nДанная программа является полностью бесплатной, спасибо за то что вы её выбрали.\r\n\r\n<a href=\"" APP_WEBSITE L"\">" APP_HOST L"</a>", WS_VISIBLE | WS_CHILD | WS_GROUP | SS_LEFT, 56, 50, 280, 140, hDlg, 0, hInstance, 0);
			SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, 0);

			RECT rc = {0};
			GetClientRect(hDlg, &rc);

			hWnd = CreateWindowEx(0, WC_BUTTON, L"OK", WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_DEFPUSHBUTTON, 270, rc.bottom - 33, 70, 23, hDlg, 0, hInstance, 0);
			SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, 0);
			SetWindowLong(hWnd, GWL_ID, 100);

			while(GetMessage(&msg, 0, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			DeleteObject(hFont);
			DeleteObject(hTitle);

			return msg.wParam;
		}
};