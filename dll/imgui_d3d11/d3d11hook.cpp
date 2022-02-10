#include "stdafx.h"
#include <d3d11.h>
#include <mutex>

#include "MinHook\include\MinHook.h"

#include "d3d11hook.h"

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"D3DCompiler.lib")

#ifdef _WIN64 // is x64
#pragma comment(lib,"libMinHook.x64.lib")
#else
#pragma comment(lib,"libMinHook.x86.lib")
#endif

// KeyBoard Options.
const int OpenMenuKey = VK_F9;
const int UninjectLibraryKey = VK_DELETE;

#include "Menu.h"
#include "InputHook.h"

Menu menu = Menu();
InputHook inputHook = InputHook();

typedef HRESULT(__stdcall *D3D11PresentHook) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef void(__stdcall *D3D11DrawIndexedHook) (ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation);
typedef void(__stdcall *D3D11CreateQueryHook) (ID3D11Device* pDevice, const D3D11_QUERY_DESC *pQueryDesc, ID3D11Query **ppQuery);
typedef void(__stdcall *D3D11PSSetShaderResourcesHook) (ID3D11DeviceContext* pContext, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView *const *ppShaderResourceViews);
typedef void(__stdcall *D3D11ClearRenderTargetViewHook) (ID3D11DeviceContext* pContext, ID3D11RenderTargetView *pRenderTargetView, const FLOAT ColorRGBA[4]);

static HWND                     g_hWnd = nullptr;
static HMODULE					g_hModule = nullptr;
static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static std::once_flag           g_isInitialized;

D3D11PresentHook                phookD3D11Present = nullptr;
D3D11DrawIndexedHook            phookD3D11DrawIndexed = nullptr;
D3D11CreateQueryHook			phookD3D11CreateQuery = nullptr;
D3D11PSSetShaderResourcesHook	phookD3D11PSSetShaderResources = nullptr;
D3D11ClearRenderTargetViewHook  phookD3D11ClearRenderTargetViewHook = nullptr;

DWORD_PTR*                         pSwapChainVTable = nullptr;
DWORD_PTR*						   pDeviceVTable = nullptr;
DWORD_PTR*                         pDeviceContextVTable = nullptr;

D3D11_HOOK_API void ImplHookDX11_Present(ID3D11Device *device, ID3D11DeviceContext *ctx, IDXGISwapChain *swap_chain)
{
	if (GetAsyncKeyState(OpenMenuKey) & 0x1) {
		menu.IsOpen ? menu.IsOpen = false : menu.IsOpen = true;
	}

	menu.Render();
}

HRESULT __stdcall PresentHook(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	std::call_once(g_isInitialized, [&]()
		{
			pSwapChain->GetDevice(__uuidof(g_pd3dDevice), reinterpret_cast<void**>(&g_pd3dDevice));
			g_pd3dDevice->GetImmediateContext(&g_pd3dContext);

			ImGui_ImplDX11_Init(g_hWnd, g_pd3dDevice, g_pd3dContext);
			inputHook.Init(g_hWnd);
			MessageBox(g_hWnd, L"PresentHook.", L"g_isInitialized DONE", MB_ICONERROR);
		});
	MessageBox(g_hWnd, L"PresentHook.", L"PresentHook OK", MB_ICONERROR);

	ImplHookDX11_Present(g_pd3dDevice, g_pd3dContext, g_pSwapChain);

	return phookD3D11Present(pSwapChain, SyncInterval, Flags);
}

void __stdcall DrawIndexedHook(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation)
{
	return phookD3D11DrawIndexed(pContext, IndexCount, StartIndexLocation, BaseVertexLocation);
}

void __stdcall hookD3D11CreateQuery(ID3D11Device* pDevice, const D3D11_QUERY_DESC *pQueryDesc, ID3D11Query **ppQuery)
{
	if (pQueryDesc->Query == D3D11_QUERY_OCCLUSION)
	{
		D3D11_QUERY_DESC oqueryDesc = CD3D11_QUERY_DESC();
		(&oqueryDesc)->MiscFlags = pQueryDesc->MiscFlags;
		(&oqueryDesc)->Query = D3D11_QUERY_TIMESTAMP;

		return phookD3D11CreateQuery(pDevice, &oqueryDesc, ppQuery);
	}

	return phookD3D11CreateQuery(pDevice, pQueryDesc, ppQuery);
}

UINT pssrStartSlot;
D3D11_SHADER_RESOURCE_VIEW_DESC Descr;

void __stdcall hookD3D11PSSetShaderResources(ID3D11DeviceContext* pContext, UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView *const *ppShaderResourceViews)
{
	pssrStartSlot = StartSlot;

	for (UINT j = 0; j < NumViews; j++)
	{
		ID3D11ShaderResourceView* pShaderResView = ppShaderResourceViews[j];
		if (pShaderResView)
		{
			pShaderResView->GetDesc(&Descr);

			if ((Descr.ViewDimension == D3D11_SRV_DIMENSION_BUFFER) || (Descr.ViewDimension == D3D11_SRV_DIMENSION_BUFFEREX))
			{
				continue; //Skip buffer resources
			}
		}
	}

	return phookD3D11PSSetShaderResources(pContext, StartSlot, NumViews, ppShaderResourceViews);
}

void __stdcall ClearRenderTargetViewHook(ID3D11DeviceContext* pContext, ID3D11RenderTargetView *pRenderTargetView, const FLOAT ColorRGBA[4])
{
	return phookD3D11ClearRenderTargetViewHook(pContext, pRenderTargetView, ColorRGBA);
}

DWORD __stdcall HookDX11_Init()
{
	D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1 };
	D3D_FEATURE_LEVEL obtainedLevel;
	//1.描述交换链
	DXGI_SWAP_CHAIN_DESC sd;													
	{
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;						//像素格式
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;					//缩放比例
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	//扫描方式 
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;						//设置CPU访问后台缓冲区的选项。后台缓冲区用于shader input或者render-target output阶段。一般使用DXGI_USAGE_RENDER_TARGET_OUTPUT。
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;						//可选的标志值，用于描述交换链的选项。该成员是DXGI_SWAP_CHAIN_FLAG枚举类型，其枚举定义如下：
																				//DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH：让程序通过调用IDXGISwapChain::ResizeTarget来转换模式。当应用程序切换到全屏模式时，Direct3D会自动选择与当前的后台缓冲区设置最匹配的显示模式。
																				//DXGI_SWAP_CHAIN_FLAG_NONPREROTATED：关闭图像自动翻转，当把前台缓冲区的内容转换到显示器上时，即不进行图像翻转。若程序想自己控制图像翻转显示的话，通过使用这个标志避免带宽性能损失。在全屏模式下才有效。


		sd.OutputWindow = g_hWnd;
		sd.SampleDesc.Count = 1;
		sd.Windowed = ((GetWindowLongPtr(g_hWnd, GWL_STYLE) & WS_POPUP) != 0) ? false : true;
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		sd.BufferDesc.Width = 1;		//宽度
		sd.BufferDesc.Height = 1;		//高度
		sd.BufferDesc.RefreshRate.Numerator = 0;	//刷新率，分子
		sd.BufferDesc.RefreshRate.Denominator = 1;	//分母
	}
	//2.创建设备接口和交换链接口
UINT createDeviceFlags = NULL;
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D10_CREATE_DEVICE_DEBUG;
#endif

	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		nullptr,									//pAdapter：指定显卡序号，表示要为哪个物理显卡创建设备对象。设为空值时，表示使用主显卡。
		D3D_DRIVER_TYPE_HARDWARE,					//指定Direct3D设备类型。D3D_DRIVER_TYPE_HARDWARE一般使用该参数，表示使用3D硬件加快渲染速度。
		nullptr, 
		createDeviceFlags,							//可选的设备创建标志值。该成员是D3D10_CREATE_DEVICE_FLAG枚举类型，该枚举类型定义如下：
		levels,										//这个指针指向一个D3D_FEATURE_LEVEL数组，来决定创建时排序的功能级别。默认使用最佳的功能级别。
		sizeof(levels) / sizeof(D3D_FEATURE_LEVEL), //pFeatureLevels中的元素数量.
		D3D11_SDK_VERSION, 
		&sd, 
		&g_pSwapChain, 
		&g_pd3dDevice,								//返回一个创建好的 ID3D11Device 设备的指针地址。供应一个NULL指针，返回一个pFeatureLevel中所支持的最高功能级别。
		&obtainedLevel,								//返回一个D3D_FEATURE_LEVEL指针，它来着pFeatureLevel中设备所支持的第一个元素。
		&g_pd3dContext);							//返回一个ID3D11DeviceContext对象指针，它代表这个设备上下文。
	if (FAILED(hr))
	{
		MessageBox(g_hWnd, L"Failed to create device and swapchain.", L"Fatal Error", MB_ICONERROR);
		return E_FAIL;
	}

	pSwapChainVTable = (DWORD_PTR*)(g_pSwapChain);
	pSwapChainVTable = (DWORD_PTR*)(pSwapChainVTable[0]);

	pDeviceVTable = (DWORD_PTR*)(g_pd3dDevice);
	pDeviceVTable = (DWORD_PTR*)pDeviceVTable[0];

	pDeviceContextVTable = (DWORD_PTR*)(g_pd3dContext);
	pDeviceContextVTable = (DWORD_PTR*)(pDeviceContextVTable[0]);

	//MiniHook是一个InlineHOOK 原理就是将目标函数的头部指令copy到一个称之为跳板的区域，并在头部修改代码指向钩子函数，钩子函数中可以调用跳板再跳回去，

	//minihook 初始化
	if (MH_Initialize() != MH_OK) { return 1; }
	//交换链中的下标8的函数func_8执行前，先执行PresentHook，然后跳回func_8，过度函数指针: phookD3D11Present
	if (MH_CreateHook((DWORD_PTR*)pSwapChainVTable[8], PresentHook, reinterpret_cast<void**>(&phookD3D11Present)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)pSwapChainVTable[8]) != MH_OK) { return 1; }

	//设备上下文 中的下标8的函数func_12执行前，先执行DrawIndexedHook，然后跳回func_12，过度函数指针: phookD3D11DrawIndexed
	if (MH_CreateHook((DWORD_PTR*)pDeviceContextVTable[12], DrawIndexedHook, reinterpret_cast<void**>(&phookD3D11DrawIndexed)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)pDeviceContextVTable[12]) != MH_OK) { return 1; }

	if (MH_CreateHook((DWORD_PTR*)pDeviceVTable[24], hookD3D11CreateQuery, reinterpret_cast<void**>(&phookD3D11CreateQuery)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)pDeviceVTable[24]) != MH_OK) { return 1; }

	if (MH_CreateHook((DWORD_PTR*)pDeviceContextVTable[8], hookD3D11PSSetShaderResources, reinterpret_cast<void**>(&phookD3D11PSSetShaderResources)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)pDeviceContextVTable[8]) != MH_OK) { return 1; }

	if (MH_CreateHook((DWORD_PTR*)pSwapChainVTable[50], ClearRenderTargetViewHook, reinterpret_cast<void**>(&phookD3D11ClearRenderTargetViewHook)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)pSwapChainVTable[50]) != MH_OK) { return 1; }

	DWORD old_protect;
	VirtualProtect(phookD3D11Present, 2, PAGE_EXECUTE_READWRITE, &old_protect);

	do {
		Sleep(100);
	} while (!(GetAsyncKeyState(UninjectLibraryKey) & 0x1));

	g_pd3dDevice->Release();
	g_pd3dContext->Release();
	g_pSwapChain->Release();

	ImplHookDX11_Shutdown();
	inputHook.Remove(g_hWnd);

	Beep(220, 100);

	FreeLibraryAndExitThread(g_hModule, 0);

	return S_OK;
}

D3D11_HOOK_API void ImplHookDX11_Init(HMODULE hModule, void *hwnd)
{
	g_hWnd = (HWND)hwnd;
	g_hModule = hModule;
	HookDX11_Init();
}

D3D11_HOOK_API void ImplHookDX11_Shutdown()
{
	if (MH_DisableHook(MH_ALL_HOOKS)) { return; };
	if (MH_Uninitialize()) { return; }
}