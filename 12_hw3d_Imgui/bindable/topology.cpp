#include "topology.h"

Topology::Topology( D3dClass& d3d,D3D11_PRIMITIVE_TOPOLOGY type )
	:
	type( type )
{}

void Topology::Bind( D3dClass& d3d ) noexcept
{
	GetContext( d3d )->IASetPrimitiveTopology( type );
}
