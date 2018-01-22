#include "stdafx.h"
#include "ConsoleAdapter.h"
#include "ffplay.h"
#include "ffmpeg_dxva.h"

//#define CONSOLE_DEBUG

HINSTANCE hInst;

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
					   _In_opt_ HINSTANCE hPrevInstance,
					   _In_ LPTSTR lpCmdLine,
					   _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

#ifdef CONSOLE_DEBUG
	//create console
	CConsoleAdapter console;
	console.CreateConsole();
#endif

	TCHAR szTitle[125] = {0};
	TCHAR szWinClass[125] = {0};

	LoadString(hInstance, IDS_APP_TITLE, szTitle, sizeof(szTitle));
	LoadString(hInstance, IDC_ffmpeg_dxva, szWinClass, sizeof(szWinClass));

	//register window class
	RegisterMyClass(hInstance, szWinClass);

	//create window
	HWND winId = CreateMyWindow(szWinClass, szTitle, hInstance, nCmdShow);
	if (!winId)
	{
		return -1;
	}

	//char *pFileName = "E:\\audio_video_nums\\1.flv";
	//char *pFileName = "E:\\audio_video_nums\\Gladiator.EXTENDED.CUT.2000.BD-RMVB--levn.rmvb";
	//char *pFileName = "E:\\audio_video_nums\\The Last Witch Hunter.mp4";

	//PlayVideoFile(pFileName, winId);

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ffmpeg_dxva));
	MSG msg = {0};
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

#ifdef CONSOLE_DEBUG
	//destroy console
	console.DestroyConsole();
#endif

	return (int) msg.wParam;
}

INT_PTR CALLBACK About(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (msg)
	{
		case WM_INITDIALOG:
		{
			return (INT_PTR)TRUE;
		}
		
		case WM_COMMAND:
		{
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hDlg, LOWORD(wParam));

				return (INT_PTR)TRUE;
			}
			break;
		}

		default:
		{
			break;
		}
	}

	return (INT_PTR)FALSE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
		{
			//HMENU hMenu = LoadMenu()
			break;
		}
		case WM_COMMAND:
		{
			WORD wmId    = LOWORD(wParam);
			WORD wmEvent = HIWORD(wParam);
			switch (wmId)
			{
				case IDM_ABOUT:
				{
					DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
					break;
				}
				case IDM_EXIT:
				{
					DestroyWindow(hWnd);
					break;
				}
				default:
				{
					return DefWindowProc(hWnd, message, wParam, lParam);
				}
			}

			break;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT pst = {0};
			BeginPaint(hWnd, &pst);
			EndPaint(hWnd, &pst);
			break;
		}
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			break;
		}
		default:
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}	
	}
	
	return 0;
}

ATOM RegisterMyClass(HINSTANCE hInstance, LPCWSTR winClass)
{
	hInst = hInstance;

	WNDCLASSEX winClassX = {0};
	winClassX.cbSize		= sizeof(WNDCLASSEX);
	winClassX.style			= CS_HREDRAW | CS_VREDRAW;
	winClassX.lpfnWndProc	= WndProc;
	winClassX.cbClsExtra	= 0;
	winClassX.cbWndExtra	= 0;
	winClassX.hInstance		= hInstance;
	winClassX.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ffmpeg_dxva));
	winClassX.hCursor		= LoadCursor(NULL, IDC_ARROW);
	winClassX.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
	winClassX.lpszMenuName	= MAKEINTRESOURCE(IDC_ffmpeg_dxva);
	winClassX.lpszClassName	= winClass;
	winClassX.hIconSm		= LoadIcon(winClassX.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&winClassX);
}

HWND CreateMyWindow(TCHAR * pWinClass, TCHAR * pTitle, HINSTANCE hInstance, int cmdShow)
{
	HWND winId = CreateWindow(pWinClass, pTitle, 
								WS_OVERLAPPEDWINDOW,
								CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, 
								NULL, NULL, 
								hInstance, 
								NULL);

	if (!winId)
	{
		return winId;
	}

	ShowWindow(winId, cmdShow);

	UpdateWindow(winId);

	return winId;
}