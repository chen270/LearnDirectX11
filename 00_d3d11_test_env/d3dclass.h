#ifndef __D3DCLASS_H__
#define __D3DCLASS_H__

#include <windows.h>
#include <d3d11.h>
#include <DirectXMath.h>

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
	int DrawTriangle();
	int DrawRect();
	int D3dClass::RenderToTexture();
private:
	bool m_vsync_enabled;
	int m_screenWidth;
	int m_screenHeight;

	//D3d设备三要素
	ID3D11Device*			pDevice			  = nullptr;//分配内存，创建资源
	IDXGISwapChain*			pSwap			  = nullptr;
	ID3D11DeviceContext*	pContext		  = nullptr;//发布渲染命令

	//渲染目标视图
	ID3D11RenderTargetView *pRenderTargetView = nullptr;

private:
	struct Vertex {				// 定义一个顶点结构体，暂时就一个坐标属性
		DirectX::XMFLOAT3 pos;
	};
};

#endif //__D3DCLASS_H__