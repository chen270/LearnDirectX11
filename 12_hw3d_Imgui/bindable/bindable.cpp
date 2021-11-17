#include "bindable.h"

ID3D11DeviceContext* Bindable::GetContext( D3dClass& d3d ) noexcept
{
	return d3d.pContext.Get();
}

ID3D11Device* Bindable::GetDevice( D3dClass& d3d ) noexcept
{
	return d3d.pDevice.Get();
}

