#include <windows.h>
#include "Header.h"

#pragma data_seg("Shared")
HHOOK g_hHook = NULL;
HWND  g_hCaller = NULL;
HWND  g_hTarget = NULL;
UINT  g_uMessage = 0;
#pragma data_seg()

#pragma comment(linker, "/section:Shared,rws")

HINSTANCE g_hinstDll = NULL;

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		g_hinstDll = (HINSTANCE)hModule;
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

BOOL WINAPI UnsetHook(HWND hWndCaller, HWND hWndTarget)
{
	if (g_hHook && UnhookWindowsHookEx(g_hHook))
	{
		g_hHook = NULL;
		return TRUE;
	}
	return FALSE;
}

LRESULT CALLBACK CallWndProcHook(int nCode,WPARAM wParam,LPARAM lParam)
{
	CWPSTRUCT* pCwp = (CWPSTRUCT*)lParam;
	if (pCwp && pCwp->message == g_uMessage && pCwp->hwnd == g_hTarget)
	{
		if (pCwp->wParam == 0)
		{
			TCHAR szBuffer[1024] = { 0 };
			SendMessage(g_hTarget, WM_GETTEXT, _countof(szBuffer), (LPARAM)szBuffer);
			COPYDATASTRUCT cds = { 0 };
			cds.dwData = (DWORD_PTR)g_hTarget;
			cds.cbData = (lstrlen(szBuffer) + 1) * sizeof(TCHAR);
			cds.lpData = szBuffer;
			SendMessage(g_hCaller, WM_COPYDATA, (WPARAM)g_hTarget, (LPARAM)&cds);
		}
	}
	return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

BOOL WINAPI SetHook(HWND hWndCaller, HWND hWndTarget, UINT uMsg)
{
	if (g_hHook)
	{
		UnsetHook(g_hCaller, g_hTarget);
	}
	g_hHook = SetWindowsHookEx(WH_CALLWNDPROC, (HOOKPROC)CallWndProcHook, g_hinstDll, GetWindowThreadProcessId(hWndTarget, NULL));
	if (g_hHook == NULL)
	{
		return FALSE;
	}
	g_hCaller = hWndCaller;
	g_hTarget = hWndTarget;
	g_uMessage = uMsg;
	return TRUE;
}

BOOL WINAPI QueryPasswordEdit()
{
	if (g_hHook == NULL || g_hCaller == NULL || g_hTarget == NULL || g_uMessage == 0)
		return FALSE;
	SendMessage(g_hTarget, g_uMessage, 0, 0);
	return TRUE;
}
