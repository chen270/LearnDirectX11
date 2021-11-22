#ifndef __GRAPHICS2D_H__
#define __GRAPHICS2D_H__

#include "drawableBase.h"

class Graphics2D : public DrawableBase<Graphics2D>
{
public:
	Graphics2D(int w, int h);

	void Update(float dt) noexcept override;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;

	void InitTriangle(D3dClass& d3d);

	void InitCube(D3dClass& d3d);

	void InitRectWithTexture(D3dClass& d3d, WCHAR* texPath);

private:
	struct VertexWithColor {				// 定义一个顶点结构体，暂时就一个坐标属性
		DirectX::XMFLOAT3 pos;
		unsigned char color[4]; // RGBA 比float节省空间
	};

	struct VertexWithTex {
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 tex;
	};

	int m_screenWidth;
	int m_screenHeight;
	float angle;
	float ratio_hw;

	DirectX::XMMATRIX graph2DTransform;
};

#endif // __GRAPHICS2D_H__