#include "vertexBuffer.h"

void VertexBuffer::Bind( D3dClass& d3d ) noexcept
{
	const UINT offset = 0u;
	GetContext( d3d )->IASetVertexBuffers( 0u,1u,pVertexBuffer.GetAddressOf(),&stride,&offset );
}
