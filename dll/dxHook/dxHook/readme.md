--------------22.02.09-----------------------------------------project_dll_init-----------------------------------------------
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
D:\\BaiduNetdiskWorkspace\\GitHub\\Universal-ImGui-D3D11-Hook\\x64\\Debug\\Universal-ImGui-D11-Hook.dll


D:\BaiduNetdiskWorkspace\GitHub\minhook\build\VC16\lib\Debug\
libMinHook.x86.lib
D:\BaiduNetdiskWorkspace\GitHub\minhook\build\VC16\lib\Debug
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
--------------22.02.10-----------------------------------------dx10_init-----------------------------------------------
Direct3D 10的初始化可分为以下几个步骤：

1.填充一个DXGI_SWAP_CHAIN_DESC结构体，用于描述了所要创建的交换链特性。
	1.描述交换链
		填充一个DXGI_SWAP_CHAIN_DESC结构体，该结构体定义如下：
		typedef struct DXGI_SWAP_CHAIN_DESC
		{ 
			DXGI_MODE_DESC BufferDesc; 
				BufferDesc：指定显示模式，其中的DXGI_MODE_DESC类型是另一个结构体，其定义如下：
				typedef struct DXGI_MODE_DESC
				{
					UINT Width;//宽度
					UINT Height;//高度
					DXGI_RATIONAL RefreshRate;//刷新率
					DXGI_FORMAT Format;//像素格式
					DXGI_MODE_SCANLINE_ORDER ScanlineOrdering;//扫描方式 
					DXGI_MODE_SCALING Scaling;//伸缩比例
				}DXGI_MODE_DESC;
			DXGI_SAMPLE_DESC SampleDesc; 
				SampleDesc：进行多重采样设置，其中的DXGI_SAMPLE_DESC定义如下：
				typedef struct DXGI_SAMPLE_DESC
				{
					UINT Count;//每像素多重采样个数
					UINT Quality;//图像质量等级，等级越高，性能越低，可选范围是0到ID3D10Device::CheckMultisampleQualityLevels
				}DXGI_SAMPLE_DESC;
				默认的sample模式是：Count=1，Quality=0，即不采用多重采样。		
			DXGI_USAGE BufferUsage;
				BufferUsage：设置CPU访问后台缓冲区的选项。后台缓冲区用于shader input或者render-target output阶段。一般使用DXGI_USAGE_RENDER_TARGET_OUTPUT。
			UINT BufferCount; 
				BufferCount：指定交换链中缓冲区的数量。
			HWNDOutput Window; 
				OutputWindow：指定图形绘制窗口。Windowed标识显示方式。若Windowed为TRUE，以窗口方式显示；若为FALSE，以全屏方式显示。


			BOOL Windowed;
			
			DXGI_SWAP_EFFECT SwapEffect; 
				SwapEffect：指定系统将后台缓冲区的内容复制到前台缓冲区的方式。其枚举如下：
				typedef enum DXGI_SWAP_EFFECT
				{
					DXGI_SWAP_EFFECT_DISCARD = 0,
						DXGI_SWAP_EFFECT_DISCARD：后台缓存复制到前台缓存后，清除后台缓存内容。这个选项，让显卡驱动程序选择最高效的显示模式；只能在交换链中只有一个后台缓冲区的情况下使用。
					DXGI_SWAP_EFFECT_SEQUENTIAL = 1
						DXGI_SWAP_EFFECT_SEQUENTIAL：后台缓存复制到前台缓存后，保留后台缓存原内容不变。这个选项，让交换链按顺序显示所有后台缓冲区中的内容。不能使用多重采样反走样。
				}DXGI_SWAP_EFFECT;

			UINT Flags;
				Flags：可选的标志值，用于描述交换链的选项。该成员是DXGI_SWAP_CHAIN_FLAG枚举类型，其枚举定义如下：
				typedef enum DXGI_SWAP_CHAIN_FLAG
				{
					DXGI_SWAP_CHAIN_FLAG_NONPREROTATED = 1,
						DXGI_SWAP_CHAIN_FLAG_NONPREROTATED ：关闭图像自动翻转，当把前台缓冲区的内容转换到显示器上时，即不进行图像翻转。若程序想自己控制图像翻转显示的话，通过使用这个标志避免带宽性能损失。在全屏模式下才有效。
					DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH = 2,
						DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH ：让程序通过调用IDXGISwapChain::ResizeTarget来转换模式。当应用程序切换到全屏模式时，Direct3D会自动选择与当前的后台缓冲区设置最匹配的显示模式。
					DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE = 4,
					DXGI_SWAP_CHAIN_FLAG_RESTRICTED_CONTENT = 8,
					DXGI_SWAP_CHAIN_FLAG_RESTRICT_SHARED_RESOURCE_DRIVER = 16,
					DXGI_SWAP_CHAIN_FLAG_DISPLAY_ONLY = 32
				}DXGI_SWAP_CHAIN_FLAG;
				如果未指定Flags标志值，当应用程序切换到全屏模式时，Direct3D会使用当前的桌面显示模式。
		}DXGI_SWAP_CHAIN_DESC;
	
	2.代码
		//交换链描述
		DXGI_SWAP_CHAIN_DESC sd;
		
		//后台缓冲区描述
		
		//宽度、高度
		sd.BufferDesc.Width = WINDOW_WIDTH;
		sd.BufferDesc.Height = WINDOW_HEIGHT;
		
		//像素格式
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8_UNORM;
		
		//刷新率
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		
		//扫描方式
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		
		//按比例伸缩
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		
		//多重采样描述
		
		//每像素多重采样个数
		sd.SampleDesc.Count = 1;
		
		//图像质量等级，可选范围为0到ID3D10Device::CheckMultisampleQualityLevels
		sd.SampleDesc.Quality = 0;
		
		//CPU访问后台缓冲区的选项
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		
		//后台缓冲区数量
		sd.BufferCount = 1;
		
		//进行渲染的窗口句柄
		sd.OutputWindow = hMainWnd;
		
		//窗口/全屏显示
		sd.Windowed = TRUE;
		
		//将后台缓冲区内容复制到前台缓冲区的方式
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		
		//交换链行为
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

2.使用 D3D10CreateDeviceAndSwapChain 函数创建 ID3D10Device 设备接口和 IDXGISwapChain 交换链接口。
	1.创建设备接口和交换链接口
	
	先声明两个设备：指向 ID3D10Device 类型的设备接口和指向 IDXGISwapChain 类型的交换链接口。

	这两个接口可以通过下面函数进行创建：
	HRESULT D3D10CreateDeviceAndSwapChain(
				IDXGIAdapter *pAdapter,//pAdapter：指定显卡序号，表示要为哪个物理显卡创建设备对象。设为空值时，表示使用主显卡。
				D3D10_DRIVER_TYPE DriverType,
					DriverType：指定Direct3D设备类型。该成员是D3D_DRIVER_TYPE枚举类型，该枚举类型定义如下：
						D3D10_DRIVER_TYPE_HARDWARE：一般使用该参数，表示使用3D硬件加快渲染速度。
						D3D10_DRIVER_TYPE_REFERENCE：创建引用设备。引用设备为Direct3D的软件实现，具有Direct3D所有功能，但运行速度非常慢。使用引用设备主要用于：测试硬件不支持的代码，
							例如在一块不支持Direct3D 10的显卡上测试Direct3D 10代码；测试驱动程序缺陷，当代码能在引用设备上正常运行，而在硬件上不能正常运行时，说明硬件驱动程序可能存在缺陷。
				HMODULE Software,
					Software：用于软件光栅化设备。如果 DriverType 是 D3D_DRIVER_TYPE_SOFTWARE ，此值不能为 NULL，且必须先安装一个软件光栅化设备。否则设置为 NULL，因为使用硬件加速渲染。
				UINT Flags,
					Flags：可选的设备创建标志值。该成员是D3D10_CREATE_DEVICE_FLAG枚举类型，该枚举类型定义如下：
						D3D10_CREATE_DEVICE_DEBUG：当以debug模式生成程序时，参数应设为 D3D10_CREATE_DEVICE_DEBUG，以激活调试层。
						在指定调试标志后，Direct3D会向VS输出窗口发送调试信息。否则，即当以release模式生成程序时，Flags设为空。
				UINT SDKVersion,
					SDKVersion：SDK版本，始终设为 D3D10_SDK_VERSION。
				DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
					pSwapChainDesc：指向 DXGI_SWAP_CHAIN_DESC 结构体的指针，即指向一开始填充的交换链描述结构体。
				IDXGISwapChain **ppSwapChain,
					ppSwapChain：用于返回创建后的交换链对象。即应指向所要创建的 IDXGISwapChain 类型的交换链接口。
				ID3D10Device **ppDevice
					ppDevice：用于返回创建后的设备对象。即应指向所要创建的 ID3D10Device 类型的设备接口。
			D3D11 添加
			    CONST D3D_FEATURE_LEVEL* pFeatureLevels,
					pFeatureLevels：这个指针指向一个D3D_FEATURE_LEVEL数组，来决定创建时排序的功能级别。默认使用最佳的功能级别。
				UINT FeatureLevels,
					FeatureLevels : pFeatureLevels 中的元素数量.
			    D3D_FEATURE_LEVEL* pFeatureLevel,
					pFeatureLevel:返回一个D3D_FEATURE_LEVEL指针，它来着pFeatureLevel中设备所支持的第一个元素。
				ID3D11DeviceContext** ppImmediateContext
					ppImmediateContext:返回一个 ID3D11DeviceContext 对象指针，它代表这个设备上下文。
				）;
	2.示例
	//运行层开关
    UINT createDeviceFlags = NULL;
    #if defined(DEBUG) || defined(_DEBUG)
        createDeviceFlags |= D3D10_CREATE_DEVICE_DEBUG;
    #endif

    //创建设备和交换链
    ID3D10Device* pd3dDevice;
    IDXGISwapChain* pSwapChain;
    D3D10CreateDeviceAndSwapChain(NULL, D3D10_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, D3D10_SDK_VERSION, &sd, &pSwapChain, &pd3dDevice);
	
3.为交换链后台缓冲区创建一个渲染目标视图。
	1.创建渲染目标视图
	先声明两个接口：指向 ID3D10RenderTargetView 类型的渲染目标视图接口和指向 ID3D10Texture2D 类型（或者其他资源格式）的后台缓冲区.
	
	通过设备接口调用 ID3D10Device::CreateRenderTargetView 方法创建渲染目标视图。CreateRenderTargetView 方法如下：
	HRESULT ID3D10Device::CreateRenderTargetView(
		ID3D10Resource *pResource,
			pResource：指定将要作为渲染目标的资源。如将后台缓冲区作为渲染目标。
		const D3D10_RENDER_TARGET_VIEW_DESC *pDesc,
			pDesc：指向D3D10_RENDER_TARGET_VIEW_DESC结构体的指针，该结构体描述渲染目标即参数pResource所指定的资源中的元素的数据类型。如果创建资源时使用强类型，则可为空，表示以资源自身格式为视图格式。
		ID3D10RenderTargetView **ppRTView
			ppRTView：指向创建后所要返回的渲染目标视图对象的指针。
		);




	通过交换链接口调用 IDXGISwapChain::GetBuffer 方法来获取后台缓冲区指针。同时应注意，每次调用该方法后，后台缓冲区的COM引用计数会向上递增一次，所以应配对使用 Release 方法。
		GetBuffer方法如下：
		HRESULT IDXGISwapChain::GetBuffer(
			UINT Buffer,
				Buffer：表示所要获取的后台缓冲区的索引。由于后台缓冲区数量可以大于1，所以必须指定索引。
			const IID &riid,
				riid：缓冲区接口类型。通常为2D纹理，即ID3D10Texture2D。
			void **ppSurface
				ppSurface：指向所返回的后台缓冲区的指针。
			);




4.创建深度/模板缓冲区以及相关的深度/模板视图。

5.将渲染目标视图和深度/模板视图绑定到渲染管线的输出合并器阶段，使它们可以被 Direct3D 使用。

6.设置视口。


