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
	int SetTex(WCHAR* filename);
	int EndFrame();
	int ClearBuffer(float r, float g, float b) noexcept;
	int DrawTriangle();
	int DrawCube(float angle, float x, float z);
	int DrawRect(float angle);
	int DrawRectWithTex();

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

	//纹理
	//ID3D11ShaderResourceView* m_texture;

private:
	struct Vertex {	
		DirectX::XMFLOAT3 pos;
		unsigned char color[4]; // RGBA 比float节省空间
	};

	struct VertexWithTex {			
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 tex;
	};
};

#endif //__D3DCLASS_H__