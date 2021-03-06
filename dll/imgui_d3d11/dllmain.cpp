#include "stdafx.h"
#include "d3d11hook.h"

const LPCWSTR AppWindowTitle = L"EmptyProject11"; // Targeted D11 Application Window Title.

DWORD WINAPI MainThread(HMODULE hModule)
{
	ImplHookDX11_Init(hModule, FindWindow(0, AppWindowTitle));

	return S_OK;
}


BOOL APIENTRY DllMain(HMODULE hModule,// 指向自身的句柄
	DWORD  ul_reason_for_call,// 调用原因
	LPVOID lpReserved// 隐式加载和显式加载
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH://进程映射：进程将DLL映射到地址空间，只有第一次调用时使用；
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)MainThread, hModule, NULL, NULL);
	case DLL_THREAD_ATTACH://进程卸载

	case DLL_THREAD_DETACH://线程映射：进程创建线程时或者线程创建线程时，每次创建线程都会使用；
	case DLL_PROCESS_DETACH://线程卸载
		break;
	}
	return TRUE;
}

