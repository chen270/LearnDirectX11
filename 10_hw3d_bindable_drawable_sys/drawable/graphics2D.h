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

private:
	struct VertexWithColor {				// 定义一个顶点结构体，暂时就一个坐标属性
		DirectX::XMFLOAT3 pos;
		unsigned char color[4]; // RGBA 比float节省空间
	};

	int m_screenWidth;
	int m_screenHeight;
};

#endif // __GRAPHICS2D_H__