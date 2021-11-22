#include "vertexShader.h"
#include "misc/DXTrace.h"
#include <d3dcompiler.h>


VertexShader::VertexShader( D3dClass& d3d,const std::wstring& path )
{
	HR( D3DReadFileToBlob( path.c_str(),pBytecodeBlob.GetAddressOf() ) );
	HR( GetDevice( d3d )->CreateVertexShader( 
		pBytecodeBlob->GetBufferPointer(),
		pBytecodeBlob->GetBufferSize(),
		nullptr,
		pVertexShader.GetAddressOf() 
	) );
}

void VertexShader::Bind( D3dClass& d3d ) noexcept
{
	GetContext( d3d )->VSSetShader( pVertexShader.Get(),nullptr,0u );
}

ID3DBlob* VertexShader::GetBytecode() const noexcept
{
	return pBytecodeBlob.Get();
}
