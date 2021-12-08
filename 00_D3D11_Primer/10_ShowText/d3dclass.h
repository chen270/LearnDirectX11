#ifndef __D3DCLASS_H__
#define __D3DCLASS_H__

#include <windows.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include "Geometry.h"
#include "LightHelper.h"
#include "misc/m_k/keyboard.h"
#include "misc/m_k/mouse.h"
using namespace DirectX;

class D3dClass
{
public:
	D3dClass();
	~D3dClass();

	D3dClass(const D3dClass &) = delete;
	D3dClass& operator=(const D3dClass &) = delete;

	int InitD3d11(HWND hwnd, int screenWidth, int screenHeight);
	int InitD3d11_DXGI(HWND hwnd, int screenWidth, int screenHeight);
	void OnResize();

	int EndFrame();
	int ClearBuffer(float r, float g, float b) noexcept;
	bool InitEffect();
private:
	bool m_vsync_enabled;
	int m_screenWidth;
	int m_screenHeight;

	// 使用模板别名(C++11)简化类型名
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;


	//D3d设备三要素
	ComPtr<ID3D11Device>			pDevice			  = nullptr;//分配内存，创建资源
	ComPtr<IDXGISwapChain>			pSwap			  = nullptr;
	ComPtr<ID3D11DeviceContext>	    pContext		  = nullptr;//发布渲染命令

	// Direct3D 11.1
	ComPtr<ID3D11Device1> m_pd3dDevice1;						// D3D11.1设备
	ComPtr<ID3D11DeviceContext1> m_pd3dImmediateContext1;		// D3D11.1设备上下文
	ComPtr<IDXGISwapChain1> m_pSwapChain1;						// D3D11.1交换链

	//渲染目标视图
	ComPtr<ID3D11RenderTargetView>  pRenderTargetView = nullptr;

	//深度模板视图
	ComPtr<ID3D11DepthStencilView>  pDepthStencilView = nullptr;

public:
	bool ResetMesh(const Geometry::MeshData<VertexPosNormalColor>& meshData);
	void InitShader_CompileInRunTime(LPCWSTR vsFilePath, LPCWSTR psFilePath);
	void UpdateScene(float dt, Keyboard::State& state);
	void InitLightResource();
	void DrawScene();
private:
	float AspectRatio();

private:
	struct VSConstantBuffer
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX proj;
		DirectX::XMMATRIX worldInvTranspose;
	};
	struct PSConstantBuffer
	{
		DirectionalLight dirLight;
		PointLight pointLight;
		SpotLight spotLight;
		Material material;
		DirectX::XMFLOAT4 eyePos;
	};

	ComPtr<ID3D11Buffer> m_pIndexBuffer;			// 索引缓冲区
	ComPtr<ID3D11Buffer> m_pVertexBuffer;			// 顶点缓冲区
	ComPtr<ID3D11Buffer> m_pConstantBuffers[2];	    // 常量缓冲区

	DirectionalLight m_DirLight;					// 默认环境光
	PointLight m_PointLight;						// 默认点光
	SpotLight m_SpotLight;						    // 默认汇聚光
	VSConstantBuffer m_VSConstantBuffer;			// 用于修改用于VS的GPU常量缓冲区的变量
	PSConstantBuffer m_PSConstantBuffer;			// 用于修改用于PS的GPU常量缓冲区的变量
	ComPtr<ID3D11InputLayout> m_pVertexLayout;	    // 顶点输入布局

	ComPtr<ID3D11RasterizerState> m_pRSWireframe;	// 光栅化状态: 线框模式
	bool m_IsWireframeMode;							// 当前是否为线框模式
	UINT m_IndexCount;							    // 绘制物体的索引数组大小

	ComPtr<ID3D11VertexShader> pVertexShader;
	ComPtr<ID3D11PixelShader> pPixelShader;

	DirectX::Keyboard::KeyboardStateTracker m_KeyboardTracker;
	//std::unique_ptr<DirectX::Keyboard>m_pKeyboard;
};

#endif //__D3DCLASS_H__