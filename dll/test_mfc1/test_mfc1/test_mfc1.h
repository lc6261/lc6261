
// test_mfc1.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// Ctest_mfc1App: 
// �йش����ʵ�֣������ test_mfc1.cpp
//

class Ctest_mfc1App : public CWinApp
{
public:
	Ctest_mfc1App();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern Ctest_mfc1App theApp;