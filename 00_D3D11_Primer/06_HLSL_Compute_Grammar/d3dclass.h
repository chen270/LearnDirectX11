#ifndef __D3DCLASS_H__
#define __D3DCLASS_H__

#include <windows.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

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
	int DrawTriangle(float angle);
	int DrawRect();
	int DrawCube(float angle, float x, float z);
	void DrawTestCube(float angle, float x, float z);

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
	struct Vertex {				// 定义一个顶点结构体，暂时就一个坐标属性
		DirectX::XMFLOAT3 pos;
		unsigned char color[4]; // RGBA 比float节省空间
	};

private:
	void DrawTestTriangleErr();

public:

	void InitShader_CSO();
	void InitShader_INC();
	void InitShader_CompileInRunTime();
	void InitResource();
	void DrawScene();
	void UseComputeShader();


	ComPtr<ID3D11VertexShader> pVertexShader;
	ComPtr<ID3D11PixelShader> pPixelShader;

	bool InitComputeShaderResource();
	void Compute();
	ComPtr<ID3D11ShaderResourceView> m_pTextureInputA;
	ComPtr<ID3D11ShaderResourceView> m_pTextureInputB;

	ComPtr<ID3D11Texture2D> m_pTextureOutputA;
	ComPtr<ID3D11Texture2D> m_pTextureOutputB;
	ComPtr<ID3D11UnorderedAccessView> m_pTextureOutputA_UAV;
	ComPtr<ID3D11UnorderedAccessView> m_pTextureOutputB_UAV;
};

#endif //__D3DCLASS_H__