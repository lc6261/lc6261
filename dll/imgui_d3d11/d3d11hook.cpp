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
	//1.����������
	DXGI_SWAP_CHAIN_DESC sd;													
	{
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;						//���ظ�ʽ
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;					//���ű���
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	//ɨ�跽ʽ 
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;						//����CPU���ʺ�̨��������ѡ���̨����������shader input����render-target output�׶Ρ�һ��ʹ��DXGI_USAGE_RENDER_TARGET_OUTPUT��
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;						//��ѡ�ı�־ֵ������������������ѡ��ó�Ա��DXGI_SWAP_CHAIN_FLAGö�����ͣ���ö�ٶ������£�
																				//DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH���ó���ͨ������IDXGISwapChain::ResizeTarget��ת��ģʽ����Ӧ�ó����л���ȫ��ģʽʱ��Direct3D���Զ�ѡ���뵱ǰ�ĺ�̨������������ƥ�����ʾģʽ��
																				//DXGI_SWAP_CHAIN_FLAG_NONPREROTATED���ر�ͼ���Զ���ת������ǰ̨������������ת������ʾ����ʱ����������ͼ��ת�����������Լ�����ͼ��ת��ʾ�Ļ���ͨ��ʹ�������־�������������ʧ����ȫ��ģʽ�²���Ч��


		sd.OutputWindow = g_hWnd;
		sd.SampleDesc.Count = 1;
		sd.Windowed = ((GetWindowLongPtr(g_hWnd, GWL_STYLE) & WS_POPUP) != 0) ? false : true;
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		sd.BufferDesc.Width = 1;		//���
		sd.BufferDesc.Height = 1;		//�߶�
		sd.BufferDesc.RefreshRate.Numerator = 0;	//ˢ���ʣ�����
		sd.BufferDesc.RefreshRate.Denominator = 1;	//��ĸ
	}
	//2.�����豸�ӿںͽ������ӿ�
UINT createDeviceFlags = NULL;
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D10_CREATE_DEVICE_DEBUG;
#endif

	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		nullptr,									//pAdapter��ָ���Կ���ţ���ʾҪΪ�ĸ������Կ������豸������Ϊ��ֵʱ����ʾʹ�����Կ���
		D3D_DRIVER_TYPE_HARDWARE,					//ָ��Direct3D�豸���͡�D3D_DRIVER_TYPE_HARDWAREһ��ʹ�øò�������ʾʹ��3DӲ���ӿ���Ⱦ�ٶȡ�
		nullptr, 
		createDeviceFlags,							//��ѡ���豸������־ֵ���ó�Ա��D3D10_CREATE_DEVICE_FLAGö�����ͣ���ö�����Ͷ������£�
		levels,										//���ָ��ָ��һ��D3D_FEATURE_LEVEL���飬����������ʱ����Ĺ��ܼ���Ĭ��ʹ����ѵĹ��ܼ���
		sizeof(levels) / sizeof(D3D_FEATURE_LEVEL), //pFeatureLevels�е�Ԫ������.
		D3D11_SDK_VERSION, 
		&sd, 
		&g_pSwapChain, 
		&g_pd3dDevice,								//����һ�������õ� ID3D11Device �豸��ָ���ַ����Ӧһ��NULLָ�룬����һ��pFeatureLevel����֧�ֵ���߹��ܼ���
		&obtainedLevel,								//����һ��D3D_FEATURE_LEVELָ�룬������pFeatureLevel���豸��֧�ֵĵ�һ��Ԫ�ء�
		&g_pd3dContext);							//����һ��ID3D11DeviceContext����ָ�룬����������豸�����ġ�
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

	//MiniHook��һ��InlineHOOK ԭ����ǽ�Ŀ�꺯����ͷ��ָ��copy��һ����֮Ϊ��������򣬲���ͷ���޸Ĵ���ָ���Ӻ��������Ӻ����п��Ե�������������ȥ��

	//minihook ��ʼ��
	if (MH_Initialize() != MH_OK) { return 1; }
	//�������е��±�8�ĺ���func_8ִ��ǰ����ִ��PresentHook��Ȼ������func_8�����Ⱥ���ָ��: phookD3D11Present
	if (MH_CreateHook((DWORD_PTR*)pSwapChainVTable[8], PresentHook, reinterpret_cast<void**>(&phookD3D11Present)) != MH_OK) { return 1; }
	if (MH_EnableHook((DWORD_PTR*)pSwapChainVTable[8]) != MH_OK) { return 1; }

	//�豸������ �е��±�8�ĺ���func_12ִ��ǰ����ִ��DrawIndexedHook��Ȼ������func_12�����Ⱥ���ָ��: phookD3D11DrawIndexed
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