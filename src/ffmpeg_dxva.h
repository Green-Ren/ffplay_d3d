#pragma once
#include <windows.h>
#include "resource.h"

INT_PTR CALLBACK About(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

ATOM RegisterMyClass(HINSTANCE hInstance, LPCWSTR winClass);

HWND CreateMyWindow(TCHAR * pWinClass, TCHAR * pTitle, HINSTANCE hInstance, int cmdShow);