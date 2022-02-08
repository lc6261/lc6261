// win32_0.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <SDKDDKVer.h>
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <string>

#if 0
//加载静态库
//#pragma comment(lib,"testdll_1")
//extern "C" int testccc();
#else
//动态加载
#endif

typedef int(CALLBACK* LPFNREGISTER)(void);

#if 1
#include <TlHelp32.h>
#include <Locale.h>
//获取PID
UINT GetProPid(LPCTSTR pszExeFile)
{
	WCHAR strNameWchar[100];

	//查找当前的进程的pid
	UINT nProcessID = 0;
	PROCESSENTRY32 pe = { sizeof(PROCESSENTRY32) };
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
	if (hSnapshot != INVALID_HANDLE_VALUE)
	{
		if (Process32First(hSnapshot, &pe))
		{
			while (Process32Next(hSnapshot, &pe))
			{
				if (lstrcmpi(pszExeFile, pe.szExeFile) == 0)
				{
					nProcessID = pe.th32ProcessID;
					wprintf(L"name:%s,pid=%d\n", pszExeFile, pe.th32ProcessID);
					break;
				}
			}
		}
		CloseHandle(hSnapshot);
	}
	return nProcessID;
}
//获取进程名
int GetProcess(int nID)
{
	//显示中文
	_wsetlocale(LC_ALL, L"chs");

	WCHAR strNameWchar[100];
	HANDLE hProcessSnap = NULL;
	PROCESSENTRY32 pe32 = { 0 };
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == (HANDLE)-1)
	{
		//::MessageBox(NULL, L"查询进程失败！:(", L"错误提示", MB_OK);
		printf("%s:%d:%d:error CreateToolhelp32Snapshot\n", __FUNCTION__, __LINE__, nID);
		return FALSE;
	}
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hProcessSnap, &pe32))
	{
		do
		{
			if (nID == pe32.th32ProcessID) //判断制定进程号 
			{
				wprintf(L"pid:%d,name=%s\n", nID, pe32.szExeFile);
			}
		} while (Process32Next(hProcessSnap, &pe32));
	}
	else
	{
		//::MessageBox(NULL, L"出现意外错误！", L"错误提示", MB_OK);

		printf("%s:%d:%d:error Process32First\n", __FUNCTION__, __LINE__, nID);
		return FALSE;
	}
	CloseHandle(hProcessSnap);
	printf("%s:%d nID=%d,done\n", __FUNCTION__, __LINE__, nID);
	return TRUE;
}

BOOL showejectdll(DWORD dwPID, LPCTSTR szDllName)
{
	BOOL bMore = FALSE, bFound = FALSE;
	HANDLE hSnapshot, hProcess;
	MODULEENTRY32 me = { sizeof(me) };
	LPTHREAD_START_ROUTINE pThreadProc;
	if (INVALID_HANDLE_VALUE == (hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID)))
	{
		return FALSE;
	}


	bMore = Module32First(hSnapshot, &me);
	for (; bMore; bMore = Module32Next(hSnapshot, &me))
	{
		wprintf(L"%s:%d:%s\n", _T(__FUNCTION__), __LINE__, me.szModule);
		if (!_tcsicmp(me.szModule, szDllName) || !_tcsicmp(me.szExePath, szDllName))
		{
			bFound = TRUE;
			break;
		}

	}

	if (!bFound)
	{
		CloseHandle(hSnapshot);
		wprintf(L"%s:%d:not found:%s\n", _T(__FUNCTION__), __LINE__, szDllName);
		return FALSE;
	}

	if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID)))
	{
		CloseHandle(hSnapshot);
		return FALSE;
	}
	CloseHandle(hSnapshot);

	return bFound;
}
#endif
int main()
{
	//将制定的模块加载到调用进程的地址空间
	//HMODULE hModule = LoadLibrary(L"D:\\BaiduNetdiskWorkspace\\GitHub\\lc6261\\dll\\dll_test_0\\x64\\Debug\\dll_test_0.dll");
	HMODULE hModule = LoadLibrary(L"D:\\BaiduNetdiskWorkspace\\GitHub\\lc6261\\dll\\dll_test_0\\Debug\\dll_test_0.dll");
	if (hModule == NULL)
	{
		DWORD dwErr = GetLastError();

		printf("%s:%d:0x%x:LoadLibrary err=%d\n", __FUNCTION__, __LINE__, hModule, dwErr);
	}

	//在制定的动态链接库中找函数地址
	typedef int(CALLBACK* LPFNREGISTER)(void);
	LPFNREGISTER pfunc = (LPFNREGISTER)GetProcAddress(hModule, "testccc");
	if (pfunc)
	{
		(*pfunc)();
		printf("%s:%d:0x%x:lpfnRegister done\n", __FUNCTION__, __LINE__, pfunc);

	}
	else
	{
		printf("%s:%d:0x%x:lpfnRegister err\n", __FUNCTION__, __LINE__, pfunc);
	}
	//std::getchar();
	FreeLibrary(hModule);

#if 1
	int ret = 0;
	UINT dwProcessID = 0;
	dwProcessID = GetProPid(L"test_mfc1.exe");
	if(dwProcessID !=0)
	{
		ret = GetProcess(dwProcessID);
	}

	//showejectdll(dwProcessID, L"D:\\BaiduNetdiskWorkspace\\GitHub\\lc6261\\dll\\testdll0\\dll_test_0\\x64\\Debug\\dll_test_0.dll");
	showejectdll(dwProcessID, L"D:\\BaiduNetdiskWorkspace\\GitHub\\lc6261\\dll\\dll_test_0\\Debug\\dll_test_0.dll");

#endif
	//free
	std::getchar();
    return 0;
}

