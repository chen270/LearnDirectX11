#ifndef __D3DCLASS_H__
#define __D3DCLASS_H__

#include <windows.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include "Geometry.h"
#include "LightHelper.h"

class D3dClass
{
public:
	D3dClass();
	~D3dClass();

	D3dClass(const D3dClass &) = delete;
	D3dClass& operator=(const D3dClass &) = delete;

	int InitD3d11(HWND hwnd, int screenWidth, int screenHeight);
	int EndFrame();
	int ClearBuffer(float r, float g, float b) noexcept;

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

	//渲染目标视图
	ComPtr<ID3D11RenderTargetView>  pRenderTargetView = nullptr;

	//深度模板视图
	ComPtr<ID3D11DepthStencilView>  pDepthStencilView = nullptr;


private:
	void DrawTestTriangleErr();

public:
	bool ResetMesh(const Geometry::MeshData<VertexPosNormalColor>& meshData);
	void InitShader_CSO();
	void InitShader_CompileInRunTime(LPCWSTR vsFilePath, LPCWSTR psFilePath, const D3D11_INPUT_ELEMENT_DESC* _inputLayout, UINT _numelement);
	void InitTriangleResource();
	void InitCubeResource();
	void UpdateScene(float x, float y);
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
	ComPtr<ID3D11Buffer> m_pIndexBuffer;			// 索引缓冲区
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

	void InitLightResource();
	void DrawScene();
	void UseComputeShader();


	ComPtr<ID3D11VertexShader> pVertexShader;
	ComPtr<ID3D11PixelShader> pPixelShader;

	ComPtr<ID3D11ShaderResourceView> m_pTextureInputA;
	ComPtr<ID3D11ShaderResourceView> m_pTextureInputB;

	ComPtr<ID3D11Texture2D> m_pTextureOutputA;
	ComPtr<ID3D11Texture2D> m_pTextureOutputB;
	ComPtr<ID3D11UnorderedAccessView> m_pTextureOutputA_UAV;
	ComPtr<ID3D11UnorderedAccessView> m_pTextureOutputB_UAV;
};

#endif //__D3DCLASS_H__