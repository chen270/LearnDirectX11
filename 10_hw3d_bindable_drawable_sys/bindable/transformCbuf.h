#ifndef __TRANSFORMCBUF_H__
#define __TRANSFORMCBUF_H__

#include "ConstantBuffers.h"
#include "drawable.h"
#include <DirectXMath.h>

class TransformCbuf : public Bindable
{
public:
	TransformCbuf( Graphics& gfx,const Drawable& parent );
	void Bind( Graphics& gfx ) noexcept override;
private:
	VertexConstantBuffer<DirectX::XMMATRIX> vcbuf;
	const Drawable& parent;
};

#endif // __TRANSFORMCBUF_H__