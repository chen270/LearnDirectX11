#include "d3dclass.h"
#include <stdio.h>
#include <d3dcompiler.h>
#include <direct.h>
#include "misc/DXTrace.h"
#include <math.h>
#include "misc/DirectXTex/DDSTextureLoader11.h"
#include "misc/DirectXTex/ScreenGrab.h"
#include "ComputeShader.h"


D3dClass::D3dClass():m_vsync_enabled(false)
{
	char path[1024];
	_getcwd(path, 1024);
	printf("cwd path:%s\n", path);
}


D3dClass::~D3dClass()
{

}

int D3dClass::EndFrame()
{
	//翻转后缓存的画面
	//Present(同步间隔，标签),屏幕刷新60hz的话，那么若游戏画面想要30hz，则间隔填入2，若能达到60，填入1
	HR( pSwap->Present(1, 0));

	return 0;
}

int D3dClass::ClearBuffer(float r, float g, float b) noexcept
{
	const float color[] = { r, g, b, 1.0f };
	pContext->ClearRenderTargetView(pRenderTargetView.Get(), color);
	pContext->ClearDepthStencilView(pDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);
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

#if defined(DEBUG) || defined(_DEBUG)  
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

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
			pSwap.GetAddressOf(),
			pDevice.GetAddressOf(),
			&pD3DFeatureLevel,
			pContext.GetAddressOf());
		if (SUCCEEDED(hr)) {
			break;
		}
	}
	HR(hr);


	//3.渲染目标视图（Render Target View）,先读取纹理，然后用纹理创建渲染目标视图
	//3.1 获取交换链的后缓存
	ComPtr<ID3D11Resource> pId3D11Texture2D;
	hr = pSwap->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(pId3D11Texture2D.GetAddressOf()));//COM queryInterface
	HR(hr);

	// CreateRenderTargetView 方法的第二个参数为 D3D11_RENDERTARGETVIEW_DESC 结构，此处使用默认，即nullptr
	hr = pDevice->CreateRenderTargetView(pId3D11Texture2D.Get(), nullptr, pRenderTargetView.GetAddressOf());
	HR(hr);

	//pId3D11Texture2D.Reset();
	//创建深度缓冲区
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	//dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;

	ComPtr<ID3D11DepthStencilState> pDSState;
	HR(pDevice->CreateDepthStencilState(&dsDesc, pDSState.GetAddressOf()));

	//绑定深度缓冲区
	THROW_D3D_EXCEPTION( pContext->OMSetDepthStencilState(pDSState.Get(), 1u) );

	//创建深度模板纹理
	//深度缓冲区也是一种纹理
	ComPtr<ID3D11Texture2D> pDepthStencil;
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = screenWidth;
	descDepth.Height = screenHeight;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;//DXGI_FORMAT 枚举类型中也定义了深度缓存能使用的格式
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;//设置深度缓冲

	HR(pDevice->CreateTexture2D(&descDepth, nullptr, pDepthStencil.GetAddressOf()));

	//创建深度模板视图
	D3D11_DEPTH_STENCIL_VIEW_DESC descDsv = {};
	descDsv.Format = DXGI_FORMAT_D32_FLOAT;
	descDsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDsv.Texture2D.MipSlice = 0;

	HR(pDevice->CreateDepthStencilView(pDepthStencil.Get(), &descDsv, pDepthStencilView.GetAddressOf()));


	THROW_D3D_EXCEPTION( pContext->OMSetRenderTargets(1u, pRenderTargetView.GetAddressOf(), pDepthStencilView.Get()) );

	//10.设置视口变换
	D3D11_VIEWPORT vp;// 视口
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = static_cast<float>(m_screenWidth);
	vp.Height = static_cast<float>(m_screenHeight);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	pContext->RSSetViewports(1, &vp);

	return 0;
}


//Error code for test
void D3dClass::DrawTestTriangleErr()
{
	namespace wrl = Microsoft::WRL;
	HRESULT hr;

	struct Vertex
	{
		float x;
		float y;
	};

	// create vertex buffer (1 2d triangle at center of screen)
	const Vertex vertices[] =
	{
		{ 0.0f,0.5f },
		{ 0.5f,-0.5f },
		{ -0.5f,-0.5f },
	};
	wrl::ComPtr<ID3D11Buffer> pVertexBuffer;
	D3D11_BUFFER_DESC bd = {};
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0u;
	bd.MiscFlags = 0u;
	bd.ByteWidth = sizeof(vertices);
	bd.StructureByteStride = sizeof(Vertex);
	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = vertices;
	HR(pDevice->CreateBuffer(&bd, &sd, &pVertexBuffer));

	// Bind vertex buffer to pipeline
	const UINT stride = sizeof(Vertex);
	const UINT offset = 0u;
	pContext->IASetVertexBuffers(0u, 1u, &pVertexBuffer, &stride, &offset);


	// create vertex shader
	//wrl::ComPtr<ID3D11VertexShader> pVertexShader;
	//wrl::ComPtr<ID3DBlob> pBlob;
	//HR(D3DReadFileToBlob(L"VertexShader.cso", &pBlob));
	//HR(pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShader));

	//// bind vertex shader
	//pContext->VSSetShader(pVertexShader.Get(), nullptr, 0u);


	THROW_D3D_EXCEPTION(pContext->Draw((UINT)std::size(vertices), 0u));
	//pContext->Draw( (UINT)std::size( vertices ),0u );
}


static const D3D11_INPUT_ELEMENT_DESC inputLayout[2] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0} //RGBA
};


void D3dClass::InitShader_CSO()
{
	//1.读取编译好的着色器二进制信息
	ComPtr<ID3DBlob> pBlobVS;
	HR(D3DReadFileToBlob(L"../bin/Triangle_VS.cso", pBlobVS.GetAddressOf()));

	//2.创建顶点着色器
	/*ComPtr<ID3D11VertexShader> pVertexShader;*/
	HR(pDevice->CreateVertexShader(pBlobVS->GetBufferPointer(),
		pBlobVS->GetBufferSize(), nullptr, pVertexShader.GetAddressOf()));


	//3.设置输入布局(Input Layout)-即定义: 读取编译好的着色器的方法
	ComPtr<ID3D11InputLayout> pInputLayout;
	HR(pDevice->CreateInputLayout(inputLayout, ARRAYSIZE(inputLayout),
		pBlobVS->GetBufferPointer(), pBlobVS->GetBufferSize(), pInputLayout.GetAddressOf()));

	pContext->IASetInputLayout(pInputLayout.Get());   // [In]输入布局


	//4.像素着色器
	ComPtr<ID3DBlob> pBlobPS;
	HR(D3DReadFileToBlob(L"../bin/Triangle_PS.cso", pBlobPS.GetAddressOf()));

	//ComPtr<ID3D11PixelShader> pPixelShader;
	HR(pDevice->CreatePixelShader(pBlobPS->GetBufferPointer(),
		pBlobPS->GetBufferSize(), nullptr, pPixelShader.GetAddressOf()));

}


void D3dClass::InitShader_CompileInRunTime(LPCWSTR vsFilePath, LPCWSTR psFilePath, const D3D11_INPUT_ELEMENT_DESC* _inputLayout, UINT _numelement)
{
	//用来查错的Blob对象
	ComPtr<ID3DBlob> pErrorMessage;

	//顶点着色器 Blob 对象和像素着色器 Blob 对象
	ComPtr<ID3DBlob>  pVertexShaderBlob;
	ComPtr<ID3DBlob>  pPixelShaderBlob;

	// 1.编译顶点着色器
	HR( D3DCompileFromFile(vsFilePath, nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0",
		D3DCOMPILER_STRIP_DEBUG_INFO, 0,
		pVertexShaderBlob.GetAddressOf(), pErrorMessage.ReleaseAndGetAddressOf()) );

	HR(D3DCompileFromFile(psFilePath, nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0",
		D3DCOMPILER_STRIP_DEBUG_INFO, 0,
		pPixelShaderBlob.GetAddressOf(), pErrorMessage.ReleaseAndGetAddressOf()));

	// 2.创建顶点着色器
	HR(pDevice->CreateVertexShader(pVertexShaderBlob->GetBufferPointer(),
		pVertexShaderBlob->GetBufferSize(), nullptr, pVertexShader.GetAddressOf()));

	HR(pDevice->CreatePixelShader(pPixelShaderBlob->GetBufferPointer(),
		pPixelShaderBlob->GetBufferSize(), nullptr, pPixelShader.GetAddressOf()));


	//3.设置输入布局(Input Layout)-即定义: 读取编译好的着色器的方法
	ComPtr<ID3D11InputLayout> pInputLayout;
	(pDevice->CreateInputLayout(_inputLayout, _numelement,//ARRAYSIZE(_inputLayout),
		pVertexShaderBlob->GetBufferPointer(), pVertexShaderBlob->GetBufferSize(), pInputLayout.GetAddressOf()));

	pContext->IASetInputLayout(pInputLayout.Get());   // [In]输入布局
}

bool D3dClass::ResetMesh(const Geometry::MeshData<VertexPosNormalColor>& meshData)
{
	// 释放旧资源
	m_pVertexBuffer.Reset();
	m_pIndexBuffer.Reset();

	// 设置顶点缓冲区描述
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = (UINT)meshData.vertexVec.size() * sizeof(VertexPosNormalColor);
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	// 新建顶点缓冲区
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = meshData.vertexVec.data();
	HR(pDevice->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.GetAddressOf()));

	// 输入装配阶段的顶点缓冲区设置
	UINT stride = sizeof(VertexPosNormalColor);	// 跨越字节数
	UINT offset = 0;							// 起始偏移量

	pContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);


	// 设置索引缓冲区描述
	m_IndexCount = (UINT)meshData.indexVec.size();
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = m_IndexCount * sizeof(DWORD);
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	// 新建索引缓冲区
	InitData.pSysMem = meshData.indexVec.data();
	HR(pDevice->CreateBuffer(&ibd, &InitData, m_pIndexBuffer.GetAddressOf()));
	// 输入装配阶段的索引缓冲区设置
	pContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);


	return true;
}



void D3dClass::InitLightResource()
{
	//InitShader_CompileInRunTime(L"../shader/Light_VS.hlsl", L"../shader/Light_PS.hlsl", 
	//	VertexPosNormalColor::inputLayout, ARRAYSIZE(VertexPosNormalColor::inputLayout));

	// ******************
	// 初始化网格模型
	//
	auto meshData = Geometry::CreateBox<VertexPosNormalColor>();
	ResetMesh(meshData);


	// ******************
	// 设置常量缓冲区描述
	//
	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.ByteWidth = sizeof(VSConstantBuffer);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	// 新建用于VS和PS的常量缓冲区
	HR(pDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[0].GetAddressOf()));
	cbd.ByteWidth = sizeof(PSConstantBuffer);
	HR(pDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffers[1].GetAddressOf()));

	// ******************
	// 初始化默认光照
	// 方向光
	m_DirLight.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	m_DirLight.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_DirLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_DirLight.direction = XMFLOAT3(-0.577f, -0.577f, 0.577f);
	// 点光
	m_PointLight.position = XMFLOAT3(0.0f, 0.0f, -10.0f);
	m_PointLight.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_PointLight.diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	m_PointLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_PointLight.att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	m_PointLight.range = 25.0f;
	// 聚光灯
	m_SpotLight.position = XMFLOAT3(0.0f, 0.0f, -5.0f);
	m_SpotLight.direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_SpotLight.ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_SpotLight.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_SpotLight.specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_SpotLight.att = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_SpotLight.spot = 12.0f;
	m_SpotLight.range = 10000.0f;
	// 初始化用于VS的常量缓冲区的值
	m_VSConstantBuffer.world = XMMatrixIdentity();
	m_VSConstantBuffer.view = XMMatrixTranspose(XMMatrixLookAtLH(
		XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f),
		XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
	));
	m_VSConstantBuffer.proj = XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PIDIV2, AspectRatio(), 1.0f, 1000.0f));
	m_VSConstantBuffer.worldInvTranspose = XMMatrixIdentity();

	// 初始化用于PS的常量缓冲区的值
	m_PSConstantBuffer.material.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_PSConstantBuffer.material.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_PSConstantBuffer.material.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 5.0f);
	// 使用默认平行光
	m_PSConstantBuffer.dirLight = m_DirLight;
	// 注意不要忘记设置此处的观察位置，否则高亮部分会有问题
	m_PSConstantBuffer.eyePos = XMFLOAT4(0.0f, 0.0f, -5.0f, 0.0f);

	// 更新PS常量缓冲区资源
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(pContext->Map(m_pConstantBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(PSConstantBuffer), &m_PSConstantBuffer, sizeof(PSConstantBuffer));
	pContext->Unmap(m_pConstantBuffers[1].Get(), 0);

	// ******************
	// 初始化光栅化状态
	//
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
	rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;
	rasterizerDesc.FrontCounterClockwise = false;
	rasterizerDesc.DepthClipEnable = true;
	HR(pDevice->CreateRasterizerState(&rasterizerDesc, m_pRSWireframe.GetAddressOf()));

	// ******************
	// 给渲染管线各个阶段绑定好所需资源
	//

	// 设置图元类型，设定输入布局
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pContext->IASetInputLayout(m_pVertexLayout.Get());
	// 将着色器绑定到渲染管线
	pContext->VSSetShader(pVertexShader.Get(), nullptr, 0);
	// VS常量缓冲区对应HLSL寄存于b0的常量缓冲区
	pContext->VSSetConstantBuffers(0, 1, m_pConstantBuffers[0].GetAddressOf());
	// PS常量缓冲区对应HLSL寄存于b1的常量缓冲区
	pContext->PSSetConstantBuffers(1, 1, m_pConstantBuffers[1].GetAddressOf());
	pContext->PSSetShader(pPixelShader.Get(), nullptr, 0);
}


void D3dClass::UpdateScene(float x, float y)
{
	using namespace DirectX;
	static float phi = 0.0f, theta = 0.0f;
	phi += 0.0001f, theta += 0.00015f;
	XMMATRIX W = XMMatrixRotationX(phi) * XMMatrixRotationY(theta);
	m_VSConstantBuffer.world = XMMatrixTranspose(W);
	m_VSConstantBuffer.worldInvTranspose = XMMatrixTranspose(InverseTranspose(W));

	// 键盘切换灯光类型
	Keyboard::State state = m_pKeyboard->GetState();
	m_KeyboardTracker.Update(state);
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::D1))
	{
		m_PSConstantBuffer.dirLight = m_DirLight;
		m_PSConstantBuffer.pointLight = PointLight();
		m_PSConstantBuffer.spotLight = SpotLight();
	}
	else if (m_KeyboardTracker.IsKeyPressed(Keyboard::D2))
	{
		m_PSConstantBuffer.dirLight = DirectionalLight();
		m_PSConstantBuffer.pointLight = m_PointLight;
		m_PSConstantBuffer.spotLight = SpotLight();
	}
	else if (m_KeyboardTracker.IsKeyPressed(Keyboard::D3))
	{
		m_PSConstantBuffer.dirLight = DirectionalLight();
		m_PSConstantBuffer.pointLight = PointLight();
		m_PSConstantBuffer.spotLight = m_SpotLight;
	}

	// 键盘切换模型类型
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::Q))
	{
		auto meshData = Geometry::CreateBox<VertexPosNormalColor>();
		ResetMesh(meshData);
	}
	else if (m_KeyboardTracker.IsKeyPressed(Keyboard::W))
	{
		auto meshData = Geometry::CreateSphere<VertexPosNormalColor>();
		ResetMesh(meshData);
	}
	else if (m_KeyboardTracker.IsKeyPressed(Keyboard::E))
	{
		auto meshData = Geometry::CreateCylinder<VertexPosNormalColor>();
		ResetMesh(meshData);
	}
	else if (m_KeyboardTracker.IsKeyPressed(Keyboard::R))
	{
		auto meshData = Geometry::CreateCone<VertexPosNormalColor>();
		ResetMesh(meshData);
	}
	// 键盘切换光栅化状态
	else if (m_KeyboardTracker.IsKeyPressed(Keyboard::S))
	{
		m_IsWireframeMode = !m_IsWireframeMode;
		m_pd3dImmediateContext->RSSetState(m_IsWireframeMode ? m_pRSWireframe.Get() : nullptr);
	}

	// 更新常量缓冲区，让立方体转起来
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[0].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(VSConstantBuffer), &m_VSConstantBuffer, sizeof(VSConstantBuffer));
	m_pd3dImmediateContext->Unmap(m_pConstantBuffers[0].Get(), 0);

	HR(m_pd3dImmediateContext->Map(m_pConstantBuffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(PSConstantBuffer), &m_PSConstantBuffer, sizeof(PSConstantBuffer));
	m_pd3dImmediateContext->Unmap(m_pConstantBuffers[1].Get(), 0);
}

void D3dClass::DrawScene()
{
	assert(pContext);
	assert(pSwap);

	static float black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };    // RGBA = (0,0,0,255)
	pContext->ClearRenderTargetView(pRenderTargetView.Get(), black);
	pContext->ClearDepthStencilView(pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// 绘制三角形
	//pContext->Draw(3, 0);
	pContext->DrawIndexed(36, 0, 0);


	HR(pSwap->Present(0, 0));
}

void D3dClass::UseComputeShader()
{
	ComputeShader* computeShader = new ComputeShader(pDevice, pContext);
	computeShader->RunGPUCompute();
	delete computeShader;
}
