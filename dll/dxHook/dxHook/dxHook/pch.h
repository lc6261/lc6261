// pch.h: 这是预编译标头文件。
// 下方列出的文件仅编译一次，提高了将来生成的生成性能。
// 这还将影响 IntelliSense 性能，包括代码完成和许多代码浏览功能。
// 但是，如果此处列出的文件中的任何一个在生成之间有更新，它们全部都将被重新编译。
// 请勿在此处添加要频繁更新的文件，这将使得性能优势无效。

#ifndef PCH_H
#define PCH_H

// 添加要在此处预编译的标头
//#include "framework.h"


#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
// Windows 头文件
#include <windows.h>

#include "ImGui\imconfig.h"
#include "ImGui\imgui.h"
#include "ImGui\imgui_internal.h"
#include "ImGui\imstb_rectpack.h"
#include "ImGui\imstb_textedit.h"
#include "ImGui\imstb_truetype.h"
#include "ImGui\imgui_impl_dx11.h"

#include "d3d11.h"
#include "Detours.h"
#include "myhook.h"

#endif //PCH_H
