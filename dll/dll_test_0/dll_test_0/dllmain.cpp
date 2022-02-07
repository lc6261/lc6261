// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <tchar.h>
#include <string>
//C编译,不加前缀
extern "C" __declspec(dllexport)

int testccc()
{
    //MessageBox(NULL, _T("testccc"), _T("dll"), MB_OK);
    printf("%s:%d:testccc\n", __FUNCTION__, __LINE__);
    return 0;
}



int attach_dll()
{
    //MessageBox(NULL, _T("attach_dll"), _T("dll"), MB_OK);
    printf("%s:%d:attach_dll\n", __FUNCTION__, __LINE__);
    return 0;
}
int detach_dll()
{
    //MessageBox(NULL, _T("detach_dll"), _T("dll"), MB_OK);
    printf("%s:%d:detach_dll\n", __FUNCTION__, __LINE__);
    return 0;
}
int attachThread_dll()
{
    //MessageBox(NULL, _T("attachThread_dll"), _T("dll"), MB_OK);
    printf("%s:%d:attachThread_dll\n", __FUNCTION__, __LINE__);
    return 0;
}
int detachThread_dll()
{
    //MessageBox(NULL, _T("detachThread_dll"), _T("dll"), MB_OK);
    printf("%s:%d:detachThread_dll\n", __FUNCTION__, __LINE__);
    return 0;
}
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            attach_dll();
            break;
        case DLL_THREAD_ATTACH:
            attachThread_dll();
            break;
        case DLL_THREAD_DETACH:
            detachThread_dll();
            break;
        case DLL_PROCESS_DETACH:
            detach_dll();
            break;
        default:
            break;
    }
    return TRUE;
}

