#include "indexBuffer.h"
#include "misc/DXTrace.h"

IndexBuffer::IndexBuffer( D3dClass& d3d,const std::vector<unsigned short>& indices )
	:
	count( (UINT)indices.size() )
{
	D3D11_BUFFER_DESC ibd = {};
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.CPUAccessFlags = 0u;
	ibd.MiscFlags = 0u;
	ibd.ByteWidth = UINT( count * sizeof( unsigned short ) );
	ibd.StructureByteStride = sizeof( unsigned short );
	D3D11_SUBRESOURCE_DATA isd = {};
	isd.pSysMem = indices.data();
	HR( GetDevice( d3d )->CreateBuffer( &ibd,&isd,pIndexBuffer.GetAddressOf() ) );
}

void IndexBuffer::Bind( D3dClass& d3d ) noexcept
{
	GetContext( d3d )->IASetIndexBuffer( pIndexBuffer.Get(),DXGI_FORMAT_R16_UINT,0u );
}

UINT IndexBuffer::GetCount() const noexcept
{
	return count;
}
