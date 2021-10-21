#include "d3dDemo.h"
#include <d3dcompiler.h>
#include "WICTextureLoader11.h"
#include <stdio.h>

D3dDemo::D3dDemo(HWND h) :hwnd(h)
{
	CreateSwapChain();
	CreateBuffer();

	CreateVBO();
	CreateIBO();
	CompileShader();
}

HRESULT D3dDemo::CreateSwapChain()
{
	HRESULT hr = S_FALSE;

	// 配置交换链
	DXGI_SWAP_CHAIN_DESC ds;
	memset(&ds, 0, sizeof(DXGI_SWAP_CHAIN_DESC));
	ds.BufferCount = 1;
	ds.BufferDesc.Width = (UINT)width;
	ds.BufferDesc.Height = (UINT)height;
	ds.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	ds.BufferDesc.RefreshRate.Numerator = 60;
	ds.BufferDesc.RefreshRate.Denominator = 1;
	ds.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	ds.OutputWindow = this->hwnd;
	ds.SampleDesc.Count = 1;
	ds.SampleDesc.Quality = 0;
	ds.Windowed = TRUE;

	//创建交换链
	//IDXGISwapChain* pIdxgiSwapChain = nullptr;//声明交换链
	//ID3D11RenderTargetView* pD3D11RenderTargetView = nullptr;//渲染目标视图
	//ID3D11Device* pD3D11Device = nullptr;//设备
	//ID3D11DeviceContext*    pImmediateContext = nullptr;//设备上下文
	D3D_DRIVER_TYPE         gD3DDriverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL       pD3DFeatureLevel = D3D_FEATURE_LEVEL_11_0;

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	const auto numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	const auto numFeatureLevels = ARRAYSIZE(featureLevels);
	
	//HRESULT hr = S_OK;
	UINT createDeviceFlags = 0;
	for (auto driverType : driverTypes) {
		hr = D3D11CreateDeviceAndSwapChain(
			nullptr,
			driverType,
			nullptr,
			createDeviceFlags,
			featureLevels,
			numFeatureLevels,
			D3D11_SDK_VERSION,
			&ds,
			&pIdxgiSwapChain,
			&pD3D11Device,
			&pD3DFeatureLevel,
			&pImmediateContext);
		if (SUCCEEDED(hr)) {
			break;
		}
	}
	if (FAILED(hr)) {
		return hr;
	}

	return S_OK;
}

HRESULT D3dDemo::CreateBuffer()
{
	HRESULT hr = S_FALSE;

	// 创建渲染目标视图
	ID3D11Texture2D *pId3D11Texture2D = nullptr;
	//获取后备缓冲对象
	hr = pIdxgiSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&pId3D11Texture2D));
	if (FAILED(hr)) {
		return hr;
	}

	// CreateRenderTargetView 方法的第二个参数为 D3D11_RENDERTARGETVIEW_DESC 结构，此处使用默认，即nullptr
	hr = pD3D11Device->CreateRenderTargetView(pId3D11Texture2D, nullptr, &pD3D11RenderTargetView);
	if (FAILED(hr)) {
		return hr;
	}
	//将渲染目标视图绑定到图形管线
	pImmediateContext->OMSetRenderTargets(1, &pD3D11RenderTargetView, nullptr);
	D3D11_VIEWPORT dv;
	dv.Width = width;
	dv.Height = height;
	dv.MaxDepth = 1;
	dv.MinDepth = 0;
	dv.TopLeftX = 0;
	dv.TopLeftY = 0;
	pImmediateContext->RSSetViewports(1, &dv);

	return S_OK;
}

void D3dDemo::CreateVBO()
{
	// 缓存信息描述
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;		// 默认使用
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * 4;	// 大小（我们有三个顶点）
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;	// Bind

	D3D11_SUBRESOURCE_DATA verticesSourceData;
	ZeroMemory(&verticesSourceData, sizeof(D3D11_SUBRESOURCE_DATA));
	verticesSourceData.pSysMem = vertices;

	ID3D11Buffer *pVertexBufferObject = nullptr;
	pD3D11Device->CreateBuffer(&vertexBufferDesc, &verticesSourceData, &pVertexBufferObject); //创建缓冲

	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	//立即上下文对象 pImmediateContext,接手这一段顶点数据缓冲
	pImmediateContext->IASetVertexBuffers(0, 1, &pVertexBufferObject, &stride, &offset);
	//pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void D3dDemo::CreateIBO()
{
	// 设置索引缓冲区描述
	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.ByteWidth = sizeof(indices);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;

	// 新建索引缓冲区
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	ID3D11Buffer *pIndexBufferObject = nullptr;
	(pD3D11Device->CreateBuffer(&indexBufferDesc, &indexData, &pIndexBufferObject));

	//2.4 为索引缓冲区设置 CPU 描述符handle，分配到管道
	pImmediateContext->IASetIndexBuffer(pIndexBufferObject, DXGI_FORMAT_R16_UINT, 0);
	pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

HRESULT D3dDemo::CompileShader()
{
	HRESULT hr = S_FALSE;

	//用来查错的Blob对象
	ID3D10Blob* pErrorMessage = nullptr;

	//顶点着色器 Blob 对象和像素着色器 Blob 对象
	ID3D10Blob* pVertexShaderBlob = nullptr;
	ID3D10Blob* pPixelShaderBlob = nullptr;

	// 编译顶点着色器
	hr = D3DCompileFromFile(L"../HLSL/triangleVertexShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "Main", "vs_5_0", D3DCOMPILER_STRIP_DEBUG_INFO, 0, &pVertexShaderBlob, &pErrorMessage);
	if (FAILED(hr)) {
		printf("vs error: %s\n", static_cast<CHAR*>(pErrorMessage->GetBufferPointer()));
		if (pErrorMessage) MessageBox(NULL, (LPCWSTR)static_cast<CHAR*>(pErrorMessage->GetBufferPointer()), L"Error", MB_OK);
		else MessageBox(NULL, L"Triangle.vs File Not Found", L"Error", MB_OK);
		return hr;
	}
		
	// 编译像素着色器
	hr = D3DCompileFromFile(L"../HLSL/trianglePixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "Main", "ps_5_0", D3DCOMPILER_STRIP_DEBUG_INFO, 0, &pPixelShaderBlob, &pErrorMessage);
	if (FAILED(hr)) {
		printf("ps error: %s\n", static_cast<CHAR*>(pErrorMessage->GetBufferPointer()));
		if (pErrorMessage) MessageBox(NULL, (LPCWSTR)static_cast<CHAR*>(pErrorMessage->GetBufferPointer()), L"Error", MB_OK);
		else MessageBox(NULL, L"Triangle.vs File Not Found", L"Error", MB_OK);
		return hr;
	}

	//创建顶点着色器和像素着色器
	hr = pD3D11Device->CreateVertexShader(pVertexShaderBlob->GetBufferPointer(), 
										  pVertexShaderBlob->GetBufferSize(), nullptr, &pVertexShader);
	if (FAILED(hr)) {
		MessageBox(NULL, L"ERROR::CreateVertexShader", L"Error", MB_OK);
		return hr;
	}
	hr = pD3D11Device->CreatePixelShader(pPixelShaderBlob->GetBufferPointer(), 
										 pPixelShaderBlob->GetBufferSize(), nullptr, &pPixelShader);
	if (FAILED(hr)) {
		MessageBox(NULL, L"ERROR::CreateVertexShader", L"Error", MB_OK);
		return hr;
	}

	//在顶点着色器中,使用了 POSITION,
	//在 DirectX 代码中创建一个 InputLayout 来描述 input-assembler 阶段的数据。
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 } ,
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	const UINT numElements = ARRAYSIZE(layout);
	hr = pD3D11Device->CreateInputLayout(layout, numElements, pVertexShaderBlob->GetBufferPointer(), pVertexShaderBlob->GetBufferSize(), &m_pVertexLayout);
	if (FAILED(hr)) {
		MessageBox(NULL, L"ERROR::CreateInputLayout", L"Error", MB_OK);
		return hr;
	}
	pImmediateContext->IASetInputLayout(m_pVertexLayout);	// 设置

	return S_OK;
}

HRESULT D3dDemo::SetTex()
{
	HRESULT hr = S_FALSE;

	//创建一个采样器描述
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	//使用采样器描述创建一个指针对象：
	ID3D11SamplerState *pSamplerState = nullptr;
	hr = this->pD3D11Device->CreateSamplerState(&samplerDesc, &pSamplerState);
	if (FAILED(hr)) {
		MessageBox(nullptr, L"ERROR::CreateSampler", L"Error", MB_OK);
		return hr;
	}

	//从文件创建一个渲染视图：
	ID3D11ShaderResourceView *pShaderResourceView;
	ID3D11Resource* tex;
	hr = DirectX::CreateWICTextureFromFile(this->pD3D11Device, L"../tex/cheese.jpg", &tex, &pShaderResourceView);
	if (FAILED(hr)) {
		MessageBox(nullptr, L"ERROR::CreateShaderResourceView", L"Error", MB_OK);
		return hr;
	}

	//为立即上下文对象赋值渲染视图和采样器：
	pImmediateContext->PSSetShaderResources(0, 1, &pShaderResourceView);
	pImmediateContext->PSSetSamplers(0, 1, &pSamplerState);

	return S_OK;
}


HRESULT D3dDemo::CreateFbo()
{
	// Our map camera's view and projection matrices
	DirectX::XMMATRIX mapView;
	DirectX::XMMATRIX mapProjection;


	HRESULT hr = S_FALSE;

	//1.Create Texture
	//ID3D11Texture2D*  pTexure2D = nullptr;
	D3D11_TEXTURE2D_DESC td;
	ZeroMemory(&td, sizeof(td));
	td.Width = (UINT)width/2;
	td.Height = (UINT)height/2;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	td.SampleDesc.Count = 1;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	hr = pD3D11Device->CreateTexture2D(&td, nullptr, &renderTargetTextureMap);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"Create texture failed!", L"Error", 0);
		return hr;
	}

	//2.Create	Render Target 创建渲染目标
	D3D11_RENDER_TARGET_VIEW_DESC rd;
	ZeroMemory(&rd, sizeof(rd));
	rd.Format = td.Format;
	rd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rd.Texture2D.MipSlice = 0;

	hr = this->pD3D11Device->CreateRenderTargetView(renderTargetTextureMap, &rd, &renderTargetViewMap);
	if (FAILED(hr)) {
		return hr;
	}


	//3.创建着色器资源视图
	D3D11_SHADER_RESOURCE_VIEW_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.Format = td.Format;
	sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	sd.Texture2D.MostDetailedMip = 0;
	sd.Texture2D.MipLevels = 1;

	hr = this->pD3D11Device->CreateShaderResourceView(renderTargetTextureMap, &sd, &shaderResourceViewMap);
	if (FAILED(hr)) {
		return hr;
	}

	//4.创建视图和投影矩阵
	//////////////////////// Map's camera information
	// We will have the camera follow the player
	//DirectX::XMVECTOR mapCamPosition = DirectX::XMVectorSetY(camPosition, DirectX::XMVectorGetY(camPosition) + 100.0f);
	//DirectX::XMVECTOR mapCamTarget = camPosition;
	//DirectX::XMVECTOR mapCamUp = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	////Set the View matrix
	//mapView = DirectX::XMMatrixLookAtLH(mapCamPosition, mapCamTarget, mapCamUp);

	//// Build an orthographic projection matrix
	//mapProjection = DirectX::XMMatrixOrthographicLH(512, 512, 1.0f, 1000.0f);


	return S_OK;
}

void D3dDemo::DrawScene()
{
	float color[] = { 0.1f , 0.2f , 0.5f , 1.0f };
	pImmediateContext->ClearRenderTargetView(pD3D11RenderTargetView, color);
	pImmediateContext->VSSetShader(pVertexShader, nullptr, 0);	//set vs
	pImmediateContext->PSSetShader(pPixelShader, nullptr, 0);	//set ps

	/************************************New Stuff****************************************************/
	 // Set our maps Render Target
	pImmediateContext->OMSetRenderTargets(1, &renderTargetViewMap, nullptr);

	// Now clear the render target
	//pImmediateContext->ClearRenderTargetView(renderTargetViewMap, color);

	// Make sure to set the render target back
	//pImmediateContext->OMSetRenderTargets(1, &pD3D11RenderTargetView, nullptr);

	pImmediateContext->PSSetShaderResources(0, 1, &shaderResourceViewMap);    // Draw the map to the square
	//pImmediateContext->PSSetSamplers(0, 1, &pSamplerState);

	//为立即上下文对象赋值渲染视图和采样器：
	//pImmediateContext->PSSetSamplers(0, 1, &pSamplerState);

	pImmediateContext->Draw(3, 0);		//draw
	pIdxgiSwapChain->Present(0, 0);


}