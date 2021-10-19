#include "d3dclass.h"
#include <stdio.h>
#include <d3dcompiler.h>
#include <direct.h>
#include "WICTextureLoader11.h"
#include <cmath>
#include "dxtrace.h"

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
	if (m_pDevice != nullptr)
		m_pDevice->Release();
	if (m_pSwap != nullptr)
		m_pSwap->Release();
	if (m_pContext != nullptr)
		m_pContext->Release();
	if (m_pRenderTargetView != nullptr)
		m_pRenderTargetView->Release();
}

int D3dClass::DrawCube(float angle, float x, float z)
{
	HRESULT hr = S_FALSE;
	VertexPosColor vertices[] =			// 顶点数组
	{
		{DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f),{255, 0, 0, 1}},
		{DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f),{0, 255, 0, 1} },
		{DirectX::XMFLOAT3(-1.0f, 1.0f, -1.0f),{0, 0, 255, 1} },
		{DirectX::XMFLOAT3(1.0f, 1.0f, -1.0f), {255, 255, 0, 1} },
		{DirectX::XMFLOAT3(-1.0f, -1.0f, 1.0f), {255, 0, 255, 1} },
		{DirectX::XMFLOAT3(1.0f, -1.0f, 1.0f), {0, 255, 255, 1} },
		{DirectX::XMFLOAT3(-1.0f, 1.0f, 1.0f), {0, 0, 0, 1} },
		{DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), {255, 255, 255, 1} },
	};

	//IBO
	//
	const unsigned short indices[] =
	{
		0,2,1, 2,3,1,
		1,3,5, 3,7,5,
		2,6,3, 3,6,7,
		4,5,7, 4,7,6,
		0,4,2, 2,4,6,
		0,1,4, 1,5,4
	};
	const int indexCount = sizeof(indices) / sizeof(unsigned short);

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
	hr = m_pDevice->CreateBuffer(&vertexBufferDesc, &vsData, &pVBO);
	CHECK_D3D_ERROR(hr);

	//1.4 为顶点缓冲区设置 CPU 描述符handle，分配到管道
	UINT stride = sizeof(VertexPosColor);
	UINT offset = 0;
	m_pContext->IASetVertexBuffers(0, 1, &pVBO, &stride, &offset);
	//VBO-end *************************************************/

	//start ***********************************************/
	//2.1 索引缓冲描述
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(indices);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = sizeof(unsigned short);

	//2.2 索引数据
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	//2.3 创建索引缓冲区
	ID3D11Buffer *pIBO = nullptr;
	hr = m_pDevice->CreateBuffer(&indexBufferDesc, &indexData, &pIBO);
	CHECK_D3D_ERROR(hr);

	//2.4 为索引缓冲区设置 CPU 描述符handle，分配到管道
	m_pContext->IASetIndexBuffer(pIBO, DXGI_FORMAT_R16_UINT, 0);
	//end *************************************************/

	//3.创建 vertex shader
	ID3D11VertexShader* pVertexShader;
	ID3DBlob* pBlob;//存储shader中的内容
	hr = D3DReadFileToBlob(L"../bin/vs.cso", &pBlob);
	CHECK_D3D_ERROR(hr);
	hr = m_pDevice->CreateVertexShader(pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(), nullptr, &pVertexShader);
	CHECK_D3D_ERROR(hr);

	//4.绑定 vertex shader 到渲染管线
	m_pContext->VSSetShader(pVertexShader, nullptr, 0);

	//5.告诉CPU如何从shader中读取数据
	ID3D11InputLayout* pInputLayout;
	//在 DirectX 代码中创建一个 InputLayout 来描述 input-assembler 阶段的数据。
	const D3D11_INPUT_ELEMENT_DESC layout[]{
		{"POSITIONT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, sizeof(float) * 3, D3D11_INPUT_PER_VERTEX_DATA, 0},

	};
	const UINT numElements = ARRAYSIZE(layout);
	hr = m_pDevice->CreateInputLayout(layout, numElements,
		pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pInputLayout);
	CHECK_D3D_ERROR(hr);

	//6.绑定 layout
	m_pContext->IASetInputLayout(pInputLayout);

	//end *************************************************/

	//start ***********************************************/
	struct ConstantBuffer
	{
		DirectX::XMMATRIX transform;
	};

	//常数缓存
	const ConstantBuffer cb{
	{
		DirectX::XMMatrixTranspose(
				DirectX::XMMatrixRotationZ(angle) *
				DirectX::XMMatrixRotationX(angle) *
				DirectX::XMMatrixTranslation(x,0.0f,z + 4.0f) *
				DirectX::XMMatrixPerspectiveLH(1.0f,3.0f / 4.0f,0.5f,10.0f)
		)
	}
	};


	//创建资源
	ID3D11Buffer *pCBO = nullptr;
	D3D11_BUFFER_DESC cbd;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;//设置成常数缓存标志位
	cbd.Usage = D3D11_USAGE_DYNAMIC;//需要每帧刷新
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.MiscFlags = 0;
	cbd.ByteWidth = sizeof(cb);
	cbd.StructureByteStride = 0;

	//常数缓存数据
	D3D11_SUBRESOURCE_DATA csData;
	csData.pSysMem = &cb;
	csData.SysMemPitch = 0;
	csData.SysMemSlicePitch = 0;

	m_pDevice->CreateBuffer(&cbd, &csData, &pCBO);

	//绑定管道
	m_pContext->VSSetConstantBuffers(0, 1, &pCBO);

	//end *************************************************/

	//start ***********************************************/
	//7.创建 pixel shader
	ID3D11PixelShader* pPixelShader;
	ID3DBlob* pBlob_PS;//存储shader中的内容
	hr = D3DReadFileToBlob(L"../bin/ps.cso", &pBlob_PS);
	CHECK_D3D_ERROR(hr);
	hr = m_pDevice->CreatePixelShader(pBlob_PS->GetBufferPointer(),
		pBlob_PS->GetBufferSize(), nullptr, &pPixelShader);
	CHECK_D3D_ERROR(hr);

	//8.绑定 pixel shader 到渲染管线
	m_pContext->PSSetShader(pPixelShader, nullptr, 0);
	//end *************************************************/

	//9.指定输出目标（渲染对象）
	//m_pContext->OMSetRenderTargets(1, &m_pRenderTargetView, nullptr);

	//三角形list
	m_pContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_pContext->DrawIndexed(indexCount, 0, 0);
}


int D3dClass::EndFrame()
{
	//翻转后缓存的画面
	HRESULT hr = S_FALSE;

	//Present(同步间隔，标签),屏幕刷新60hz的话，那么若游戏画面想要30hz，则间隔填入2，若能达到60，填入1
	hr = m_pSwap->Present(1, 0);
	CHECK_D3D_ERROR(hr);

	return 0;
}

int D3dClass::ClearBuffer(float r, float g, float b) noexcept
{
	const float color[] = { r, g, b, 1.0f };
	m_pContext->ClearRenderTargetView(m_pRenderTargetView, color);
	return 0;
}

int D3dClass::SetTex(WCHAR* filename)
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
	hr = m_pDevice->CreateSamplerState(&samplerDesc, &pSamplerState);
	CHECK_D3D_ERROR(hr);

	//从文件创建一个渲染视图：
	ID3D11ShaderResourceView *pShaderResourceView;
	ID3D11Resource* tex;
	hr = DirectX::CreateWICTextureFromFile(
		m_pDevice,
		filename,
		&tex, &pShaderResourceView);
	CHECK_D3D_ERROR(hr);

	//为立即上下文对象赋值渲染视图和采样器：
	m_pContext->PSSetShaderResources(0, 1, &pShaderResourceView);
	m_pContext->PSSetSamplers(0, 1, &pSamplerState);

	return 0;
}

void D3dClass::DrawScene()
{
	m_pContext->DrawIndexed(m_indexCount, 0, 0);
}

void D3dClass::InitTriangle()
{
	// 设置三角形顶点
	VertexPosColor vertices[] =
	{
		{DirectX::XMFLOAT3(-0.1f, -0.1f, 0.3f),{255, 0, 0, 1}},
		{DirectX::XMFLOAT3(-0.1f, 0.1f, 0.3f),{0, 255, 0, 1} },
		{DirectX::XMFLOAT3(0.1f, -0.1f, 0.3f),{0, 0, 255, 1} },
	};

	//IBO
	unsigned short indices[] = { 0,1,2};

	InitResourceEx(vertices, 3, indices, 3);
}

void D3dClass::InitRect()
{
	// 设置三角形顶点
	VertexPosColor vertices[] =
	{
		{DirectX::XMFLOAT3(-0.1f, -0.1f, 0.3f),{255, 0, 0, 1}},
		{DirectX::XMFLOAT3(-0.1f, 0.1f, 0.3f),{0, 255, 0, 1} },
		{DirectX::XMFLOAT3(0.1f, -0.1f, 0.3f),{0, 0, 255, 1} },
		{DirectX::XMFLOAT3(0.1f, 0.1f, 0.3f), {255, 0, 0, 1} },
	};

	//IBO
	unsigned short indices[] = { 0,1,2,
								 1,3,2 };
	InitResourceEx(vertices, 4, indices, 6);
}

void D3dClass::InitRectWithTex()
{
	// 设置三角形顶点
	VertexPosTex vertices[] =
	{
		//{DirectX::XMFLOAT3(-0.1f, -0.1f, 0.3f),DirectX::XMFLOAT2(0.0f, 1.0f) },
		//{DirectX::XMFLOAT3(-0.1f, 0.1f, 0.3f), DirectX::XMFLOAT2(0.0f, 0.0f) },
		//{DirectX::XMFLOAT3(0.1f, -0.1f, 0.3f), DirectX::XMFLOAT2(1.0f, 1.0f) },
		//{DirectX::XMFLOAT3(0.1f, 0.1f, 0.3f),  DirectX::XMFLOAT2(1.0f, 0.0f) },
		{ DirectX::XMFLOAT3(0.0f, 0.5f, 0.3f) , DirectX::XMFLOAT2(0.0f,1.0f) },
		{ DirectX::XMFLOAT3(0.5f, -0.5f, 0.3f) , DirectX::XMFLOAT2(1.0f,-1.0f) },
		{ DirectX::XMFLOAT3(-0.5f, -0.5f, 0.3f) , DirectX::XMFLOAT2(-1.0f,1.0f) }
	};

	//IBO
	unsigned short indices[] = { 0,1,2, };
								 //1,3,2 };
	InitResourceTex(vertices, 3, indices, 3);
}

void D3dClass::UpdateRectRotate(float angle)
{
	const ConstantBuffer cb{
	{
	DirectX::XMMatrixTranspose(
		DirectX::XMMatrixRotationZ(angle) *
		DirectX::XMMatrixScaling(m_screenHeight / m_screenWidth, 1.0f, 1.0f)
	),
	}
	};

	// 更新常量缓冲区，让立方体转起来
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(m_pContext->Map(m_pCBO, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(cb), &cb, sizeof(cb));
	m_pContext->Unmap(m_pCBO, 0);
}

int D3dClass::InitResourceTex(VertexPosTex* vertices, int verLen, unsigned short* indices, int indicesLen)
{
	if (vertices == nullptr || indices == nullptr)
		return -1;

	m_indexCount = indicesLen;

	//VBO-start ***********************************************/
	//1.1 顶点缓冲描述
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;		// 默认使用
	vertexBufferDesc.ByteWidth = sizeof(vertices[0])*verLen;	// 大小（我们有三个顶点）
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;	// Bind

	//1.2 顶点数据
	D3D11_SUBRESOURCE_DATA vsData;
	ZeroMemory(&vsData, sizeof(D3D11_SUBRESOURCE_DATA));
	vsData.pSysMem = vertices;

	//1.3 创建顶点缓冲区
	HR(m_pDevice->CreateBuffer(&vertexBufferDesc, &vsData, &m_pVBO));
	//VBO-end *************************************************/

		//IBO-start ***********************************************/
	// 设置索引缓冲区描述
	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.ByteWidth = sizeof(indices[0])*indicesLen;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;

	// 新建索引缓冲区
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	HR(m_pDevice->CreateBuffer(&indexBufferDesc, &indexData, &m_pIBO));
	//IBO-end *************************************************/


	//纹理*******************************************/
	//从文件创建一个渲染视图：
	m_pShaderResourceView = nullptr;
	ID3D11Resource* tex;
	HR(DirectX::CreateWICTextureFromFile(
		this->m_pDevice,
		L"../data/cheese.jpg",
		&tex, &m_pShaderResourceView));

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
	m_pSamplerState = nullptr;
	HR(m_pDevice->CreateSamplerState(&samplerDesc, &m_pSamplerState));
	//纹理 end***************************************/

	// ******************
	// 给渲染管线各个阶段绑定好所需资源
	//

	// 输入装配阶段的顶点缓冲区设置
	UINT stride = sizeof(VertexPosColor);	// 跨越字节数
	UINT offset = 0;						// 起始偏移量

	m_pContext->IASetVertexBuffers(0, 1, &m_pVBO, &stride, &offset);

	//2.4 为索引缓冲区设置 CPU 描述符handle，分配到管道
	m_pContext->IASetIndexBuffer(m_pIBO, DXGI_FORMAT_R16_UINT, 0);

	//绑定管道
	m_pContext->VSSetConstantBuffers(0, 1, &m_pIBO);

	// 设置图元类型，设定输入布局
	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pContext->IASetInputLayout(m_pInputLayout);

	//纹理-为立即上下文对象赋值渲染视图和采样器：
	m_pContext->PSSetShaderResources(0, 1, &m_pShaderResourceView);
	m_pContext->PSSetSamplers(0, 1, &m_pSamplerState);


	// 将着色器绑定到渲染管线
	m_pContext->VSSetShader(m_pVertexShader, nullptr, 0);
	m_pContext->PSSetShader(m_pPixelShader, nullptr, 0);
	return 0;
}

int D3dClass::InitResourceEx(VertexPosColor* vertices, int verLen, unsigned short* indices, int indicesLen)
{
	if (vertices == nullptr || indices == nullptr)
		return -1;

	m_indexCount = indicesLen;

	//VBO-start ***********************************************/
	//1.1 顶点缓冲描述
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;		// 默认使用
	vertexBufferDesc.ByteWidth = sizeof(vertices[0])*verLen;	// 大小（我们有三个顶点）
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;	// Bind

	//1.2 顶点数据
	D3D11_SUBRESOURCE_DATA vsData;
	ZeroMemory(&vsData, sizeof(D3D11_SUBRESOURCE_DATA));
	vsData.pSysMem = vertices;

	//1.3 创建顶点缓冲区
	HR(m_pDevice->CreateBuffer(&vertexBufferDesc, &vsData, &m_pVBO));
	//VBO-end *************************************************/

	//IBO-start ***********************************************/
	// 设置索引缓冲区描述
	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.ByteWidth = sizeof(indices[0])*indicesLen;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;

	// 新建索引缓冲区
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	HR(m_pDevice->CreateBuffer(&indexBufferDesc, &indexData, &m_pIBO));
	//IBO-end *************************************************/

	//设置常量缓冲区描述
	//创建资源
	m_pCBO = nullptr;
	D3D11_BUFFER_DESC cbd;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;//设置成常数缓存标志位
	cbd.Usage = D3D11_USAGE_DYNAMIC;//需要每帧刷新
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.MiscFlags = 0;
	cbd.ByteWidth = sizeof(ConstantBuffer);
	cbd.StructureByteStride = 0;

	//常数缓存数据-需要实时更新，不在此更新，暂时为nullptr
	m_pDevice->CreateBuffer(&cbd, nullptr, &m_pCBO);

	// ******************
	// 给渲染管线各个阶段绑定好所需资源
	//

	// 输入装配阶段的顶点缓冲区设置
	UINT stride = sizeof(VertexPosColor);	// 跨越字节数
	UINT offset = 0;						// 起始偏移量

	m_pContext->IASetVertexBuffers(0, 1, &m_pVBO, &stride, &offset);

	//2.4 为索引缓冲区设置 CPU 描述符handle，分配到管道
	m_pContext->IASetIndexBuffer(m_pIBO, DXGI_FORMAT_R16_UINT, 0);

	//绑定管道
	m_pContext->VSSetConstantBuffers(0, 1, &m_pIBO);

	// 设置图元类型，设定输入布局
	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pContext->IASetInputLayout(m_pInputLayout);
	// 将着色器绑定到渲染管线
	m_pContext->VSSetShader(m_pVertexShader, nullptr, 0);
	m_pContext->PSSetShader(m_pPixelShader, nullptr, 0);

	return 0;
}

int D3dClass::InitResource()
{
	// 设置三角形顶点
	VertexPosColor vertices[] =
	{
		{DirectX::XMFLOAT3(-0.1f, -0.1f, 0.3f),{255, 0, 0, 1}},
		{DirectX::XMFLOAT3(-0.1f, 0.1f, 0.3f),{0, 255, 0, 1} },
		{DirectX::XMFLOAT3(0.1f, -0.1f, 0.3f),{0, 0, 255, 1} },
		{DirectX::XMFLOAT3(0.1f, 0.1f, 0.3f), {255, 0, 0, 1} },
	};

	//IBO
	const unsigned short indices[] = { 0,1,2,
									   1,3,2 };
	m_indexCount = sizeof(indices) / sizeof(unsigned short);

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
	HR(m_pDevice->CreateBuffer(&vertexBufferDesc, &vsData, &m_pVBO));
	//VBO-end *************************************************/

	//IBO-start ***********************************************/
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

	HR(m_pDevice->CreateBuffer(&indexBufferDesc, &indexData, &m_pIBO));
	//IBO-end *************************************************/

	// ******************
	// 给渲染管线各个阶段绑定好所需资源
	//

	// 输入装配阶段的顶点缓冲区设置
	UINT stride = sizeof(VertexPosColor);	// 跨越字节数
	UINT offset = 0;						// 起始偏移量

	m_pContext->IASetVertexBuffers(0, 1, &m_pVBO, &stride, &offset);

	//2.4 为索引缓冲区设置 CPU 描述符handle，分配到管道
	m_pContext->IASetIndexBuffer(m_pIBO, DXGI_FORMAT_R16_UINT, 0);

	// 设置图元类型，设定输入布局
	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pContext->IASetInputLayout(m_pInputLayout);
	// 将着色器绑定到渲染管线
	m_pContext->VSSetShader(m_pVertexShader, nullptr, 0);
	m_pContext->PSSetShader(m_pPixelShader, nullptr, 0);

	return 0;
}

const D3D11_INPUT_ELEMENT_DESC D3dClass::VertexPosColor::inputLayout[2] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

int D3dClass::InitShader(WCHAR* vsCso, WCHAR* psCso)
{
	ID3DBlob* pBlob;
	m_pVertexShader = nullptr;
	m_pInputLayout = nullptr;
	m_pPixelShader = nullptr;

	// 1.创建顶点着色器
	HR(D3DReadFileToBlob(vsCso, &pBlob));
	HR(m_pDevice->CreateVertexShader(pBlob->GetBufferPointer(),pBlob->GetBufferSize(), nullptr, &m_pVertexShader));

	// 2.创建并绑定顶点布局
	const UINT numElements = ARRAYSIZE(VertexPosColor::inputLayout);
	HR(m_pDevice->CreateInputLayout(VertexPosColor::inputLayout, numElements,
		pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &m_pInputLayout));

	// 3.创建像素着色器
	HR(D3DReadFileToBlob(psCso, &pBlob));
	HR(m_pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pPixelShader));

	return 0;
}



const D3D11_INPUT_ELEMENT_DESC D3dClass::VertexPosTex::inputLayout[2] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(float)*3, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

int D3dClass::InitShaderTex(WCHAR* vsCso, WCHAR* psCso)
{
	ID3DBlob* pBlob;
	m_pVertexShader = nullptr;
	m_pInputLayout = nullptr;
	m_pPixelShader = nullptr;

	// 1.创建顶点着色器
	HR(D3DReadFileToBlob(vsCso, &pBlob));
	HR(m_pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pVertexShader));

	// 2.创建并绑定顶点布局
	const UINT numElements = ARRAYSIZE(VertexPosTex::inputLayout);
	HR(m_pDevice->CreateInputLayout(VertexPosTex::inputLayout, numElements,
		pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &m_pInputLayout));

	// 3.创建像素着色器
	HR(D3DReadFileToBlob(psCso, &pBlob));
	HR(m_pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pPixelShader));
	return 0;
}

int D3dClass::InitD3d11(HWND hwnd, int screenWidth, int screenHeight)
{
	//D3d设备初始化
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	HRESULT hr = S_FALSE;

	//1.创建设备
	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,//默认驱动类型，用于硬件3d加速
		D3D_DRIVER_TYPE_WARP, //快速的软件驱动程序，旧显卡适配更好，可能不支持某些特性
		D3D_DRIVER_TYPE_REFERENCE,//软件驱动类型，完美使用d3d特性而不报错，可用于测试
	};
	const auto numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,//d10兼容
		D3D_FEATURE_LEVEL_10_0,
	};
	const auto numFeatureLevels = ARRAYSIZE(featureLevels);

	
	D3D_FEATURE_LEVEL pD3DFeatureLevel;
	UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;//添加调试信息
#endif
	for (auto driverType : driverTypes) {
		hr = D3D11CreateDevice(
			nullptr,//传递NULL以使用默认适配器,若有多个显卡，可以指定一个
			driverType,//驱动程序类型
			nullptr,//驱动程序选择默认
			createDeviceFlags,
			featureLevels,//D3D特性级别
			numFeatureLevels,
			D3D11_SDK_VERSION,
			&m_pDevice,
			&pD3DFeatureLevel,
			&m_pContext);

		if (hr == E_INVALIDARG)
		{
			// Direct3D 11.0 的API不承认D3D_FEATURE_LEVEL_11_1，所以我们需要尝试特性等级11.0以及以下的版本
			hr = D3D11CreateDevice(nullptr, driverType, nullptr, createDeviceFlags,
				&featureLevels[1], numFeatureLevels - 1,
				D3D11_SDK_VERSION, &m_pDevice, &pD3DFeatureLevel, &m_pContext);
		}
		if (SUCCEEDED(hr))
			break;
	}
	CHECK_D3D_ERROR(hr);

	// 检测是否支持特性等级11.0或11.1
	if (pD3DFeatureLevel != D3D_FEATURE_LEVEL_11_0 && pD3DFeatureLevel != D3D_FEATURE_LEVEL_11_1)
	{
		MessageBox(0, L"Direct3D Feature Level 11 unsupported.", 0, 0);
		return -1;
	}
	//-------------------------------------------------

	// 2.配置交换链
	IDXGIDevice* dxgiDevice = nullptr;
	IDXGIAdapter* dxgiAdapter = nullptr;
	IDXGIFactory1* dxgiFactory1 = nullptr;	// D3D11.0(包含DXGI1.1)的接口类
	IDXGIFactory2* dxgiFactory2 = nullptr;	// D3D11.1(包含DXGI1.2)特有的接口类

	// 为了正确创建 DXGI交换链，首先我们需要获取创建 D3D设备 的 DXGI工厂，否则会引发报错：
	// "IDXGIFactory::CreateSwapChain: This function is being called with a device from a different IDXGIFactory."
	HR(m_pDevice->QueryInterface(__uuidof(IDXGIDevice), (void **)&dxgiDevice));//查询间接继承的接口
	HR(dxgiDevice->GetAdapter(&dxgiAdapter));
	HR(dxgiAdapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory1)));

	// 查看该对象是否包含IDXGIFactory2接口
	hr = dxgiFactory1->QueryInterface(__uuidof(IDXGIFactory2), (void **)&dxgiFactory2);
	// 如果包含，则说明支持D3D11.1
	if (dxgiFactory2 != nullptr)
	{
		HR(m_pDevice->QueryInterface(__uuidof(ID3D11Device1), (void **)&m_pDevice1));
		HR(m_pContext->QueryInterface(__uuidof(ID3D11DeviceContext1), (void **)&m_pContext1));
		// 填充各种结构体用以描述交换链
		DXGI_SWAP_CHAIN_DESC1 sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.Width = m_screenWidth;
		sd.Height = m_screenHeight;
		sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		sd.Flags = 0;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fd;
		fd.RefreshRate.Numerator = 0;//刷新率 60;
		fd.RefreshRate.Denominator = 1;
		fd.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		fd.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		fd.Windowed = TRUE;
		// 为当前窗口创建交换链
		HR(dxgiFactory2->CreateSwapChainForHwnd(m_pDevice, hwnd, &sd, &fd, nullptr, &m_pSwap1));
		HR(m_pSwap1->QueryInterface(__uuidof(IDXGISwapChain1), (void **)&m_pSwap));
	}
	else
	{
		// 填充DXGI_SWAP_CHAIN_DESC用以描述交换链
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferDesc.Width = m_screenWidth;
		sd.BufferDesc.Height = m_screenHeight;
		sd.BufferDesc.RefreshRate.Numerator = 0;//刷新率
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;
		sd.OutputWindow = hwnd;
		sd.Windowed = TRUE;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		sd.Flags = 0;
		HR(dxgiFactory1->CreateSwapChain(m_pDevice, &sd, &m_pSwap));
	}

	//3.渲染目标视图（Render Target View）,先读取纹理，然后用纹理创建渲染目标视图
	//3.1 获取交换链的后缓存
	m_pRenderTargetView = nullptr;
	ID3D11Resource* pId3D11Texture2D = nullptr;
	HR(m_pSwap->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&pId3D11Texture2D)));//COM queryInterface

	// CreateRenderTargetView 方法的第二个参数为 D3D11_RENDERTARGETVIEW_DESC 结构，此处使用默认，即nullptr
	HR(hr = m_pDevice->CreateRenderTargetView(pId3D11Texture2D, nullptr, &m_pRenderTargetView));

	pId3D11Texture2D->Release();


	//3.2.指定输出目标（渲染对象）
	m_pContext->OMSetRenderTargets(1, &m_pRenderTargetView, nullptr);


	//4.设置视口变换
	m_ScreenViewport.TopLeftX = 0;
	m_ScreenViewport.TopLeftY = 0;
	m_ScreenViewport.Width = static_cast<float>(m_screenWidth);
	m_ScreenViewport.Height = static_cast<float>(m_screenHeight);
	m_ScreenViewport.MinDepth = 0.0f;
	m_ScreenViewport.MaxDepth = 1.0f;
	m_pContext->RSSetViewports(1, &m_ScreenViewport);

	return 0;
}
