#ifndef __TOPOLOGY_H__
#define __TOPOLOGY_H__

#include "bindable.h"

class Topology : public Bindable
{
public:
	Topology( D3dClass& d3d, D3D11_PRIMITIVE_TOPOLOGY type );
	void Bind( D3dClass& d3d ) noexcept override;
protected:
	D3D11_PRIMITIVE_TOPOLOGY type;
};

#endif // __TOPOLOGY_H__