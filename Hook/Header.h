#pragma once

#ifdef HOOK_EXPORTS
#define HOOK_API __declspec(dllexport)
#else
#define HOOK_API __declspec(dllimport)
#endif

HOOK_API BOOL WINAPI SetHook(HWND hWndCaller, HWND hWndTarget, UINT uMsg);
HOOK_API BOOL WINAPI UnsetHook(HWND hWndCaller, HWND hWndTarget);
HOOK_API BOOL WINAPI QueryPasswordEdit();
