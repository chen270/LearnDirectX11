#ifndef __D3DAPP_H__
#define __D3DAPP_H__

#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <d3d11_1.h>
#include <wrl/client.h>
#include <DirectXMath.h>

class D3DApp
{
public:
	D3DApp(HWND hwnd) :m_hMainWnd(hwnd) {}
	int Init();
	int Run();
	int CreateTex();
private:
	bool InitDirect3D();
	bool InitEffect();
	bool InitResource();
	void DrawScene();
private:
	void OnResize();
	HRESULT CreateShaderFromFile(
		const WCHAR* csoFileNameInOut,
		const WCHAR* hlslFileName,
		LPCSTR entryPoint,
		LPCSTR shaderModel,
		ID3DBlob** ppBlobOut);


public:
	// 使用模板别名(C++11)简化类型名
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	// Direct3D 11
	ComPtr<ID3D11Device> m_pd3dDevice;                    // D3D11设备
	ComPtr<ID3D11DeviceContext> m_pd3dImmediateContext;   // D3D11设备上下文
	ComPtr<IDXGISwapChain> m_pSwapChain;                  // D3D11交换链
	UINT      m_4xMsaaQuality;   // MSAA支持的质量等级

	// Direct3D 11.1
	ComPtr<ID3D11Device1> m_pd3dDevice1;                  // D3D11.1设备
	ComPtr<ID3D11DeviceContext1> m_pd3dImmediateContext1; // D3D11.1设备上下文
	ComPtr<IDXGISwapChain1> m_pSwapChain1;                // D3D11.1交换链

	// 常用资源
	ComPtr<ID3D11Texture2D> m_pDepthStencilBuffer;        // 深度模板缓冲区
	ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;   // 渲染目标视图
	ComPtr<ID3D11DepthStencilView> m_pDepthStencilView;   // 深度模板视图
	D3D11_VIEWPORT m_ScreenViewport;                      // 视口

	bool m_Enable4xMsaa = true;
	int m_ClientWidth = 800;                                    // 视口宽度
	int m_ClientHeight= 600;                                    // 视口高度

	HWND      m_hMainWnd;        // 主窗口句柄


public:

	struct VertexPosColor
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT4 color;
		static const D3D11_INPUT_ELEMENT_DESC inputLayout[2];
	};


private:
	ComPtr<ID3D11InputLayout> m_pVertexLayout;	// 顶点输入布局
	ComPtr<ID3D11Buffer> m_pVertexBuffer;		// 顶点缓冲区
	ComPtr<ID3D11VertexShader> m_pVertexShader;	// 顶点着色器
	ComPtr<ID3D11PixelShader> m_pPixelShader;	// 像素着色器
};

#endif //__D3DAPP_H__
