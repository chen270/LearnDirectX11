#ifndef __D3DCLASS_H__
#define __D3DCLASS_H__

#include <windows.h>
//#include <d3d11.h>
#include <d3d11_1.h>
#include <DirectXMath.h>
#include <vector>

class D3dClass
{
public:
	D3dClass();
	~D3dClass();

	D3dClass(const D3dClass &) = delete;
	D3dClass& operator=(const D3dClass &) = delete;

	int InitD3d11(HWND hwnd, int screenWidth, int screenHeight);
	int InitShader(WCHAR* vsCso, WCHAR* psCso);
	int InitShaderTex(WCHAR* vsCso, WCHAR* psCso);
	int InitResource();
	void DrawScene();


	int SetTex(WCHAR* filename);
	int EndFrame();
	int ClearBuffer(float r, float g, float b) noexcept;
	int DrawCube(float angle, float x, float z);
	int DrawRect(float angle);


	//画图形
	void InitTriangle();
	void InitRect();
	void UpdateRectRotate(float angle);
	void InitRectWithTex();
private:
	bool m_vsync_enabled;
	int m_screenWidth;
	int m_screenHeight;
	
	//为了后续兼容C，不采用ComPtr指针类型
	//Direct3D 11设备三要素
	ID3D11Device*			m_pDevice		  = nullptr;//分配内存，创建资源
	IDXGISwapChain*			m_pSwap			  = nullptr;
	ID3D11DeviceContext*	m_pContext		  = nullptr;//发布渲染命令

	//Direct3D 11.1 设备三要素
	ID3D11Device1* m_pDevice1;                  // D3D11.1设备
	IDXGISwapChain1* m_pSwap1;                  // D3D11.1交换链
	ID3D11DeviceContext1* m_pContext1;  	    // D3D11.1设备上下文
	
	ID3D11RenderTargetView *m_pRenderTargetView = nullptr;//渲染目标视图
	D3D11_VIEWPORT m_ScreenViewport;                      // 视口

	//shader
	ID3D11VertexShader* m_pVertexShader;
	ID3D11InputLayout*  m_pInputLayout;
	ID3D11PixelShader*  m_pPixelShader;

	//buffer
	ID3D11Buffer *m_pVBO;
	ID3D11Buffer *m_pIBO;
	ID3D11Buffer *m_pCBO;
	int m_indexCount;

	//纹理
	ID3D11ShaderResourceView* m_pShaderResourceView;
	ID3D11SamplerState * m_pSamplerState;
	ID3D11ShaderResourceView* m_pTexture;

private:
	struct VertexPosColor {
		DirectX::XMFLOAT3 pos;
		unsigned char color[4]; // RGBA 比float节省空间
		static const D3D11_INPUT_ELEMENT_DESC inputLayout[2];//静态成员，不占用该结构体的空间
	};

	struct VertexPosTex {			
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 tex;
		static const D3D11_INPUT_ELEMENT_DESC inputLayout[2];//静态成员，不占用该结构体的空间
	};

	//常量缓冲区
	struct ConstantBuffer
	{
		DirectX::XMMATRIX world;
		//DirectX::XMMATRIX view;
		//DirectX::XMMATRIX proj;
	};

private:
	int InitResourceEx(VertexPosColor* vertices, int verLen, unsigned short* indices, int indicesLen);
	int InitResourceTex(VertexPosTex* vertices, int verLen, unsigned short* indices, int indicesLen);
};

#endif //__D3DCLASS_H__