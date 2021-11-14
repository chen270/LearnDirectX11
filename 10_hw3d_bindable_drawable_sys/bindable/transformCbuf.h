#ifndef __TRANSFORMCBUF_H__
#define __TRANSFORMCBUF_H__

#include "constantBuffers.h"
#include "drawable.h"
#include <DirectXMath.h>

class TransformCbuf : public Bindable
{
public:
	TransformCbuf(D3dClass& d3d,const Drawable& parent );
	void Bind(D3dClass& d3d) noexcept override;
private:
	VertexConstantBuffer<DirectX::XMMATRIX> vcbuf;
	const Drawable& parent;
};

#endif // __TRANSFORMCBUF_H__