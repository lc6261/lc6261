#pragma once
class myhook
{
public:
	static void HookFunc(PVOID* pFunc, PVOID Func);
	static void UnHookFunc(PVOID* pFunc, PVOID Func);

};

