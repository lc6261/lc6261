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

int main()
{
	//将制定的模块加载到调用进程的地址空间

	HMODULE hModule = LoadLibrary(_T("D:\\BaiduNetdiskWorkspace\\GitHub\\lc6261\\dll\\testdll0\\dll_test_0\\x64\\Debug\\dll_test_0.dll"));
	if (hModule == NULL)
	{
		MessageBox(NULL, _T("err hModule"), _T("testdll_1"), MB_OK);
		printf("%s:%d:0x%x:err LoadLibrary\n", __FUNCTION__, __LINE__, hModule);
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
	std::getchar();

	//free
	FreeLibrary(hModule);
	std::getchar();
    return 0;
}

