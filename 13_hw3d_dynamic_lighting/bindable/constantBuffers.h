#ifndef __CONSTANTBUFFERS_H__
#define __CONSTANTBUFFERS_H__

#include "bindable.h"
#include "misc/Dxtrace.h"

template<typename C>
class ConstantBuffer : public Bindable
{
public:
	void Update( D3dClass& d3d,const C& consts )
	{
		D3D11_MAPPED_SUBRESOURCE msr;
		HR( GetContext( d3d )->Map(
			pConstantBuffer.Get(),0u,
			D3D11_MAP_WRITE_DISCARD,0u,
			&msr
		) );
		memcpy( msr.pData,&consts,sizeof( consts ) );
		GetContext( d3d )->Unmap( pConstantBuffer.Get(),0u );
	}
	ConstantBuffer( D3dClass& d3d, const C& consts )
	{
		D3D11_BUFFER_DESC cbd;
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.MiscFlags = 0u;
		cbd.ByteWidth = sizeof( consts );
		cbd.StructureByteStride = 0u;

		D3D11_SUBRESOURCE_DATA csd = {};
		csd.pSysMem = &consts;
		HR( GetDevice( d3d )->CreateBuffer( &cbd,&csd,&pConstantBuffer ) );
	}
	ConstantBuffer( D3dClass& d3d )
	{
		D3D11_BUFFER_DESC cbd;
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.MiscFlags = 0u;
		cbd.ByteWidth = sizeof( C );
		cbd.StructureByteStride = 0u;
		HR( GetDevice( d3d )->CreateBuffer( &cbd,nullptr,&pConstantBuffer ) );
	}
protected:
	ComPtr<ID3D11Buffer> pConstantBuffer;
};

template<typename C>
class VertexConstantBuffer : public ConstantBuffer<C>
{
	using ConstantBuffer<C>::pConstantBuffer;
	using Bindable::GetContext;
public:
	using ConstantBuffer<C>::ConstantBuffer;
	void Bind( D3dClass& d3d ) noexcept override
	{
		GetContext( d3d )->VSSetConstantBuffers( 0u,1u,pConstantBuffer.GetAddressOf() );
	}
};

template<typename C>
class PixelConstantBuffer : public ConstantBuffer<C>
{
	using ConstantBuffer<C>::pConstantBuffer;
	using Bindable::GetContext;
public:
	using ConstantBuffer<C>::ConstantBuffer;
	void Bind( D3dClass& d3d ) noexcept override
	{
		GetContext( d3d )->PSSetConstantBuffers( 0u,1u,pConstantBuffer.GetAddressOf() );
	}
};

#endif // __CONSTANTBUFFERS_H__