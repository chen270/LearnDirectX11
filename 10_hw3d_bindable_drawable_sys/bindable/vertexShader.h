#ifndef __VERTEXSHADER_H__
#define __VERTEXSHADER_H__

#include "bindable.h"
#include <string>

class VertexShader : public Bindable
{
public:
	VertexShader( D3dClass& d3d,const std::wstring& path );
	void Bind( D3dClass& d3d ) noexcept override;
	ID3DBlob* GetBytecode() const noexcept;
protected:
	ComPtr<ID3DBlob> pBytecodeBlob;
	ComPtr<ID3D11VertexShader> pVertexShader;
};

#endif // __VERTEXSHADER_H__