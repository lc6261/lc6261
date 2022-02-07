// inject_0.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdlib.h>
#include <windows.h>
#include <TlHelp32.h>
//转换字符串
#include <atlconv.h>
//查询进程是否存在
#include <Locale.h>

#include <iostream>
#include <string>
#include <map>

//获取PID
UINT GetProPid(LPCTSTR pszExeFile)
{
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
					break;
				}
			}
		}
		CloseHandle(hSnapshot);
	}
	return nProcessID;
}
void chartoLPCWSTR(char* szStr, WCHAR* wszClassName)
{
	int len = strlen(szStr) + 1;
	int len1 = wcslen(wszClassName) ;
	MultiByteToWideChar(CP_ACP, 0, szStr, len, wszClassName,len1);
 }
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
				wsprintf(strNameWchar,L"编号%d的进程名为：%s", nID, pe32.szExeFile);
				wprintf(L"%s\n", strNameWchar);
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


//调用方法 int yyim = GetModAdd(dwPid, "yyim.dll");</p><p>
int GetModAdd(DWORD PID, WCHAR* ModName)
{
	MODULEENTRY32 mo;
	HANDLE LM;
	LM = CreateToolhelp32Snapshot(0x8, PID);
	if (LM > 0)
	{
		mo.dwSize = sizeof(mo);
		if (Module32First(LM, &mo))
		{
			do
			{
				if (wcscmp(mo.szModule, ModName) == 0)
				{
					CloseHandle(LM);
					return (int)mo.modBaseAddr;
				}
			} while (Module32Next(LM, &mo) != 0);
		}
	}
	printf("%s:%d done\n", __FUNCTION__,__LINE__);
	return 0;
}

int injectdll(DWORD pID,char *path)
{
	//获取进程句柄
	HANDLE hprocess =  OpenProcess(PROCESS_ALL_ACCESS,FALSE,pID);
	if (hprocess == NULL)
	{
		//MessageBox(NULL, _T("err OpenProcess"), _T("dll"), MB_OK);
		printf("%s:%d:%d:err OpenProcess\n", __FUNCTION__, __LINE__, pID);
		return FALSE;
	}
	//申请一块内存空间给dll的路径
	LPVOID paddr = VirtualAllocEx(hprocess,NULL,strlen(path)+1,MEM_COMMIT,PAGE_READWRITE);
	if (paddr == NULL)
	{
		//MessageBox(NULL, _T("err VirtualAllocEx"), _T("dll"), MB_OK);

		printf("%s:%d:%d:err VirtualAllocEx\n", __FUNCTION__, __LINE__, pID);
		return FALSE;
	}
	//写入路径到申请的内存中
	WriteProcessMemory(hprocess,paddr, path, strlen(path) + 1,NULL);
	//获取loadlibrary函数的地址
	HMODULE hModule = LoadLibrary(L"kernel32.dll");
	if (hModule == NULL)
	{
		//MessageBox(NULL, _T("err LoadLibrary"), _T("dll"), MB_OK);
		printf("%s:%d:%d:err LoadLibrary\n", __FUNCTION__, __LINE__,pID);

		return FALSE;
	}

	LPTHREAD_START_ROUTINE lpStartaddr = (LPTHREAD_START_ROUTINE)GetProcAddress(hModule,"LoadLibraryA");
	if (lpStartaddr == NULL)
	{
		//MessageBox(NULL, _T("err lpStartaddr"), _T("dll"), MB_OK);
		printf("%s:%d:%d:err lpStartaddr\n", __FUNCTION__, __LINE__,pID);

		return FALSE;
	}

	HANDLE hThread = CreateRemoteThread(hprocess,NULL,0, lpStartaddr,paddr,0,NULL);
	if (hThread == NULL)
	{
		//MessageBox(NULL, _T("err CreateRemoteThread"), _T("dll"), MB_OK);
		printf("%s:%d:%d:err CreateRemoteThread\n", __FUNCTION__, __LINE__, pID);
		return FALSE;
	}
	//等待线程创建完成
	WaitForSingleObject(hThread,20000);

	CloseHandle(hThread);
	CloseHandle(hprocess);

	printf("%s %d:done\n", __FUNCTION__, __LINE__);
	return TRUE;
}
int unjectdll(DWORD pID, WCHAR *path)
{
	//获取进程句柄
	HANDLE hprocess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
	if (hprocess == NULL)
	{
		//MessageBox(NULL, _T("err OpenProcess"), _T("dll"), MB_OK);
		printf("%s:%d:%d:err OpenProcess\n", __FUNCTION__, __LINE__, pID);

		return FALSE;
	}
	//testdll_1.dll 的地址 
	int addr = GetModAdd(pID, L"testdll_1.dll");
	printf("testdll_1.dll addr=0x%x\n", addr);
	LPVOID paddr =(LPVOID)addr;

	//函数FreeLibrary 的地址
	addr = GetModAdd(pID, L"kernel32.dll");
	HMODULE hm =(HMODULE) addr;
	LPTHREAD_START_ROUTINE lpStartaddr =(LPTHREAD_START_ROUTINE) GetProcAddress(hm,"FreeLibrary");
	printf("FreeLibrary addr:0x%x\n", (unsigned int)lpStartaddr);

	HANDLE hThread = CreateRemoteThread(hprocess, NULL, 0, lpStartaddr, paddr, 0, NULL);
	if (hThread == NULL)
	{
		//MessageBox(NULL, _T("err CreateRemoteThread"), _T("dll"), MB_OK);
		printf("%s:%d:%d:err CreateRemoteThread\n", __FUNCTION__, __LINE__, pID);

		return FALSE;
	}
	//等待线程创建完成
	WaitForSingleObject(hThread, 20000);

	CloseHandle(hThread);
	CloseHandle(hprocess);

	printf("%s done\n",__FUNCTION__);
	return TRUE;
}

#if 0
bool TraverseProcesses(std::map<std::string, int>& _mapProcess)
{
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(pe32);

	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE) {
		std::cout << "Create Toolhelp32Snapshot Error!" << std::endl;
		return false;
	}

	BOOL bResult = Process32First(hProcessSnap, &pe32);//开始遍历

	int num(0);

	while (bResult)
	{
		std::string name = (std::string)pe32.szExeFile;
		int id = pe32.th32ProcessID;

		std::cout << "[" << ++num << "]: " << "--ProcessID:" << id;

		std::cout << "--Process Name:" << name << std::endl;

		_mapProcess.insert(std::pair<std::string, int>(name, id)); //字典存储
		bResult = Process32Next(hProcessSnap, &pe32);//返回值为false，表明进程遍历完
	}

	CloseHandle(hProcessSnap);
	return true;
}
#endif
std::string wchar_tToString(wchar_t* wchar) 
{
	
	std::string szDst;
	
	wchar_t* wText = wchar;
	
	DWORD dwNum = WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, NULL, 0, NULL, FALSE);
	
	char* psText;
	
	psText = new char[dwNum];
	
	WideCharToMultiByte(CP_OEMCP, NULL, wText, -1, psText, dwNum, NULL, FALSE);
	
	szDst = psText;
	
	delete[]psText;
	
	return szDst;
	
}

int showpid()
{
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(pe32);
	HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		std::cout << "CreateToolhelp32Snapshot 调用失败" << std::endl;
		return -1;
	}
	BOOL bMore = ::Process32First(hProcessSnap, &pe32);
	while (bMore)
	{
		std::cout << "进程ID:" << pe32.th32ProcessID << " 进程名称：" << wchar_tToString(pe32.szExeFile) << std::endl;
		bMore = ::Process32Next(hProcessSnap, &pe32);
	}
	::CloseHandle(hProcessSnap);
	std::cout << "请按任意键结束……" << std::endl;
	std::getchar();
	return 0;
}

int main()
{
	int ret = 0;
	showpid();
	UINT dwProcessID = GetProPid(_T("win32calc.exe"));
	ret = GetProcess(dwProcessID);
	if (ret != TRUE)
	{
		printf("GetProcess error\n");
	}

	ret = injectdll(dwProcessID, "D:\\BaiduNetdiskWorkspace\\GitHub\\lc6261\\dll\\testdll0\\dll_test_0\\x64\\Debug\\dll_test_0.dll");
	if (ret != TRUE)
	{
		printf("injectdll error\n");
	}

	ret = unjectdll(dwProcessID,L"dll_test_0.dll");
	if (ret != TRUE)
	{
		printf("unjectdll error\n");
	}

	//system("PAUSE");
    return 0;
}