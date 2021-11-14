#include "inputLayout.h"
#include "misc/DXTrace.h"

InputLayout::InputLayout( D3dClass& d3d,
	const std::vector<D3D11_INPUT_ELEMENT_DESC>& layout,
	ID3DBlob* pVertexShaderBytecode )
{
	HR( GetDevice( d3d )->CreateInputLayout(
		layout.data(),(UINT)layout.size(),
		pVertexShaderBytecode->GetBufferPointer(),
		pVertexShaderBytecode->GetBufferSize(),
		pInputLayout.GetAddressOf()
	) );
}

void InputLayout::Bind( D3dClass& d3d ) noexcept
{
	GetContext( d3d )->IASetInputLayout( pInputLayout.Get() );
}
