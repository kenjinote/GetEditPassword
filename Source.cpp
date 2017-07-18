#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#ifdef _WIN64
#ifdef _DEBUG
#pragma comment(lib, "X64\\DEBUG\\Hook")
#else
#pragma comment(lib, "X64\\RELESER\\Hook")
#endif
#else
#ifdef _DEBUG
#pragma comment(lib, "DEBUG\\Hook")
#else
#pragma comment(lib, "RELESER\\Hook")
#endif
#endif

#include <windows.h>
#include "Hook\Header.h"
#include "resource.h"

BOOL IsPasswordEdit(HWND hWnd)
{
	TCHAR szClassName[1024];
	int nRet = GetClassName(hWnd, szClassName, _countof(szClassName));
	if (nRet == 0) return FALSE;
	szClassName[nRet] = 0;
	if (lstrcmpi(szClassName, TEXT("Edit")) != 0) return FALSE;
	DWORD dw = GetWindowLong(hWnd, GWL_STYLE);
	dw &= ES_PASSWORD;
	if (dw == ES_PASSWORD)
		return TRUE;
	return FALSE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hEdit;
	static BOOL bCapture = FALSE;
	static HBITMAP hBitmap1, hBitmap2;
	static HCURSOR hCursor;
	static UINT uMessage;

	switch (msg) {
	case WM_CREATE:
		uMessage = RegisterWindowMessage(TEXT("GetEditPassword"));
		hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), 0, WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_READONLY, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)(lParam))->hInstance, NULL);
		CreateWindow(TEXT("STATIC"), 0, WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_NOTIFY, 10, 10, 42, 42, hWnd, (HMENU)1, ((LPCREATESTRUCT)(lParam))->hInstance, NULL);
		hBitmap1 = LoadBitmap(((LPCREATESTRUCT)(lParam))->hInstance, MAKEINTRESOURCE(IDB_BITMAP1));
		hBitmap2 = LoadBitmap(((LPCREATESTRUCT)(lParam))->hInstance, MAKEINTRESOURCE(IDB_BITMAP2));
		hCursor = LoadCursor(((LPCREATESTRUCT)(lParam))->hInstance, MAKEINTRESOURCE(IDC_CURSOR1));
		SendDlgItemMessage(hWnd, 1, (UINT)STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmap1);
		break;
	case WM_SIZE:
		MoveWindow(hEdit, 10, 50, LOWORD(lParam) - 20, 32, 1);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == 1) {
			SendDlgItemMessage(hWnd, 1, (UINT)STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmap2);
			bCapture = TRUE;
			SetCursor(hCursor);
			SetCapture(hWnd);
		}
		break;
	case WM_COPYDATA:
		{
			PCOPYDATASTRUCT pCopyDataStruct = (PCOPYDATASTRUCT)lParam;
			if (pCopyDataStruct && IsPasswordEdit((HWND)pCopyDataStruct->dwData))
			{
				TCHAR szBuffer[1024] = { 0 };
				DWORD dwSize = sizeof(szBuffer) * sizeof(TCHAR);
				if (pCopyDataStruct->cbData < dwSize)
				{
					dwSize = pCopyDataStruct->cbData;
				}
				CopyMemory(szBuffer, pCopyDataStruct->lpData, dwSize);
				SetWindowText(hEdit, szBuffer);
				SendMessage(hEdit, EM_SETSEL, 0, -1);
				SetFocus(hEdit);
			}
		}
		break;
	case WM_LBUTTONUP:
		if (bCapture)
		{
			ReleaseCapture();
			SetWindowText(hEdit, 0);
			SetCursor(LoadCursor(NULL, IDC_ARROW));
			bCapture = FALSE;
			SendDlgItemMessage(hWnd, 1, (UINT)STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmap1);
			POINT point;
			GetCursorPos(&point);
			HWND hTargetWnd = WindowFromPoint(point);
			if (hTargetWnd && IsPasswordEdit(hTargetWnd))
			{
				SetHook(hWnd, hTargetWnd, uMessage);
				QueryPasswordEdit();
				UnsetHook(hWnd, hTargetWnd);
			}
		}
		break;
	case WM_DESTROY:
		DeleteObject(hBitmap1);
		DeleteObject(hBitmap2);
		PostQuitMessage(0);
		break;
	case WM_NCLBUTTONDOWN:
	case WM_NCRBUTTONDOWN:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		SetForegroundWindow(hWnd);
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	TCHAR szClassName[] = TEXT("Window");
	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("パスワードエディットボックスからパスワードを取得する"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
