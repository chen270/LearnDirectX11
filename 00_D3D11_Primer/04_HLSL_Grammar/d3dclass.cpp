﻿#include "d3dclass.h"
#include <stdio.h>
#include <d3dcompiler.h>
#include <direct.h>
#include "misc/DXTrace.h"
#include <math.h>
#include "shader/Triangle_VS.inc"
#include "shader/Triangle_PS.inc"
#include "misc/DirectXTex/DDSTextureLoader11.h"
#include "misc/DirectXTex/ScreenGrab.h"

D3dClass::D3dClass():m_vsync_enabled(false)
{
	char path[1024];
	_getcwd(path, 1024);
	printf("cwd path:%s\n", path);
}


D3dClass::~D3dClass()
{

}

void D3dClass::DrawTestCube(float angle, float x, float z)
{
	HRESULT hr;

	// create vertex buffer (1 2d triangle at center of screen)
	Vertex vertices[] =			// 顶点数组
	{
		{DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f),{255, 0, 0,  1} },
		{DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f),{0, 255, 0,  1} },
		{DirectX::XMFLOAT3(-1.0f,  1.0f, -1.0f),{0, 0, 255,  1} },
		{DirectX::XMFLOAT3(1.0f,  1.0f, -1.0f),{255, 255, 0,  1} },
		{DirectX::XMFLOAT3(-1.0f, -1.0f,  1.0f),{255, 0, 255,  1} },
		{DirectX::XMFLOAT3(1.0f, -1.0f,  1.0f),{0, 255, 255,  1} },
		{DirectX::XMFLOAT3(-1.0f,  1.0f,  1.0f),{0, 0, 0,  1} },
		{DirectX::XMFLOAT3(1.0f,  1.0f,  1.0f),{255, 255, 255,  1} },
	};
	ComPtr<ID3D11Buffer> pVertexBuffer;
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
	pContext->IASetVertexBuffers(0u, 1u, pVertexBuffer.GetAddressOf(), &stride, &offset);


	// create index buffer
	const unsigned short indices[] =
	{
		0,2,1, 2,3,1,
		1,3,5, 3,7,5,
		2,6,3, 3,6,7,
		4,5,7, 4,7,6,
		0,4,2, 2,4,6,
		0,1,4, 1,5,4
	};
	ComPtr<ID3D11Buffer> pIndexBuffer;
	D3D11_BUFFER_DESC ibd = {};
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.CPUAccessFlags = 0u;
	ibd.MiscFlags = 0u;
	ibd.ByteWidth = sizeof(indices);
	ibd.StructureByteStride = sizeof(unsigned short);
	D3D11_SUBRESOURCE_DATA isd = {};
	isd.pSysMem = indices;
	HR(pDevice->CreateBuffer(&ibd, &isd, &pIndexBuffer));

	// bind index buffer
	pContext->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);


	// create constant buffer for transformation matrix
	struct ConstantBuffer
	{
		DirectX::XMMATRIX transform;
	};
	const ConstantBuffer cb =
	{
		{
			DirectX::XMMatrixTranspose(
				DirectX::XMMatrixRotationZ(angle) *
				DirectX::XMMatrixRotationX(angle) *
				DirectX::XMMatrixTranslation(x,0.0f,z + 4.0f) *
				DirectX::XMMatrixPerspectiveLH(1.0f,3.0f / 4.0f,0.5f,10.0f)
			)
		}
	};
	ComPtr<ID3D11Buffer> pConstantBuffer;
	D3D11_BUFFER_DESC cbd;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.MiscFlags = 0u;
	cbd.ByteWidth = sizeof(cb);
	cbd.StructureByteStride = 0u;
	D3D11_SUBRESOURCE_DATA csd = {};
	csd.pSysMem = &cb;
	HR(pDevice->CreateBuffer(&cbd, &csd, &pConstantBuffer));

	// bind constant buffer to vertex shader
	pContext->VSSetConstantBuffers(0u, 1u, pConstantBuffer.GetAddressOf());

	// create pixel shader
	ComPtr<ID3D11PixelShader> pPixelShader;
	ComPtr<ID3DBlob> pBlob;
	HR(D3DReadFileToBlob(L"../bin/ps.cso", &pBlob));
	HR(pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader));

	// bind pixel shader
	pContext->PSSetShader(pPixelShader.Get(), nullptr, 0u);


	// create vertex shader
	ComPtr<ID3D11VertexShader> pVertexShader;
	HR(D3DReadFileToBlob(L"../bin/vs.cso", &pBlob));
	HR(pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShader));

	// bind vertex shader
	pContext->VSSetShader(pVertexShader.Get(), nullptr, 0u);


	// input (vertex) layout (2d position only)
	ComPtr<ID3D11InputLayout> pInputLayout;
	const D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{"POSITIONT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, sizeof(float) * 3, D3D11_INPUT_PER_VERTEX_DATA, 0},//UNORM归一化
	};
	HR(pDevice->CreateInputLayout(
		ied, (UINT)std::size(ied),
		pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(),
		&pInputLayout
	));

	// bind vertex layout
	pContext->IASetInputLayout(pInputLayout.Get());


	// Set primitive topology to triangle list (groups of 3 vertices)
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	// configure viewport
	D3D11_VIEWPORT vp;
	vp.Width = 800;
	vp.Height = 600;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pContext->RSSetViewports(1u, &vp);


	THROW_D3D_EXCEPTION(pContext->DrawIndexed((UINT)std::size(indices), 0u, 0u));
}



int D3dClass::DrawCube(float angle, float x, float z)
{
	//DrawTestTriangleErr();

	HRESULT hr = S_FALSE;
	Vertex vertices[] =			// 顶点数组
	{
		{DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f),{255, 0, 0,  1} },
		{DirectX::XMFLOAT3( 1.0f, -1.0f, -1.0f),{0, 255, 0,  1} },
		{DirectX::XMFLOAT3(-1.0f,  1.0f, -1.0f),{0, 0, 255,  1} },
		{DirectX::XMFLOAT3( 1.0f,  1.0f, -1.0f),{255, 255, 0,  1} },
		{DirectX::XMFLOAT3(-1.0f, -1.0f,  1.0f),{255, 0, 255,  1} },
		{DirectX::XMFLOAT3( 1.0f, -1.0f,  1.0f),{0, 255, 255,  1} },
		{DirectX::XMFLOAT3(-1.0f,  1.0f,  1.0f),{0, 0, 0,  1} },
		{DirectX::XMFLOAT3( 1.0f,  1.0f,  1.0f),{255, 255, 255,  1} },
	};

	const int vertexesNum = sizeof(vertices) / sizeof(Vertex);

	//IBO
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

	/************************************************************************/
	/* 顶点缓冲区  */
	/************************************************************************/

	//1.顶点缓冲描述
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;		// 默认使用
	vertexBufferDesc.ByteWidth = sizeof(vertices);	// 大小
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;	// Bind

	//2.顶点数据
	D3D11_SUBRESOURCE_DATA vsData;
	ZeroMemory(&vsData, sizeof(D3D11_SUBRESOURCE_DATA));
	vsData.pSysMem = vertices;

	//3.创建顶点缓冲区
	ComPtr<ID3D11Buffer> pVBO;
	HR(pDevice->CreateBuffer(&vertexBufferDesc, &vsData, pVBO.GetAddressOf()));

	//4.为顶点缓冲区设置 CPU 描述符handle，分配到管道
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	pContext->IASetVertexBuffers(0, 1, pVBO.GetAddressOf(), &stride, &offset);

	/************************************************************************/
	/* 索引缓冲区  */
	/************************************************************************/
	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;		// 默认使用
	indexBufferDesc.ByteWidth = sizeof(indices);	// 大小
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;	// Bind
	indexBufferDesc.StructureByteStride = sizeof(unsigned short);

	//顶点数据
	D3D11_SUBRESOURCE_DATA indexData;
	ZeroMemory(&indexData, sizeof(D3D11_SUBRESOURCE_DATA));
	indexData.pSysMem = indices;

	ComPtr<ID3D11Buffer> pIBO;
	HR(pDevice->CreateBuffer(&indexBufferDesc, &indexData, pIBO.GetAddressOf()));

	// bind index buffer
	pContext->IASetIndexBuffer(pIBO.Get(), DXGI_FORMAT_R16_UINT, 0u);
	

	/************************************************************************/
	/* 常量缓冲区  */
	/************************************************************************/
	struct ConstantBuffer
	{
		DirectX::XMMATRIX transform;//无法直接访问元素来初始化和改变元素，类似于黑匣子
	};


	float ratio_hw = (float)m_screenHeight / (float)m_screenWidth;

	//float x = 0, z = 0;

	const ConstantBuffer cb =
	{
		{
			DirectX::XMMatrixTranspose(
				DirectX::XMMatrixRotationZ(angle)*
				DirectX::XMMatrixRotationX(angle)*
				DirectX::XMMatrixTranslation(x,0.0f,z + 4.0f)*
				DirectX::XMMatrixPerspectiveLH(1.0f,ratio_hw,0.5f,10.0f) )
		}
	};
	ComPtr<ID3D11Buffer> pConstantBuffer;
	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DYNAMIC; //每帧都要更新，需要动态缓冲
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.ByteWidth = sizeof(cb);	// 大小
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;	// Bind
	cbd.MiscFlags = 0;
	cbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA csd;
	csd.pSysMem = &cb;
	HR(pDevice->CreateBuffer(&cbd, &csd, pConstantBuffer.GetAddressOf()));

	//绑定
	pContext->VSSetConstantBuffers(0, 1, pConstantBuffer.GetAddressOf());

	//不需要进行描述， 即不需要绑定 ID3D11InputLayout
	//create constant buffer end----------------------


	//5.创建 vertex shader
	ComPtr<ID3D11VertexShader> pVertexShader;
	ComPtr<ID3DBlob> pBlob;//存储shader中的内容
	HR(D3DReadFileToBlob(L"../bin/vs.cso", pBlob.GetAddressOf()));

	hr = pDevice->CreateVertexShader(pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(), nullptr, pVertexShader.GetAddressOf());
	HR(hr);

	//6.绑定 vertex shader 到渲染管线
	pContext->VSSetShader(pVertexShader.Get(), nullptr, 0);

	//7.告诉CPU如何从shader中读取数据
	ComPtr<ID3D11InputLayout> pInputLayout;
	//在 DirectX 代码中创建一个 InputLayout 来描述 input-assembler 阶段的数据。
	const D3D11_INPUT_ELEMENT_DESC layout[]{
		{"POSITIONT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, sizeof(float) * 3, D3D11_INPUT_PER_VERTEX_DATA, 0},//UNORM归一化
	};
	const UINT numElements = ARRAYSIZE(layout);
	hr = pDevice->CreateInputLayout(layout, numElements,
		pBlob->GetBufferPointer(), pBlob->GetBufferSize(), pInputLayout.GetAddressOf());
	HR(hr);

	//8.绑定 layout
	pContext->IASetInputLayout(pInputLayout.Get());

	//end *************************************************/




	//start ***********************************************/
	//9.创建 pixel shader
	ComPtr<ID3D11PixelShader> pPixelShader;
	ComPtr<ID3DBlob> pBlob_PS;//存储shader中的内容
	hr = D3DReadFileToBlob(L"../bin/ps.cso", pBlob_PS.GetAddressOf());
	HR(hr);
	hr = pDevice->CreatePixelShader(pBlob_PS->GetBufferPointer(),
		pBlob_PS->GetBufferSize(), nullptr, pPixelShader.GetAddressOf());
	HR(hr);

	//10.绑定 pixel shader 到渲染管线
	pContext->PSSetShader(pPixelShader.Get(), nullptr, 0);
	//end *************************************************/

	//11.指定输出目标（渲染对象）
	pContext->OMSetRenderTargets(1, pRenderTargetView.GetAddressOf(), pDepthStencilView.Get());//在初始化时，已经指定了输出目标，所以不需要指定

	//三角形list
	pContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);//画三角形
	//pContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST);//画线


	//12.设置视口变换
	D3D11_VIEWPORT vp;// 视口
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = static_cast<float>(m_screenWidth);
	vp.Height = static_cast<float>(m_screenHeight);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	pContext->RSSetViewports(1, &vp);

	THROW_D3D_EXCEPTION(pContext->DrawIndexed((UINT)indexCount, 0u, 0u));//三个顶点，从0号顶点开始
	return 0;
}

int D3dClass::DrawTriangle(float angle)
{
	//DrawTestTriangleErr();

	HRESULT hr = S_FALSE;
	Vertex vertices[] =			// 顶点数组
	{
		{DirectX::XMFLOAT3(0.0f, 0.3f, 0.3f),{255, 0, 0,  1} },
		{DirectX::XMFLOAT3(0.3f, -0.3f, 0.3f),{0, 255, 0, 1} },
		{DirectX::XMFLOAT3(-0.3f, -0.3f, 0.3f),{0, 0, 255, 1} },
	};

	const int vertexesNum = sizeof(vertices) / sizeof(Vertex);

	//start ***********************************************/
	//1.顶点缓冲描述
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;		// 默认使用
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * vertexesNum;	// 大小（我们有三个顶点）
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;	// Bind

	//2.顶点数据
	D3D11_SUBRESOURCE_DATA vsData;
	ZeroMemory(&vsData, sizeof(D3D11_SUBRESOURCE_DATA));
	vsData.pSysMem = vertices;

	//3.创建顶点缓冲区
	ComPtr<ID3D11Buffer> pVBO;
	HR( pDevice->CreateBuffer(&vertexBufferDesc, &vsData, pVBO.GetAddressOf()) );

	//4.为顶点缓冲区设置 CPU 描述符handle，分配到管道
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	pContext->IASetVertexBuffers(0, 1, pVBO.GetAddressOf(), &stride, &offset);

	//5.创建 vertex shader
	ComPtr<ID3D11VertexShader> pVertexShader;
	ComPtr<ID3DBlob> pBlob;//存储shader中的内容
	HR( D3DReadFileToBlob(L"../bin/vs.cso", pBlob.GetAddressOf()) );

	hr = pDevice->CreateVertexShader(pBlob->GetBufferPointer(), 
		pBlob->GetBufferSize(), nullptr, pVertexShader.GetAddressOf());
	HR(hr);

	//6.绑定 vertex shader 到渲染管线
	pContext->VSSetShader(pVertexShader.Get(), nullptr, 0);

	//7.告诉CPU如何从shader中读取数据
	ComPtr<ID3D11InputLayout> pInputLayout;
	//在 DirectX 代码中创建一个 InputLayout 来描述 input-assembler 阶段的数据。
	const D3D11_INPUT_ELEMENT_DESC layout[]{
		{"POSITIONT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, sizeof(float)*3, D3D11_INPUT_PER_VERTEX_DATA, 0},//UNORM归一化
	};
	const UINT numElements = ARRAYSIZE(layout);
	hr = pDevice->CreateInputLayout(layout, numElements, 
		pBlob->GetBufferPointer(), pBlob->GetBufferSize(), pInputLayout.GetAddressOf());
	HR(hr);

	//8.绑定 layout
	pContext->IASetInputLayout(pInputLayout.Get());

	//end *************************************************/


	//create constant buffer start--------------------
	struct ConstantBuffer
	{
		DirectX::XMMATRIX transform;//无法直接访问元素来初始化和改变元素，类似于黑匣子
	};


	float ratio_hw = (float)m_screenHeight / (float)m_screenWidth;
	
	const ConstantBuffer cb =
	{
		{
#if 0
			DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationZ(angle), DirectX::XMMatrixScaling(ratio_hw,1.0f,1.0f))
#else
			DirectX::XMMatrixRotationZ(angle)* DirectX::XMMatrixScaling(ratio_hw,1.0f,1.0f) //该库也重载了 *
#endif
		}
	};
	ComPtr<ID3D11Buffer> pConstantBuffer;
	D3D11_BUFFER_DESC cbd;
	ZeroMemory(&cbd, sizeof(cbd));
	cbd.Usage = D3D11_USAGE_DYNAMIC; //每帧都要更新，需要动态缓冲
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.ByteWidth = sizeof(cb);	// 大小
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;	// Bind
	cbd.MiscFlags = 0;
	cbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA csd;
	csd.pSysMem = &cb;
	HR( pDevice->CreateBuffer(&cbd, &csd, pConstantBuffer.GetAddressOf()) );

	//绑定
	pContext->VSSetConstantBuffers(0, 1, pConstantBuffer.GetAddressOf());

	//不需要进行描述， 即不需要绑定 ID3D11InputLayout
	//create constant buffer end----------------------



	//start ***********************************************/
	//9.创建 pixel shader
	ComPtr<ID3D11PixelShader> pPixelShader;
	ComPtr<ID3DBlob> pBlob_PS;//存储shader中的内容
	hr = D3DReadFileToBlob(L"../bin/ps.cso", pBlob_PS.GetAddressOf());
	HR(hr);
	hr = pDevice->CreatePixelShader(pBlob_PS->GetBufferPointer(),
		pBlob_PS->GetBufferSize(), nullptr, pPixelShader.GetAddressOf());
	HR(hr);

	//10.绑定 pixel shader 到渲染管线
	pContext->PSSetShader(pPixelShader.Get(), nullptr, 0);
	//end *************************************************/

	//11.指定输出目标（渲染对象）
	pContext->OMSetRenderTargets(1, pRenderTargetView.GetAddressOf(), nullptr);

	//三角形list
	pContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);//画三角形
	//pContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_LINELIST);//画线


	//12.设置视口变换
	D3D11_VIEWPORT vp;// 视口
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = static_cast<float>(m_screenWidth);
	vp.Height = static_cast<float>(m_screenHeight);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	pContext->RSSetViewports(1, &vp);

	THROW_D3D_EXCEPTION(pContext->Draw(vertexesNum, 0));//三个顶点，从0号顶点开始
	return 0;
}

int D3dClass::DrawRect()
{
	HRESULT hr = S_FALSE;
	Vertex vertices[] =			// 顶点数组
	{
		{DirectX::XMFLOAT3(-0.1f, -0.1f, 0.3f),{255, 0, 0, 1}},
		{DirectX::XMFLOAT3(-0.1f, 0.1f, 0.3f),{0, 255, 0, 1} },
		{DirectX::XMFLOAT3(0.1f, -0.1f, 0.3f),{0, 0, 255, 1} },
		{DirectX::XMFLOAT3(0.1f, 0.1f, 0.3f), {255, 0, 0, 1} },
	};

	//IBO
	const int indexCount = 6;
	const unsigned short indices[indexCount] = {0,1,2,
												1,3,2};
	//const int indexCount = 3;
	//unsigned long indices[indexCount] = { 0,1,2 };


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
	ComPtr<ID3D11Buffer> pVBO;
	hr = pDevice->CreateBuffer(&vertexBufferDesc, &vsData, pVBO.GetAddressOf());
	HR(hr);

	//1.4 为顶点缓冲区设置 CPU 描述符handle，分配到管道
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	pContext->IASetVertexBuffers(0, 1, pVBO.GetAddressOf(), &stride, &offset);
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
	ComPtr<ID3D11Buffer> pIBO = nullptr;
	hr = pDevice->CreateBuffer(&indexBufferDesc, &indexData, pIBO.GetAddressOf());
	HR(hr);

	//2.4 为索引缓冲区设置 CPU 描述符handle，分配到管道
	pContext->IASetIndexBuffer(pIBO.Get(), DXGI_FORMAT_R16_UINT, 0);
	//end *************************************************/

	//3.创建 vertex shader
	ComPtr<ID3D11VertexShader> pVertexShader;
	ID3DBlob* pBlob;//存储shader中的内容
	hr = D3DReadFileToBlob(L"../bin/vs.cso", &pBlob);
	HR(hr);
	hr = pDevice->CreateVertexShader(pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(), nullptr, pVertexShader.GetAddressOf());
	HR(hr);

	//4.绑定 vertex shader 到渲染管线
	pContext->VSSetShader(pVertexShader.Get(), nullptr, 0);

	//5.告诉CPU如何从shader中读取数据
	ComPtr<ID3D11InputLayout> pInputLayout;
	//在 DirectX 代码中创建一个 InputLayout 来描述 input-assembler 阶段的数据。
	const D3D11_INPUT_ELEMENT_DESC layout[]{
		{"POSITIONT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, sizeof(float) * 3, D3D11_INPUT_PER_VERTEX_DATA, 0},

	};
	const UINT numElements = ARRAYSIZE(layout);
	hr = pDevice->CreateInputLayout(layout, numElements,
		pBlob->GetBufferPointer(), pBlob->GetBufferSize(), pInputLayout.GetAddressOf());
	HR(hr);

	//6.绑定 layout
	pContext->IASetInputLayout(pInputLayout.Get());

	//end *************************************************/


	//start ***********************************************/
	//7.创建 pixel shader
	ComPtr<ID3D11PixelShader> pPixelShader;
	ComPtr<ID3DBlob> pBlob_PS;//存储shader中的内容
	HR( D3DReadFileToBlob(L"../bin/ps.cso", pBlob_PS.GetAddressOf()));

	hr = pDevice->CreatePixelShader(pBlob_PS->GetBufferPointer(),
		pBlob_PS->GetBufferSize(), nullptr, pPixelShader.GetAddressOf());
	HR(hr);

	//8.绑定 pixel shader 到渲染管线
	pContext->PSSetShader(pPixelShader.Get(), nullptr, 0);
	//end *************************************************/

	//9.指定输出目标（渲染对象）
	pContext->OMSetRenderTargets(1, pRenderTargetView.GetAddressOf(), nullptr);

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

	THROW_D3D_EXCEPTION(pContext->DrawIndexed(indexCount, 0, 0));
	return 0;
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


void D3dClass::InitShader_INC()
{
	// 创建顶点着色器
	HR(pDevice->CreateVertexShader(g_Triangle_VS, sizeof(g_Triangle_VS), nullptr, pVertexShader.GetAddressOf()));

	// 创建并绑定顶点布局
	ComPtr<ID3D11InputLayout> pInputLayout;
	HR(pDevice->CreateInputLayout(inputLayout, ARRAYSIZE(inputLayout),
		g_Triangle_VS, sizeof(g_Triangle_VS), pInputLayout.GetAddressOf()));

	pContext->IASetInputLayout(pInputLayout.Get());   // [In]输入布局

	//4.像素着色器
	//ComPtr<ID3D11PixelShader> pPixelShader;
	HR(pDevice->CreatePixelShader(g_Triangle_PS,
		sizeof(g_Triangle_PS), nullptr, pPixelShader.GetAddressOf()));
}

void D3dClass::InitShader_CompileInRunTime()
{
	//用来查错的Blob对象
	ComPtr<ID3DBlob> pErrorMessage;

	//顶点着色器 Blob 对象和像素着色器 Blob 对象
	ComPtr<ID3DBlob>  pVertexShaderBlob;
	ComPtr<ID3DBlob>  pPixelShaderBlob;

	// 1.编译顶点着色器
	HR( D3DCompileFromFile(L"../shader/Triangle_VS.hlsl", nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0",
		D3DCOMPILER_STRIP_DEBUG_INFO, 0,
		pVertexShaderBlob.GetAddressOf(), pErrorMessage.ReleaseAndGetAddressOf()) );

	HR(D3DCompileFromFile(L"../shader/Triangle_PS.hlsl", nullptr,
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
	HR(pDevice->CreateInputLayout(inputLayout, ARRAYSIZE(inputLayout),
		pVertexShaderBlob->GetBufferPointer(), pVertexShaderBlob->GetBufferSize(), pInputLayout.GetAddressOf()));

	pContext->IASetInputLayout(pInputLayout.Get());   // [In]输入布局
}


void D3dClass::InitResource()
{

	InitShader_CompileInRunTime();

	using namespace DirectX;
	struct VertexPosColor
	{
		XMFLOAT3 pos;
		XMFLOAT4 color;
	};

	VertexPosColor vertices[] = {
	{ XMFLOAT3(0.0f, 0.5f, 0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
	{ XMFLOAT3(0.5f, -0.5f, 0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
	{ XMFLOAT3(-0.5f, -0.5f, 0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) }
	};

	// 1.顶点缓冲区
	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof vertices;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = vertices;

	ComPtr<ID3D11Buffer> m_pVertexBuffer;
	HR(pDevice->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.GetAddressOf()));

	UINT stride = sizeof(VertexPosColor);    // 跨越字节数
	UINT offset = 0;                        // 起始偏移量
	pContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &offset);

	//设置图元信息
	pContext->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);     // [In]图元类型

	//2.设置着色器
	pContext->VSSetShader(pVertexShader.Get(), nullptr, 0);
	pContext->PSSetShader(pPixelShader.Get(), nullptr, 0);

	return;
}

void D3dClass::DrawScene()
{
	assert(pContext);
	assert(pSwap);

	static float black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };    // RGBA = (0,0,0,255)
	pContext->ClearRenderTargetView(pRenderTargetView.Get(), black);
	pContext->ClearDepthStencilView(pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// 绘制三角形
	pContext->Draw(3, 0);
	HR(pSwap->Present(0, 0));
}

#define  NUM_ELEMENTS   2048
static struct BufType
{
	int i;
	float f;
} s_vBuf0[NUM_ELEMENTS], s_vBuf1[NUM_ELEMENTS];

static bool CreateComputeShader(LPCWSTR pSrcFile, LPCSTR pFunctionName,
	ID3D11Device* pDevice, ID3D11ComputeShader** ppShaderOut)
{
	uint32_t dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined(DEBUG) | defined(_DEBUG)
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	const D3D_SHADER_MACRO defines[] =
	{
		"USE_STRUCTURED_BUFFERS", "1",
		NULL, NULL
	};

	// We generally prefer to use the higher CS shader profile when possible as CS 5.0 is better performance on 11-class hardware
	ID3DBlob* pErrorBlob = NULL;
	ID3DBlob* computeShader = NULL;

	if (D3DCompileFromFile(pSrcFile, nullptr, NULL, pFunctionName, "cs_5_0", dwShaderFlags, 0,
		&computeShader, &pErrorBlob) < 0)
	{
		if (pErrorBlob != NULL)
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

		if (pErrorBlob != NULL)
			pErrorBlob->Release();
		if (computeShader != NULL)
			computeShader->Release();

		return false;
	}

	bool result = true;
	if (pDevice->CreateComputeShader(computeShader->GetBufferPointer(),
		computeShader->GetBufferSize(), NULL, ppShaderOut))
		result = false;

	if (pErrorBlob != NULL)
		pErrorBlob->Release();
	if (computeShader != NULL)
		computeShader->Release();

	return result;
}

static bool CreateStructureBuffer(ID3D11Device* pDevice, uint32_t elementSize, uint32_t uCount,
	void* pInitData, ID3D11Buffer** ppBufferOut)
{
	*ppBufferOut = NULL;

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	desc.ByteWidth = elementSize * uCount;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = elementSize;

	if (pInitData != NULL)
	{
		D3D11_SUBRESOURCE_DATA InitData = { 0 };
		InitData.pSysMem = pInitData;
		return pDevice->CreateBuffer(&desc, &InitData, ppBufferOut) >= 0;
	}
	else
		return pDevice->CreateBuffer(&desc, NULL, ppBufferOut) >= 0;
}

static bool CreateConstantBuffer(ID3D11Device* pDevice, uint32_t nBytes, void* pInitData, ID3D11Buffer** ppBufferOut)
{
	*ppBufferOut = NULL;

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.ByteWidth = nBytes;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = pInitData;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	return pDevice->CreateBuffer(&desc, &initData, ppBufferOut) >= 0;
}

/**
 利用ID3D11Device::CreateShaderResouceView()来创建GPU中Buffer的resourceView
*/
static bool CreateBufferSRV(ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11ShaderResourceView** ppSRVOut)
{
	D3D11_BUFFER_DESC descBuf;
	ZeroMemory(&descBuf, sizeof(descBuf));
	pBuffer->GetDesc(&descBuf);

	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	desc.BufferEx.FirstElement = 0;

	//假定这是个structure buffer
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.BufferEx.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;

	return pDevice->CreateShaderResourceView((ID3D11Resource*)pBuffer, &desc, ppSRVOut) >= 0;
}

static bool CreateBufferUAV(ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11UnorderedAccessView** ppUAVOut)
{
	D3D11_BUFFER_DESC descBuf;
	ZeroMemory(&descBuf, sizeof(descBuf));
	pBuffer->GetDesc(&descBuf);

	D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	desc.Buffer.FirstElement = 0;

	//假设这是一个structure buffer
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.Buffer.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;

	return pDevice->CreateUnorderedAccessView((ID3D11Resource*)pBuffer, &desc, ppUAVOut) >= 0;
}

static void RunComputeShader(ID3D11DeviceContext* pImmediateContext, ID3D11ComputeShader* pComputeShader,
	uint32_t nSRVs, uint32_t nUAVs, ID3D11ShaderResourceView* pShaderResourceViews[],
	ID3D11UnorderedAccessView* pUnorderedViews[], uint32_t X, uint32_t Y, uint32_t Z)
{
	pImmediateContext->CSSetShader(pComputeShader, NULL, 0);
	pImmediateContext->CSSetShaderResources(0, nSRVs, pShaderResourceViews);
	pImmediateContext->CSSetUnorderedAccessViews(0, nUAVs, pUnorderedViews, NULL);
	pImmediateContext->Dispatch(NUM_ELEMENTS, 1, 1);

	//清空Shader和各个Shader Resource View、Unordered Access View以及一些Constant Buffer
	pImmediateContext->CSSetShader(NULL, NULL, 0);

	ID3D11UnorderedAccessView* ppUAViewNULL[] = { NULL, NULL };
	pImmediateContext->CSSetUnorderedAccessViews(0, 2, ppUAViewNULL, NULL);

	ID3D11ShaderResourceView* ppSRVNULL[2] = { NULL,NULL };
	pImmediateContext->CSSetShaderResources(0, 2, ppSRVNULL);

	ID3D11Buffer* ppCBNULL[1] = { NULL };
	pImmediateContext->CSSetConstantBuffers(0, 1, ppCBNULL);
}

static ID3D11Buffer* CreateAndCopyToDebugBuf(ID3D11Device* pDevice, ID3D11DeviceContext* pd3dImmediateContext, ID3D11Buffer* pBuffer)
{
	ID3D11Buffer* debugBuf = NULL;
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	pBuffer->GetDesc(&desc);
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.MiscFlags = 0;

	if (pDevice->CreateBuffer(&desc, NULL, &debugBuf) >= 0)
	{
		pd3dImmediateContext->CopyResource((ID3D11Resource*)debugBuf,
			(ID3D11Resource*)pBuffer);
	}
	return debugBuf;
}


int D3dClass::UseComputeShader()
{
	ID3D11ComputeShader* computeShader = NULL;

	//各个Buffer指针变量
	ID3D11Buffer* srcBuffer0 = NULL;
	ID3D11Buffer* srcBuffer1 = NULL;
	ID3D11Buffer* resultBuffer = NULL;
	ID3D11Buffer* srcDstBuffer = NULL;
	ID3D11Buffer* constBuffer = NULL;

	//读写上面buffer的ID3D11ShaderResourceView和UnorderedAccessView接口
	ID3D11ShaderResourceView* srcBuf0SRV = NULL;
	ID3D11ShaderResourceView* srcBuf1SRV = NULL;
	ID3D11UnorderedAccessView* resBufUAV = NULL;
	ID3D11UnorderedAccessView* srcdstBufUAV = NULL;

	int localBuffer[NUM_ELEMENTS];
	for (int i = 0; i < NUM_ELEMENTS; i++)
		localBuffer[i] = i + 1;

	if (!CreateComputeShader(L"../shader/compute_CS.hlsl", "main", pDevice.Get(), &computeShader))
	{
		puts("CreateComputeShader failed!");
		return -1;
	}


	//初始化计算数据
	for (int i = 0; i < NUM_ELEMENTS; i++)
	{
		s_vBuf0[i].i = i;
		s_vBuf0[i].f = (float)i;

		s_vBuf1[i].i = i;
		s_vBuf1[i].f = (float)i;
	}

	//为CPU中的数组创建GPU中相应Buffer
	if (!CreateStructureBuffer(pDevice.Get(), sizeof(struct BufType), NUM_ELEMENTS, s_vBuf0, &srcBuffer0))
	{
		puts("create srcBuffer0 failed");
		return -1;
	}

	if (!CreateStructureBuffer(pDevice.Get(), sizeof(struct BufType), NUM_ELEMENTS, s_vBuf1, &srcBuffer1))
	{
		puts("create srcBuffer1 failed");
		return -1;
	}

	if (!CreateStructureBuffer(pDevice.Get(), sizeof(struct BufType), NUM_ELEMENTS, NULL, &resultBuffer))
	{
		puts("Create resultBuffer failed");
		return -1;
	}

	if (!CreateStructureBuffer(pDevice.Get(), sizeof(localBuffer[0]), _countof(localBuffer), localBuffer, &srcDstBuffer))
	{
		puts("create srcDstBuffer failed");
		return -1;
	}

	// 在D3D11中，常量缓存至少需要4个int元素
	int value[4] = { 10, 20 };
	if (!CreateConstantBuffer(pDevice.Get(), sizeof(value), value, &constBuffer))
	{
		puts("Create constBuffer failed!");
		return -1;
	}
	// 绑定常量缓存
	pContext->CSSetConstantBuffers(0, 1, &constBuffer);

	//为buffer创建相应的shader resource view与unordered access view
	if (!CreateBufferSRV(pDevice.Get(), srcBuffer0, &srcBuf0SRV))
	{
		puts("create srcBuf0SRV failed");
		return -1;
	}

	if (!CreateBufferSRV(pDevice.Get(), srcBuffer1, &srcBuf1SRV))
	{
		puts("create srcBuf1SRV  failed");
		return -1;
	}

	if (!CreateBufferUAV(pDevice.Get(), resultBuffer, &resBufUAV))
	{
		puts("create resBufUAV failed");
		return -1;
	}

	if (!CreateBufferUAV(pDevice.Get(), srcDstBuffer, &srcdstBufUAV))
	{
		puts("create srcdstBufUAV failed!");
		return -1;
	}

	ID3D11ShaderResourceView* shaderResourceViews[] = { srcBuf0SRV, srcBuf1SRV };
	ID3D11UnorderedAccessView* unorderedAccessViews[] = { resBufUAV, srcdstBufUAV };

	//运行Shader Compute程序
	RunComputeShader(pContext.Get(), computeShader, _countof(shaderResourceViews), _countof(unorderedAccessViews),
		shaderResourceViews, unorderedAccessViews, NUM_ELEMENTS, 1, 1);

	//将GPU计算的结果写回CPU
	ID3D11Buffer* debugBuf = NULL;
	// 先查看resultBuffer中的内容
	debugBuf = CreateAndCopyToDebugBuf(pDevice.Get(), pContext.Get(), resultBuffer);
	if (debugBuf == NULL)
	{
		puts("debugBuf create failed!");
		return -1;
	}

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	pContext->Map((ID3D11Resource*)debugBuf, 0, D3D11_MAP_READ, 0, &mappedResource);
	struct BufType* p = (struct BufType*)mappedResource.pData;

	puts("Output GPU resultBuffer results, first ten:");
	for (int i = 0; i < 10; i++)
		printf("i: %d, f: %.1f\n", p[i].i, p[i].f);

	puts("last ten:");
	for (int i = NUM_ELEMENTS - 10; i < NUM_ELEMENTS; i++)
		printf("i: %d, f: %.1f\n", p[i].i, p[i].f);

	pContext->Unmap((ID3D11Resource*)debugBuf, 0);
	debugBuf->Release();

	// 再查看srcdstBuffer中的内容
	debugBuf = CreateAndCopyToDebugBuf(pDevice.Get(), pContext.Get(), srcDstBuffer);
	if (debugBuf == NULL)
	{
		puts("debugBuf create failed!");
		return -1;
	}
	pContext->Map((ID3D11Resource*)debugBuf, 0, D3D11_MAP_READ, 0, &mappedResource);
	int* q = (int*)mappedResource.pData;

	puts("Output GPU srcDstBuffer results, first ten:");
	for (int i = 0; i < 10; i++)
		printf("[%d] = %d\n", i, q[i]);

	puts("last ten:");
	for (int i = NUM_ELEMENTS - 10; i < NUM_ELEMENTS; i++)
		printf("[%d] = %d\n", i, q[i]);

	pContext->Unmap((ID3D11Resource*)debugBuf, 0);
	debugBuf->Release();


	return 0;
}

void D3dClass::InitComputeShader()
{
	uint32_t dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined(DEBUG) | defined(_DEBUG)
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	// We generally prefer to use the higher CS shader profile when possible as CS 5.0 is better performance on 11-class hardware
	ComPtr<ID3DBlob>pErrorBlob;
	ComPtr<ID3DBlob>computeShader;

	HR(D3DCompileFromFile(L"../shader/compute_CS.hlsl", nullptr, nullptr, "main", "cs_5_0", dwShaderFlags, 0,
		&computeShader, &pErrorBlob));

	HR(pDevice->CreateComputeShader(computeShader->GetBufferPointer(),
		computeShader->GetBufferSize(), NULL, m_ComputeShader.GetAddressOf()));
}

void D3dClass::InitComputeBuffer(void* pInitData, uint32_t elementSize, uint32_t uCount, ComPtr<ID3D11Buffer>&ppBufferOut)
{
	ppBufferOut.Reset();

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	desc.ByteWidth = elementSize * uCount;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = elementSize;

	if (pInitData != NULL)
	{
		D3D11_SUBRESOURCE_DATA InitData = { 0 };
		InitData.pSysMem = pInitData;
		HR(pDevice->CreateBuffer(&desc, &InitData, ppBufferOut.GetAddressOf()));
	}
	else
		HR( pDevice->CreateBuffer(&desc, NULL, ppBufferOut.GetAddressOf()) );
}

//ShaderResouceView
void D3dClass::InitComputeShaderResouceView(ComPtr<ID3D11Buffer> pBuffer, ComPtr<ID3D11ShaderResourceView>& m_ComputeSRV)
{
	D3D11_BUFFER_DESC descBuf;
	ZeroMemory(&descBuf, sizeof(descBuf));
	pBuffer->GetDesc(&descBuf);

	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	desc.BufferEx.FirstElement = 0;

	//假定这是个structure buffer
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.BufferEx.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;

	HR( pDevice->CreateShaderResourceView((ID3D11Resource*)pBuffer.Get(), &desc, m_ComputeSRV.GetAddressOf()) );
}


//UnorderedAccessView
void D3dClass::InitComputeUnorderedAccessView(ComPtr<ID3D11Buffer> pBuffer, ComPtr<ID3D11UnorderedAccessView>& m_ComputeUAV)
{
	D3D11_BUFFER_DESC descBuf = {};
	pBuffer->GetDesc(&descBuf);

	D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};
	desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	desc.Buffer.FirstElement = 0;

	//假设这是一个structure buffer
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.Buffer.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;

	HR( pDevice->CreateUnorderedAccessView((ID3D11Resource*)pBuffer.Get(), &desc, m_ComputeUAV.GetAddressOf()) );
}


//运行Shader Compute程序
void D3dClass::RunGPUCompute()
{
	//初始化计算数据
	for (int i = 0; i < NUM_ELEMENTS; i++)
	{
		s_vBuf0[i].i = i;
		s_vBuf0[i].f = (float)i;

		s_vBuf1[i].i = i;
		s_vBuf1[i].f = (float)i;
	}

	//各个Buffer指针变量
	ComPtr<ID3D11Buffer>srcBuffer0;
	ComPtr<ID3D11Buffer>srcBuffer1;
	ComPtr<ID3D11Buffer>resultBuffer;
	ComPtr<ID3D11Buffer>srcDstBuffer;
	ComPtr<ID3D11Buffer>constBuffer;

	//读写上面buffer的ID3D11ShaderResourceView和UnorderedAccessView接口
	ComPtr<ID3D11ShaderResourceView>srcBuf0SRV;
	ComPtr<ID3D11ShaderResourceView>srcBuf1SRV;
	ComPtr<ID3D11UnorderedAccessView>resBufUAV;
	ComPtr<ID3D11UnorderedAccessView>srcdstBufUAV;

	int localBuffer[NUM_ELEMENTS];
	for (int i = 0; i < NUM_ELEMENTS; i++)
		localBuffer[i] = i + 1;

	//1. 初始化 compute shader
	InitComputeShader();

	//2. 初始化计算数据
	for (int i = 0; i < NUM_ELEMENTS; i++)
	{
		s_vBuf0[i].i = i;
		s_vBuf0[i].f = (float)i;

		s_vBuf1[i].i = i;
		s_vBuf1[i].f = (float)i;
	}

	//3. 为CPU中的数组创建GPU中相应Buffer
	InitComputeBuffer(s_vBuf0, sizeof(struct BufType), NUM_ELEMENTS, srcBuffer0);
	InitComputeBuffer(s_vBuf1, sizeof(struct BufType), NUM_ELEMENTS, srcBuffer1);
	InitComputeBuffer(nullptr, sizeof(struct BufType), NUM_ELEMENTS, resultBuffer);
	InitComputeBuffer(nullptr, sizeof(struct BufType), NUM_ELEMENTS, srcDstBuffer);


	//4. 在D3D11中，常量缓存至少需要4个int元素
	int value[4] = { 10, 20 };
	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.ByteWidth = sizeof(value);
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = value;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;
	pDevice->CreateBuffer(&desc, &initData, constBuffer.GetAddressOf());


	//5. 绑定常量缓存
	pContext->CSSetConstantBuffers(0, 1, constBuffer.GetAddressOf());


	//6. 为buffer创建相应的shader resource view与unordered access view
	InitComputeShaderResouceView(srcBuffer0, srcBuf0SRV);
	InitComputeShaderResouceView(srcBuffer1, srcBuf1SRV);
	InitComputeUnorderedAccessView(resultBuffer, resBufUAV);
	InitComputeUnorderedAccessView(srcDstBuffer, srcdstBufUAV);
	

	//7. 运行Shader Compute程序
	ID3D11ShaderResourceView* shaderResourceViews[] = { srcBuf0SRV.Get(), srcBuf1SRV.Get() };
	ID3D11UnorderedAccessView* unorderedAccessViews[] = { resBufUAV.Get(), srcdstBufUAV.Get() };

	pContext->CSSetShader(m_ComputeShader.Get(), NULL, 0);
	pContext->CSSetShaderResources(0, _countof(shaderResourceViews), shaderResourceViews);
	pContext->CSSetUnorderedAccessViews(0, _countof(unorderedAccessViews), unorderedAccessViews, NULL);
	pContext->Dispatch(NUM_ELEMENTS, 1, 1);

	//清空Shader和各个Shader Resource View、Unordered Access View以及一些Constant Buffer
	pContext->CSSetShader(NULL, NULL, 0);

	ID3D11UnorderedAccessView* ppUAViewNULL[] = { NULL, NULL };
	pContext->CSSetUnorderedAccessViews(0, 2, ppUAViewNULL, NULL);

	ID3D11ShaderResourceView* ppSRVNULL[2] = { NULL,NULL };
	pContext->CSSetShaderResources(0, 2, ppSRVNULL);

	ID3D11Buffer* ppCBNULL[1] = { NULL };
	pContext->CSSetConstantBuffers(0, 1, ppCBNULL);


	//8.将GPU计算的结果写回CPU
	ComPtr<ID3D11Buffer> debugBuf;
	desc = {};
	resultBuffer->GetDesc(&desc);
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.MiscFlags = 0;

	//8.1 先查看resultBuffer中的内容
	HR(pDevice->CreateBuffer(&desc, NULL, &debugBuf));
	THROW_D3D_EXCEPTION( pContext->CopyResource((ID3D11Resource*)debugBuf.Get(),
		(ID3D11Resource*)resultBuffer.Get()) );//取到 debugBuf 中

	//8.2 拿到 CPU 内存中
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	pContext->Map((ID3D11Resource*)debugBuf.Get(), 0, D3D11_MAP_READ, 0, &mappedResource);
	struct BufType* p = (struct BufType*)mappedResource.pData;

	puts("Output GPU resultBuffer results, first ten:");
	for (int i = 0; i < 10; i++)
		printf("i: %d, f: %.1f\n", p[i].i, p[i].f);

	puts("last ten:");
	for (int i = NUM_ELEMENTS - 10; i < NUM_ELEMENTS; i++)
		printf("i: %d, f: %.1f\n", p[i].i, p[i].f);

	pContext->Unmap((ID3D11Resource*)debugBuf.Get(), 0);


	//8.3 再查看srcdstBuffer中的内容
	D3D11_BUFFER_DESC desc2 = {};
	srcDstBuffer->GetDesc(&desc2);
	desc2.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc2.Usage = D3D11_USAGE_STAGING;
	desc2.BindFlags = 0;
	desc2.MiscFlags = 0;

	debugBuf.Reset();
	HR(pDevice->CreateBuffer(&desc2, NULL, &debugBuf));
	THROW_D3D_EXCEPTION( pContext->CopyResource((ID3D11Resource*)debugBuf.Get(),
		(ID3D11Resource*)srcDstBuffer.Get()) );//取到 debugBuf 中

	D3D11_MAPPED_SUBRESOURCE mappedResource2;
	pContext->Map((ID3D11Resource*)debugBuf.Get(), 0, D3D11_MAP_READ, 0, &mappedResource2);
	int* q = (int*)mappedResource2.pData;

	puts("Output GPU srcDstBuffer results, first ten:");
	for (int i = 0; i < 10; i++)
		printf("[%d] = %d\n", i, q[i]);

	puts("last ten:");
	for (int i = NUM_ELEMENTS - 10; i < NUM_ELEMENTS; i++)
		printf("[%d] = %d\n", i, q[i]);

	pContext->Unmap((ID3D11Resource*)debugBuf.Get(), 0);
}
