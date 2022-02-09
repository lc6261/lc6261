#include "pch.h"

void myhook::HookFunc(PVOID* pFunc, PVOID Func)
{
    // Apply the detour to the vtable.
    DetourTransactionBegin();

    DetourUpdateThread(GetCurrentThread());

    DetourAttach(&(PVOID&)pFunc, Func);

    DetourTransactionCommit();
}

void myhook::UnHookFunc(PVOID* pFunc, PVOID Func)
{
    DetourTransactionBegin();

    DetourUpdateThread(GetCurrentThread());

    DetourDetach(&(PVOID&)pFunc, Func);

    DetourTransactionCommit();
}
