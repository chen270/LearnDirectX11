#ifndef __VERTEXBUFFER_H__
#define __VERTEXBUFFER_H__

#include "bindable.h"
#include <vector>

class VertexBuffer : public Bindable
{
public:
	template<class V>
	VertexBuffer( D3dClass& d3d,const std::vector<V>& vertices )
		:
		stride( sizeof( V ) )
	{
		D3D11_BUFFER_DESC bd = {};
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.CPUAccessFlags = 0u;
		bd.MiscFlags = 0u;
		bd.ByteWidth = UINT( sizeof( V ) * vertices.size() );
		bd.StructureByteStride = sizeof( V );
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = vertices.data();
		HR( GetDevice( d3d )->CreateBuffer( &bd,&sd,pVertexBuffer.GetAddressOf() ) );
	}
	void Bind( D3dClass& d3d ) noexcept override;
protected:
	UINT stride;
	ComPtr<ID3D11Buffer> pVertexBuffer;
};


#endif // __VERTEXBUFFER_H__