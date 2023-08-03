#include "d3dclass.h"
#include <stdio.h>
#include <d3dcompiler.h>
#include <direct.h>
#include "WICTextureLoader11.h"
#include "ScreenGrab.h"
#include "wincodec.h"
#define CHECK_D3D_ERROR(hr) if(FAILED(hr)){printf("D3d error = %d, at %s:%i\n", hr, __FILE__, __LINE__); exit(-1);}

D3dClass::D3dClass():m_vsync_enabled(false)
{
	char path[1024];
	getcwd(path, 1024);
	printf("cwd path:%s\n", path);
}


D3dClass::~D3dClass()
{
	//释放资源
	if (pDevice != nullptr)
		pDevice->Release();
	if (pSwap != nullptr)
		pSwap->Release();
	if (pContext != nullptr)
		pContext->Release();
	if (pRenderTargetView != nullptr)
		pRenderTargetView->Release();
}

int D3dClass::RenderToTexture()
{
    HRESULT hr = S_FALSE;
    Vertex vertices[] =			// 顶点数组
    {
        DirectX::XMFLOAT3(0.0f, 0.3f, 0.3f),
        DirectX::XMFLOAT3(0.3f, -0.3f, 0.3f),
        DirectX::XMFLOAT3(-0.3f, -0.3f, 0.3f),
    };

#if 1
    static ID3D11Texture2D* srcTexture2D = nullptr;
    static ID3D11Texture2D* dstTexture2D = nullptr;
    static ID3D11RenderTargetView* dstRenderTargetView = nullptr;
	static int dstW = 0;
	static int dstH = 0;
	if (srcTexture2D != nullptr)
		return -1;

        ID3D11Resource* tex;
        ID3D11ShaderResourceView* testShaderResView = nullptr;
        hr = DirectX::CreateWICTextureFromFile(
            pDevice,
            L"../data/small.jpg",
            &tex, &testShaderResView);
        if (FAILED(hr))
        {
            MessageBox(NULL, L"ERROR::CreateWICTextureFromFile", L"Error", MB_OK);
            return -1;
        }
        if (FAILED(tex->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&srcTexture2D)))
        {
            MessageBox(NULL, L"ERROR::QueryInterface pTexture2D", L"Error", MB_OK);
            return false;
        }

        D3D11_TEXTURE2D_DESC srcTexDesc;
		srcTexture2D->GetDesc(&srcTexDesc);

        D3D11_TEXTURE2D_DESC dstTextureDesc = {};
        // Setup the render target texture description.
        dstTextureDesc.Width = srcTexDesc.Width;
        dstTextureDesc.Height = srcTexDesc.Height;
        dstTextureDesc.MipLevels = 1;
        dstTextureDesc.ArraySize = 1;
        dstTextureDesc.Format = srcTexDesc.Format;
        dstTextureDesc.SampleDesc.Count = 1;
        dstTextureDesc.Usage = D3D11_USAGE_DEFAULT;
        dstTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        dstTextureDesc.CPUAccessFlags = 0; // D3D11_CPU_ACCESS_READ;
        dstTextureDesc.MiscFlags = 0;

        hr = pDevice->CreateTexture2D(&dstTextureDesc, NULL, &dstTexture2D);
        if (FAILED(hr))
        {
			MessageBox(NULL, L"ERROR::CreateTexture2D dst", L"Error", MB_OK);
            return -1;
        }

		pContext->CopyResource(dstTexture2D, srcTexture2D);
		//hr = DirectX::SaveWICTextureToFile(pContext, dstTexture2D, GUID_ContainerFormatPng, L"output2.png");


		// rtv
        // Setup the description of the render target view.
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
        renderTargetViewDesc.Format = srcTexDesc.Format;
        renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        renderTargetViewDesc.Texture2D.MipSlice = 0;

        // Create the render target view.
        hr = pDevice->CreateRenderTargetView(dstTexture2D, &renderTargetViewDesc, &dstRenderTargetView);
        if (FAILED(hr))
        {
            MessageBox(NULL, L"ERROR::CreateRenderTargetView dst", L"Error", MB_OK);
            return -1;
        }

		dstW = srcTexDesc.Width;
		dstH = srcTexDesc.Height;

#endif

    //start ***********************************************/
    //1.顶点缓冲描述
    D3D11_BUFFER_DESC vertexBufferDesc;
    ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;		// 默认使用
    vertexBufferDesc.ByteWidth = sizeof(Vertex) * 3;	// 大小（我们有三个顶点）
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;	// Bind

    //2.顶点数据
    D3D11_SUBRESOURCE_DATA vsData;
    ZeroMemory(&vsData, sizeof(D3D11_SUBRESOURCE_DATA));
    vsData.pSysMem = vertices;

    //3.创建顶点缓冲区
    ID3D11Buffer* pVBO = nullptr;
    hr = pDevice->CreateBuffer(&vertexBufferDesc, &vsData, &pVBO);
    CHECK_D3D_ERROR(hr);

    //4.为顶点缓冲区设置 CPU 描述符handle，分配到管道
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    pContext->IASetVertexBuffers(0, 1, &pVBO, &stride, &offset);

    //5.创建 vertex shader
    ID3D11VertexShader* pVertexShader;
    ID3DBlob* pBlob;//存储shader中的内容
    hr = D3DReadFileToBlob(L"../bin/vs.cso", &pBlob);
    CHECK_D3D_ERROR(hr);
    hr = pDevice->CreateVertexShader(pBlob->GetBufferPointer(),
        pBlob->GetBufferSize(), nullptr, &pVertexShader);
    CHECK_D3D_ERROR(hr);

    //6.绑定 vertex shader 到渲染管线
    pContext->VSSetShader(pVertexShader, nullptr, 0);

    //7.告诉CPU如何从shader中读取数据
    ID3D11InputLayout* pInputLayout;
    //在 DirectX 代码中创建一个 InputLayout 来描述 input-assembler 阶段的数据。
    const D3D11_INPUT_ELEMENT_DESC layout[]{
        {"POSITIONT", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    const UINT numElements = ARRAYSIZE(layout);
    hr = pDevice->CreateInputLayout(layout, numElements,
        pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pInputLayout);
    CHECK_D3D_ERROR(hr);

    //8.绑定 layout
    pContext->IASetInputLayout(pInputLayout);

    //end *************************************************/


    //start ***********************************************/
    //9.创建 pixel shader
    ID3D11PixelShader* pPixelShader;
    ID3DBlob* pBlob_PS;//存储shader中的内容
    hr = D3DReadFileToBlob(L"../bin/ps.cso", &pBlob_PS);
    CHECK_D3D_ERROR(hr);
    hr = pDevice->CreatePixelShader(pBlob_PS->GetBufferPointer(),
        pBlob_PS->GetBufferSize(), nullptr, &pPixelShader);
    CHECK_D3D_ERROR(hr);

    //10.绑定 pixel shader 到渲染管线
    pContext->PSSetShader(pPixelShader, nullptr, 0);
    //end *************************************************/

    //11.指定输出目标（渲染对象）
#if 1
	pContext->OMSetRenderTargets(1, &dstRenderTargetView, nullptr);

    //三角形list
    pContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //12.设置视口变换
    D3D11_VIEWPORT vp;// 视口
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.Width = static_cast<float>(dstW);
    vp.Height = static_cast<float>(dstH);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    pContext->RSSetViewports(1, &vp);

    pContext->Draw(3, 0);//三个顶点，从0号顶点开始

    hr = DirectX::SaveWICTextureToFile(pContext, dstTexture2D, GUID_ContainerFormatPng, L"output2.png");
#else
    pContext->OMSetRenderTargets(1, &pRenderTargetView, nullptr);

    //三角形list
    pContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //12.设置视口变换
    D3D11_VIEWPORT vp;// 视口
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    vp.Width = static_cast<float>(m_screenWidth);
    vp.Height = static_cast<float>(m_screenHeight);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    pContext->RSSetViewports(1, &vp);

    pContext->Draw(3, 0);//三个顶点，从0号顶点开始
#endif

    return 0;
}

int D3dClass::DrawTriangle()
{
	HRESULT hr = S_FALSE;
	Vertex vertices[] =			// 顶点数组
	{
		DirectX::XMFLOAT3(0.0f, 0.3f, 0.3f),
		DirectX::XMFLOAT3(0.3f, -0.3f, 0.3f),
		DirectX::XMFLOAT3(-0.3f, -0.3f, 0.3f),
	};

	//start ***********************************************/
	//1.顶点缓冲描述
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;		// 默认使用
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * 3;	// 大小（我们有三个顶点）
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;	// Bind

	//2.顶点数据
	D3D11_SUBRESOURCE_DATA vsData;
	ZeroMemory(&vsData, sizeof(D3D11_SUBRESOURCE_DATA));
	vsData.pSysMem = vertices;

	//3.创建顶点缓冲区
	ID3D11Buffer *pVBO = nullptr;
	hr = pDevice->CreateBuffer(&vertexBufferDesc, &vsData, &pVBO);
	CHECK_D3D_ERROR(hr);

	//4.为顶点缓冲区设置 CPU 描述符handle，分配到管道
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	pContext->IASetVertexBuffers(0, 1, &pVBO, &stride, &offset);

	//5.创建 vertex shader
	ID3D11VertexShader* pVertexShader;
	ID3DBlob* pBlob;//存储shader中的内容
	hr = D3DReadFileToBlob(L"../bin/vs.cso", &pBlob);
	CHECK_D3D_ERROR(hr);
	hr = pDevice->CreateVertexShader(pBlob->GetBufferPointer(), 
		pBlob->GetBufferSize(), nullptr, &pVertexShader);
	CHECK_D3D_ERROR(hr);

	//6.绑定 vertex shader 到渲染管线
	pContext->VSSetShader(pVertexShader, nullptr, 0);

	//7.告诉CPU如何从shader中读取数据
	ID3D11InputLayout* pInputLayout;
	//在 DirectX 代码中创建一个 InputLayout 来描述 input-assembler 阶段的数据。
	const D3D11_INPUT_ELEMENT_DESC layout[]{
		{"POSITIONT", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	const UINT numElements = ARRAYSIZE(layout);
	hr = pDevice->CreateInputLayout(layout, numElements, 
		pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pInputLayout);
	CHECK_D3D_ERROR(hr);

	//8.绑定 layout
	pContext->IASetInputLayout(pInputLayout);

	//end *************************************************/


	//start ***********************************************/
	//9.创建 pixel shader
	ID3D11PixelShader* pPixelShader;
	ID3DBlob* pBlob_PS;//存储shader中的内容
	hr = D3DReadFileToBlob(L"../bin/ps.cso", &pBlob_PS);
	CHECK_D3D_ERROR(hr);
	hr = pDevice->CreatePixelShader(pBlob_PS->GetBufferPointer(),
		pBlob_PS->GetBufferSize(), nullptr, &pPixelShader);
	CHECK_D3D_ERROR(hr);

	//10.绑定 pixel shader 到渲染管线
	pContext->PSSetShader(pPixelShader, nullptr, 0);
	//end *************************************************/

	//11.指定输出目标（渲染对象）
	pContext->OMSetRenderTargets(1, &pRenderTargetView, nullptr);

	//三角形list
	pContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	//12.设置视口变换
	D3D11_VIEWPORT vp;// 视口
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = static_cast<float>(m_screenWidth);
	vp.Height = static_cast<float>(m_screenHeight);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	pContext->RSSetViewports(1, &vp);

	pContext->Draw(3, 0);//三个顶点，从0号顶点开始
	return 0;
}

int D3dClass::DrawRect()
{
	HRESULT hr = S_FALSE;
	Vertex vertices[] =			// 顶点数组
	{
		DirectX::XMFLOAT3(-0.1f, -0.1f, 0.3f),
		DirectX::XMFLOAT3(-0.1f, 0.1f, 0.3f),
		DirectX::XMFLOAT3(0.1f, -0.1f, 0.3f),
		DirectX::XMFLOAT3(0.3f, 0.3f, 0.3f),
	};

	//IBO
	//const int indexCount = 6;
	//unsigned long indices[indexCount] = {0,1,2,
	//									 1,3,2};
	const int indexCount = 3;
	unsigned long indices[indexCount] = { 0,1,2 };


	//VBO-start ***********************************************/
	//1.1 顶点缓冲描述
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;		// 默认使用
	vertexBufferDesc.ByteWidth = sizeof(vertices);	// 大小（我们有三个顶点）
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;	// Bind

	//1.2 顶点数据
	D3D11_SUBRESOURCE_DATA vsData;
	ZeroMemory(&vsData, sizeof(D3D11_SUBRESOURCE_DATA));
	vsData.pSysMem = vertices;

	//1.3 创建顶点缓冲区
	ID3D11Buffer *pVBO = nullptr;
	hr = pDevice->CreateBuffer(&vertexBufferDesc, &vsData, &pVBO);
	CHECK_D3D_ERROR(hr);

	//1.4 为顶点缓冲区设置 CPU 描述符handle，分配到管道
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	pContext->IASetVertexBuffers(0, 1, &pVBO, &stride, &offset);
	//VBO-end *************************************************/

	//start ***********************************************/
	//2.1 索引缓冲描述
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	//2.2 索引数据
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	//2.3 创建索引缓冲区
	ID3D11Buffer *pIBO = nullptr;
	hr = pDevice->CreateBuffer(&indexBufferDesc, &indexData, &pIBO);
	CHECK_D3D_ERROR(hr);

	//2.4 为索引缓冲区设置 CPU 描述符handle，分配到管道
	pContext->IASetIndexBuffer(pIBO, DXGI_FORMAT_R32_UINT, 0);
	//end *************************************************/

	//3.创建 vertex shader
	ID3D11VertexShader* pVertexShader;
	ID3DBlob* pBlob;//存储shader中的内容
	hr = D3DReadFileToBlob(L"../bin/vs.cso", &pBlob);
	CHECK_D3D_ERROR(hr);
	hr = pDevice->CreateVertexShader(pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(), nullptr, &pVertexShader);
	CHECK_D3D_ERROR(hr);

	//4.绑定 vertex shader 到渲染管线
	pContext->VSSetShader(pVertexShader, nullptr, 0);

	//5.告诉CPU如何从shader中读取数据
	ID3D11InputLayout* pInputLayout;
	//在 DirectX 代码中创建一个 InputLayout 来描述 input-assembler 阶段的数据。
	const D3D11_INPUT_ELEMENT_DESC layout[]{
		{"POSITIONT", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	const UINT numElements = ARRAYSIZE(layout);
	hr = pDevice->CreateInputLayout(layout, numElements,
		pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pInputLayout);
	CHECK_D3D_ERROR(hr);

	//6.绑定 layout
	pContext->IASetInputLayout(pInputLayout);

	//end *************************************************/


	//start ***********************************************/
	//7.创建 pixel shader
	ID3D11PixelShader* pPixelShader;
	ID3DBlob* pBlob_PS;//存储shader中的内容
	hr = D3DReadFileToBlob(L"../bin/ps.cso", &pBlob_PS);
	CHECK_D3D_ERROR(hr);
	hr = pDevice->CreatePixelShader(pBlob_PS->GetBufferPointer(),
		pBlob_PS->GetBufferSize(), nullptr, &pPixelShader);
	CHECK_D3D_ERROR(hr);

	//8.绑定 pixel shader 到渲染管线
	pContext->PSSetShader(pPixelShader, nullptr, 0);
	//end *************************************************/

	//9.指定输出目标（渲染对象）
	pContext->OMSetRenderTargets(1, &pRenderTargetView, nullptr);

	//三角形list
	pContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	//10.设置视口变换
	D3D11_VIEWPORT vp;// 视口
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = static_cast<float>(m_screenWidth);
	vp.Height = static_cast<float>(m_screenHeight);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	pContext->RSSetViewports(1, &vp);

	pContext->DrawIndexed(indexCount, 0, 0);
	return 0;
}

int D3dClass::EndFrame()
{
	//翻转后缓存的画面
	HRESULT hr = S_FALSE;

	//Present(同步间隔，标签),屏幕刷新60hz的话，那么若游戏画面想要30hz，则间隔填入2，若能达到60，填入1
	hr = pSwap->Present(1, 0);
	CHECK_D3D_ERROR(hr);

	return 0;
}

int D3dClass::ClearBuffer(float r, float g, float b) noexcept
{
	const float color[] = { r, g, b, 1.0f };
	pContext->ClearRenderTargetView(pRenderTargetView, color);
	return 0;
}


int D3dClass::InitD3d11(HWND hwnd, int screenWidth, int screenHeight)
{
	//D3d设备初始化
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	// 1.配置交换链
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	memset(&swapChainDesc, 0, sizeof(DXGI_SWAP_CHAIN_DESC));

	// Set to a single back buffer.
	swapChainDesc.BufferCount = 1;// 用1个后缓冲

	//帧缓冲的大小和应用程序窗口大小相等.
	swapChainDesc.BufferDesc.Width = m_screenWidth;
	swapChainDesc.BufferDesc.Height = m_screenHeight;

	// 后缓冲的surface的格式为DXGI_FORMAT_R8G8B8A8_UNORM.
	// surface的每个像素用4个无符号的8bit[映射到0-1]来表示。NORM表示归一化。
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	//刷新率
	//刷新率是每秒将后台缓冲区绘制到前台缓冲区的次数。
	//如果 vsync 设置为 true，那么这会将刷新率锁定到系统设置（例如 60hz）。
	//这意味着它每秒只会绘制屏幕 60 次（如果系统刷新率超过 60，则更高）。
	//但是，如果我们将 vsync 设置为 false，那么它会在一秒钟内尽可能多地绘制屏幕，​​但这可能会导致一些视觉伪影。
	if(m_vsync_enabled)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;// numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;// denominator;
	}
	else
	{
	    swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;//不全屏不用管刷新率
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//RENDER_TARGET
	swapChainDesc.OutputWindow = hwnd;

	swapChainDesc.SampleDesc.Count = 1;//不需要抗锯齿
	swapChainDesc.SampleDesc.Quality = 0;

	swapChainDesc.Windowed = TRUE;

	//2.创建设备
	D3D_FEATURE_LEVEL       pD3DFeatureLevel = D3D_FEATURE_LEVEL_11_0;
	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,//默认驱动类型，用于硬件3d加速
		D3D_DRIVER_TYPE_WARP, //快速的软件驱动程序，旧显卡适配更好，可能不支持某些特性
		D3D_DRIVER_TYPE_REFERENCE,//软件驱动类型，完美使用d3d特性而不报错，可用于测试
	};
	const auto numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,//d10兼容
		D3D_FEATURE_LEVEL_10_0,
	};
	const auto numFeatureLevels = ARRAYSIZE(featureLevels);

	HRESULT hr = S_FALSE;
	UINT createDeviceFlags = 0;
	for (auto driverType : driverTypes) {
		hr = D3D11CreateDeviceAndSwapChain(
			nullptr,//传递NULL以使用默认适配器,若有多个显卡，可以指定一个
			driverType,//驱动程序类型
			nullptr,//驱动程序选择默认
			createDeviceFlags,
			featureLevels,//D3D特性级别
			numFeatureLevels,
			D3D11_SDK_VERSION,
			&swapChainDesc,
			&pSwap,
			&pDevice,
			&pD3DFeatureLevel,
			&pContext);
		if (SUCCEEDED(hr)) {
			break;
		}
	}
	CHECK_D3D_ERROR(hr);


	//3.渲染目标视图（Render Target View）,先读取纹理，然后用纹理创建渲染目标视图
	//3.1 获取交换链的后缓存
	ID3D11Resource* pId3D11Texture2D = nullptr;
	hr = pSwap->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&pId3D11Texture2D));//COM queryInterface
	CHECK_D3D_ERROR(hr);


	// CreateRenderTargetView 方法的第二个参数为 D3D11_RENDERTARGETVIEW_DESC 结构，此处使用默认，即nullptr
	hr = pDevice->CreateRenderTargetView(pId3D11Texture2D, nullptr, &pRenderTargetView);
	CHECK_D3D_ERROR(hr);


	pId3D11Texture2D->Release();

	return 0;
}
