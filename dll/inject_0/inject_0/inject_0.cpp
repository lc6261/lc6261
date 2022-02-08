// inject_0.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <stdlib.h>
#include <windows.h>
#include <TlHelp32.h>
//ת���ַ���
#include <atlconv.h>
//��ѯ�����Ƿ����
#include <Locale.h>

#include <iostream>
#include <string>
#include <map>

//��ȡPID
UINT GetProPid(LPCTSTR pszExeFile)
{
	//���ҵ�ǰ�Ľ��̵�pid
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
	int len1 = wcslen(wszClassName);
	MultiByteToWideChar(CP_ACP, 0, szStr, len, wszClassName, len1);
}
int GetProcess(int nID)
{
	//��ʾ����
	_wsetlocale(LC_ALL, L"chs");

	WCHAR strNameWchar[100];
	HANDLE hProcessSnap = NULL;
	PROCESSENTRY32 pe32 = { 0 };
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == (HANDLE)-1)
	{
		//::MessageBox(NULL, L"��ѯ����ʧ�ܣ�:(", L"������ʾ", MB_OK);
		printf("%s:%d:%d:error CreateToolhelp32Snapshot\n", __FUNCTION__, __LINE__, nID);

		return FALSE;
	}
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hProcessSnap, &pe32))
	{
		do
		{
			if (nID == pe32.th32ProcessID) //�ж��ƶ����̺� 
			{
				wsprintf(strNameWchar, L"���%d�Ľ�����Ϊ��%s", nID, pe32.szExeFile);
				wprintf(L"%s\n", strNameWchar);
			}

		} while (Process32Next(hProcessSnap, &pe32));
	}
	else
	{
		//::MessageBox(NULL, L"�����������", L"������ʾ", MB_OK);

		printf("%s:%d:%d:error Process32First\n", __FUNCTION__, __LINE__, nID);
		return FALSE;
	}
	CloseHandle(hProcessSnap);
	printf("%s:%d nID=%d,done\n", __FUNCTION__, __LINE__, nID);
	return TRUE;
}


//���÷��� int yyim = GetModAdd(dwPid, "yyim.dll");</p><p>
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
	printf("%s:%d done\n", __FUNCTION__, __LINE__);
	return 0;
}
#if 0
int InjectDll(DWORD pID, char* path)
{
	//��ȡ���̾��
	HANDLE hprocess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
	if (hprocess == NULL)
	{
		//MessageBox(NULL, _T("err OpenProcess"), _T("dll"), MB_OK);
		printf("%s:%d:%d:err OpenProcess\n", __FUNCTION__, __LINE__, pID);
		return FALSE;
	}
	//����һ���ڴ�ռ��dll��·��
	LPVOID paddr = VirtualAllocEx(hprocess, NULL, strlen(path) + 1, MEM_COMMIT, PAGE_READWRITE);
	if (paddr == NULL)
	{
		//MessageBox(NULL, _T("err VirtualAllocEx"), _T("dll"), MB_OK);

		printf("%s:%d:%d:err VirtualAllocEx\n", __FUNCTION__, __LINE__, pID);
		return FALSE;
	}
	//д��·����������ڴ���
	WriteProcessMemory(hprocess, paddr, path, strlen(path) + 1, NULL);
	//��ȡloadlibrary�����ĵ�ַ
	HMODULE hModule = LoadLibrary(L"kernel32.dll");
	if (hModule == NULL)
	{
		//MessageBox(NULL, _T("err LoadLibrary"), _T("dll"), MB_OK);
		printf("%s:%d:%d:err LoadLibrary\n", __FUNCTION__, __LINE__, pID);

		return FALSE;
	}

	LPTHREAD_START_ROUTINE lpStartaddr = (LPTHREAD_START_ROUTINE)GetProcAddress(hModule, "LoadLibraryA");
	if (lpStartaddr == NULL)
	{
		//MessageBox(NULL, _T("err lpStartaddr"), _T("dll"), MB_OK);
		printf("%s:%d:%d:err lpStartaddr\n", __FUNCTION__, __LINE__, pID);

		return FALSE;
	}

	HANDLE hThread = CreateRemoteThread(hprocess, NULL, 0, lpStartaddr, paddr, 0, NULL);
	if (hThread == NULL)
	{
		//MessageBox(NULL, _T("err CreateRemoteThread"), _T("dll"), MB_OK);
		printf("%s:%d:%d:err CreateRemoteThread\n", __FUNCTION__, __LINE__, pID);
		return FALSE;
	}
	//�ȴ��̴߳������
	WaitForSingleObject(hThread, 20000);

	CloseHandle(hThread);
	CloseHandle(hprocess);

	printf("%s %d:done\n", __FUNCTION__, __LINE__);
	return TRUE;
}
int EjectDll(DWORD pID, WCHAR* path)
{
	//��ȡ���̾��
	HANDLE hprocess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
	if (hprocess == NULL)
	{
		//MessageBox(NULL, _T("err OpenProcess"), _T("dll"), MB_OK);
		printf("%s:%d:%d:err OpenProcess\n", __FUNCTION__, __LINE__, pID);

		return FALSE;
	}
	//testdll_1.dll �ĵ�ַ 
	int addr = GetModAdd(pID, L"testdll_1.dll");
	printf("testdll_1.dll addr=0x%x\n", addr);
	LPVOID paddr = (LPVOID)addr;

	//����FreeLibrary �ĵ�ַ
	addr = GetModAdd(pID, L"kernel32.dll");
	HMODULE hm = (HMODULE)addr;
	LPTHREAD_START_ROUTINE lpStartaddr = (LPTHREAD_START_ROUTINE)GetProcAddress(hm, "FreeLibrary");
	printf("FreeLibrary addr:0x%x\n", (unsigned int)lpStartaddr);

	HANDLE hThread = CreateRemoteThread(hprocess, NULL, 0, lpStartaddr, paddr, 0, NULL);
	if (hThread == NULL)
	{
		//MessageBox(NULL, _T("err CreateRemoteThread"), _T("dll"), MB_OK);
		printf("%s:%d:%d:err CreateRemoteThread\n", __FUNCTION__, __LINE__, pID);

		return FALSE;
	}
	//�ȴ��̴߳������
	WaitForSingleObject(hThread, 20000);

	CloseHandle(hThread);
	CloseHandle(hprocess);

	printf("%s done\n", __FUNCTION__);
	return TRUE;
}
#else

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
		//wprintf(L"%s:%d:%s\n", _T(__FUNCTION__), __LINE__, me.szModule);
		if (!_tcsicmp(me.szModule, szDllName) || !_tcsicmp(me.szExePath, szDllName))
		{
			bFound = TRUE;
			break;
		}

	}

	if (!bFound)
	{
		CloseHandle(hSnapshot);
		wprintf(L"%s:%d:not found\n", _T(__FUNCTION__), __LINE__);
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
BOOL InjectDll(DWORD dwPID, LPCTSTR szDllName)
{
	HANDLE hProcess, hThread;
	LPVOID pRemoteBufferData;  //Զ�̽����е��ڴ�
	DWORD dwBufSize = (DWORD)(_tcslen(szDllName) + 1) * sizeof(TCHAR);
	LPTHREAD_START_ROUTINE pThreadProc;

	if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID)))
	{
		DWORD dwErr = GetLastError();
		printf("%s:0x%x:%d:err OpenProcess\n", __FUNCTION__, __LINE__, dwErr);
		return FALSE;
	}

	//�����ڴ� �ɶ���д
	pRemoteBufferData = VirtualAllocEx(hProcess, NULL, dwBufSize, MEM_COMMIT, PAGE_READWRITE);
	if (pRemoteBufferData == NULL)
	{
		DWORD dwErr = GetLastError();
		printf("%s:%d:%d:err VirtualAllocEx\n", __FUNCTION__, __LINE__, dwErr);
		return FALSE;
	}
	//���ڴ�д��dll·��
	WriteProcessMemory(hProcess, pRemoteBufferData, (LPVOID)szDllName, dwBufSize, NULL);

	//���loadlibraryw��ַ
#ifdef _UNICODE    
	pThreadProc = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(_T("Kernel32")), "LoadLibraryW");
#else
	HMODULE kernel = ::GetModuleHandle("Kernel32");
	pThreadProc = (PTHREAD_START_ROUTINE)::GetProcAddress(kernel, "LoadLibraryA");
#endif
	if (pThreadProc == NULL)
	{
		DWORD dwErr = GetLastError();
		printf("%s:%d:%d:err GetProcAddress\n", __FUNCTION__, __LINE__, dwErr);

		return FALSE;
	}
	//����Զ���߳�
	hThread = CreateRemoteThread(hProcess, NULL, 0, pThreadProc, pRemoteBufferData, 0, NULL);
	if (hThread == NULL)
	{
		DWORD dwErr = GetLastError();
		printf("%s:%d:%d:err CreateRemoteThread\n", __FUNCTION__, __LINE__, dwErr);
		return FALSE;
	}

	//�ȴ��̴߳������
	WaitForSingleObject(hThread, INFINITE);

	showejectdll(dwPID, szDllName);

	CloseHandle(hThread);
	CloseHandle(hProcess);
	wprintf(L"%s:%d:%s done\n", _T(__FUNCTION__), __LINE__, szDllName);

	return TRUE;
}
BOOL EjectDll(DWORD dwPID, LPCTSTR szDllName)
{
	BOOL bMore = FALSE, bFound = FALSE;
	HANDLE hSnapshot, hProcess, hThread;
	MODULEENTRY32 me = { sizeof(me) };
	LPTHREAD_START_ROUTINE pThreadProc;

	if (INVALID_HANDLE_VALUE == (hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID)))
	{
		return FALSE;
	}


	bMore = Module32First(hSnapshot, &me);
	for (; bMore; bMore = Module32Next(hSnapshot, &me))
	{
		//wprintf(L"%s:%d:%s\n", _T(__FUNCTION__), __LINE__, me.szModule);
		if (!_tcsicmp(me.szModule, szDllName) || !_tcsicmp(me.szExePath, szDllName))
		{
			bFound = TRUE;
			break;
		}
	}

	if (!bFound)
	{
		CloseHandle(hSnapshot);
		wprintf(L"%s:%d:not found\n", _T(__FUNCTION__), __LINE__);
		return FALSE;
	}

	if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID)))
	{
		CloseHandle(hSnapshot);
		return FALSE;
	}
	CloseHandle(hSnapshot);


	pThreadProc = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "FreeLibrary");
	if (pThreadProc == NULL)
	{
		DWORD dwErr = GetLastError();
		wprintf(L"%s:%d:%d:err GetProcAddress\n", _T(__FUNCTION__), __LINE__, dwErr);

		return FALSE;
	}
	hThread = CreateRemoteThread(hProcess, NULL, 0, pThreadProc, me.modBaseAddr, 0, NULL);
	if (hThread == NULL)
	{
		DWORD dwErr = GetLastError();
		wprintf(L"%s:%d:%d:err CreateRemoteThread\n", _T(__FUNCTION__), __LINE__, dwErr);
		return FALSE;
	}
	WaitForSingleObject(hThread, INFINITE);

	CloseHandle(hThread);
	CloseHandle(hProcess);
	wprintf(L"%s:%d:%s done\n", _T(__FUNCTION__), __LINE__, szDllName);

	return TRUE;
}

#endif
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

	BOOL bResult = Process32First(hProcessSnap, &pe32);//��ʼ����

	int num(0);

	while (bResult)
	{
		std::string name = (std::string)pe32.szExeFile;
		int id = pe32.th32ProcessID;

		std::cout << "[" << ++num << "]: " << "--ProcessID:" << id;

		std::cout << "--Process Name:" << name << std::endl;

		_mapProcess.insert(std::pair<std::string, int>(name, id)); //�ֵ�洢
		bResult = Process32Next(hProcessSnap, &pe32);//����ֵΪfalse���������̱�����
	}

	CloseHandle(hProcessSnap);
	return true;
}
#endif
#if 1
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
		std::cout << "CreateToolhelp32Snapshot ����ʧ��" << std::endl;
		return -1;
	}
	BOOL bMore = ::Process32First(hProcessSnap, &pe32);
	while (bMore)
	{
		std::cout << "����ID:" << pe32.th32ProcessID << " �������ƣ�" << wchar_tToString(pe32.szExeFile) << std::endl;
		bMore = ::Process32Next(hProcessSnap, &pe32);
	}
	::CloseHandle(hProcessSnap);
	std::cout << "�밴�������������" << std::endl;
	//std::getchar();
	return 0;
}
#endif

DWORD _EnableNTPrivilege(LPCTSTR szPrivilege, DWORD dwState)
{
	DWORD dwRtn = 0;
	HANDLE hToken;
	if (OpenProcessToken(GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		LUID luid;
		if (LookupPrivilegeValue(NULL, szPrivilege, &luid))
		{
			BYTE t1[sizeof(TOKEN_PRIVILEGES) + sizeof(LUID_AND_ATTRIBUTES)];
			BYTE t2[sizeof(TOKEN_PRIVILEGES) + sizeof(LUID_AND_ATTRIBUTES)];
			DWORD cbTP = sizeof(TOKEN_PRIVILEGES) + sizeof(LUID_AND_ATTRIBUTES);

			PTOKEN_PRIVILEGES pTP = (PTOKEN_PRIVILEGES)t1;
			PTOKEN_PRIVILEGES pPrevTP = (PTOKEN_PRIVILEGES)t2;

			pTP->PrivilegeCount = 1;
			pTP->Privileges[0].Luid = luid;
			pTP->Privileges[0].Attributes = dwState;

			if (AdjustTokenPrivileges(hToken, FALSE, pTP, cbTP, pPrevTP, &cbTP))
				dwRtn = pPrevTP->Privileges[0].Attributes;
		}

		CloseHandle(hToken);
	}

	return dwRtn;
}
#define TEST_0  0
int _tmain(int argc, TCHAR* argv[])
{
	int ret = 0;
	UINT dwProcessID = 0;
	TCHAR* test = NULL;
	showpid();
	//����Ȩ��
	_EnableNTPrivilege(SE_DEBUG_NAME, SE_PRIVILEGE_ENABLED);

#ifdef TEST_0
	//dwProcessID = GetProPid(_T("win32calc.exe"));
	dwProcessID = GetProPid(_T("test_mfc1.exe"));
	ret = GetProcess(dwProcessID);
	if (ret != TRUE)
	{
		printf("GetProcess error\n");
	}

	//test = L"D:\\BaiduNetdiskWorkspace\\GitHub\\lc6261\\dll\\testdll0\\dll_test_0\\x64\\Debug\\dll_test_0.dll";
	test = L"D:\\BaiduNetdiskWorkspace\\GitHub\\lc6261\\dll\\dll_test_0\\Debug\\dll_test_0.dll";

#else
	dwProcessID = (DWORD)_tstoi(argv[2]);
	test = argv[3];
#endif


#ifdef TEST_0
	InjectDll(dwProcessID, test);
	getchar();
	EjectDll(dwProcessID, test);
#else
	// InjectDll.exe <i|e> <PID> <dll_path>  
	if (!_tcsicmp(argv[1], L"i"))
	{
		InjectDll((DWORD)_tstoi(argv[2]), argv[3]);
	}
	else if (!_tcsicmp(argv[1], L"e"))
	{
		EjectDll((DWORD)_tstoi(argv[2]), argv[3]);
	}

#endif
	//system("PAUSE");
	return 0;
}