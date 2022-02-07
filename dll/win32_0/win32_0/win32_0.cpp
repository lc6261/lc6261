// win32_0.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <SDKDDKVer.h>
#include <stdio.h>
#include <tchar.h>
#include <windows.h>

#include <string>
#if 0
//���ؾ�̬��
//#pragma comment(lib,"testdll_1")
//extern "C" int testccc();
#else
//��̬����
#endif

typedef int(CALLBACK* LPFNREGISTER)(void);

int main()
{
	//���ƶ���ģ����ص����ý��̵ĵ�ַ�ռ�

	HMODULE hModule = LoadLibrary(_T("D:\\BaiduNetdiskWorkspace\\GitHub\\lc6261\\dll\\testdll0\\dll_test_0\\x64\\Debug\\dll_test_0.dll"));
	if (hModule == NULL)
	{
		MessageBox(NULL, _T("err hModule"), _T("testdll_1"), MB_OK);
		printf("%s:%d:0x%x:err LoadLibrary\n", __FUNCTION__, __LINE__, hModule);
	}
	//���ƶ��Ķ�̬���ӿ����Һ�����ַ

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
	std::getchar();

	//free
	FreeLibrary(hModule);
	std::getchar();
    return 0;
}

