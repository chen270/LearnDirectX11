#ifndef __D3D_DEMO_H__
#define __D3D_DEMO_H__

#include <windows.h>
#include <d3d11.h>
#include <DirectXMath.h>

class D3dDemo
{
public:
	D3dDemo(HWND h);
	void DrawScene();
private:
	float width = 600;
	float height = 600;
	HWND hwnd;
public:
	IDXGISwapChain* pIdxgiSwapChain = nullptr;//声明交换链
	ID3D11RenderTargetView* pD3D11RenderTargetView = nullptr;//渲染目标视图
	ID3D11Device* pD3D11Device = nullptr;//设备
	ID3D11DeviceContext*    pImmediateContext = nullptr;//设备上下文

public:
	ID3D11VertexShader* pVertexShader;			// 顶点着色器
	ID3D11PixelShader*  pPixelShader;			// 像素着色器
	ID3D11InputLayout*  m_pVertexLayout;	// 顶点输入布局

public:
	struct Vertex {				// 定义一个顶点结构体，暂时就一个坐标属性
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 tex;
	};

	Vertex vertices[4] =			// 顶点数组
	{
		{ DirectX::XMFLOAT3(-0.3f, -0.3f, 1.0f), DirectX::XMFLOAT2(0.0f,1.0f) },
		{ DirectX::XMFLOAT3(-0.3f,  0.3f, 1.0f), DirectX::XMFLOAT2(0.0f,0.0f) },
		{ DirectX::XMFLOAT3(0.3f,  0.3f,  1.0f) , DirectX::XMFLOAT2(1.0f,0.0f) },
		{ DirectX::XMFLOAT3(0.3f, -0.3f,  1.0f), DirectX::XMFLOAT2(1.0f,1.0f) }
	};

	//IBO
	const unsigned short indices[6] = { 0,1,2,
									   0,2,3 };

public:
	HRESULT CreateSwapChain();
	HRESULT CreateBuffer();
	void CreateVBO();
	void CreateIBO();
	HRESULT CompileShader();
	HRESULT SetTex();
	HRESULT CreateFbo();

private:
	ID3D11Texture2D* renderTargetTextureMap;//贴图渲染目标和着色器资源视图指向的纹理
	ID3D11RenderTargetView* renderTargetViewMap;//指向纹理、贴图渲染目标指针
	ID3D11ShaderResourceView* shaderResourceViewMap;//贴图着色器资源视图的指针
};

#endif //__D3D_DEMO_H__

