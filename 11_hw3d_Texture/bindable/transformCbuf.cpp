#include "TransformCbuf.h"

TransformCbuf::TransformCbuf(D3dClass& d3d, const Drawable& parent)
	:
	vcbuf(d3d),
	parent(parent)
{}

void TransformCbuf::Bind(D3dClass& d3d) noexcept
{
	vcbuf.Update(d3d,
		DirectX::XMMatrixTranspose(
			parent.GetTransformXM() * d3d.GetProjection()
		)
	);
	vcbuf.Bind(d3d);
}
