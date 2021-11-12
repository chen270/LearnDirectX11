#include "PixelShader.h"
#include "misc/DXTrace.h"

PixelShader::PixelShader( D3dClass& d3d,const std::wstring& path )
{
	Microsoft::WRL::ComPtr<ID3DBlob> pBlob;
	HR( D3DReadFileToBlob( path.c_str(),&pBlob ) );
	HR( GetDevice( d3d )->CreatePixelShader( pBlob->GetBufferPointer(),pBlob->GetBufferSize(),nullptr,&pPixelShader ) );
}

void PixelShader::Bind( D3dClass& d3d ) noexcept
{
	GetContext( d3d )->PSSetShader( pPixelShader.Get(),nullptr,0u );
}
