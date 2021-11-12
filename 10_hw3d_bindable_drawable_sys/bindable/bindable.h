#ifndef __BINDABLE_H__
#define __BINDABLE_H__

#include "d3dclass.h"

class Bindable
{
public:
	virtual void Bind( D3dClass& d3d ) noexcept = 0;
	virtual ~Bindable() = default;
protected:
	static ID3D11DeviceContext* GetContext( D3dClass& d3d ) noexcept;
	static ID3D11Device* GetDevice( D3dClass& d3d ) noexcept;
};


#endif // __BINDABLE_H__