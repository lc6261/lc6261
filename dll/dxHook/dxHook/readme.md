
vs 项目  -> 属性 ->VC++目录->包含目录->
$(DXSDK_DIR)include
D:\BaiduNetdiskWorkspace\GitHub\Detours\include

vs 项目  -> 属性 ->C/C++->预编译头->
不使用预编译头


vs 项目  -> 属性 ->链接器->常规->附加库目录->
$(DXSDK_DIR)Lib\$(PlatformTarget)
D:\BaiduNetdiskWorkspace\GitHub\Detours\lib.X86

vs 项目  -> 属性 ->链接器->输入->附加依赖项->
d3d11.lib
detours.lib


pch.h->

#include "ImGui\imconfig.h"
#include "ImGui\imgui.h"
#include "ImGui\imgui_internal.h"
#include "ImGui\imstb_rectpack.h"
#include "ImGui\imstb_textedit.h"
#include "ImGui\imstb_truetype.h"
#include "ImGui\imgui_impl_dx11.h"

#include "d3d11.h"
#include "Detours.h"


D:\BaiduNetdiskWorkspace\GitHub\Universal-ImGui-D3D11-Hook\x64\Debug\Universal-ImGui-D11-Hook.dll

libMinHook.x86.lib
libMinHook.x64.lib


#ifdef _WIN64 // is x64
#pragma comment(lib,"libMinHook.x64.lib")
#else
#pragma comment(lib,"libMinHook.x86.lib")
#endif

整理
ctrl + k + f
注释
ctrl + k + c
去注释
ctrl + k + u

翻译
ctrl + alt +f

