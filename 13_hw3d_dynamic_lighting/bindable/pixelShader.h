#ifndef __PIXELSHADER_H__
#define __PIXELSHADER_H__

#include "bindable.h"
#include <string>

class PixelShader : public Bindable
{
public:
	PixelShader( D3dClass& d3d,const std::wstring& path );
	void Bind( D3dClass& d3d ) noexcept override;
protected:
	ComPtr<ID3D11PixelShader> pPixelShader;
};


#endif // __PIXELSHADER_H__