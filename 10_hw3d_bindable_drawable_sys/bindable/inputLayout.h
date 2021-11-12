#ifndef __INPUTLAYOUT_H__
#define __INPUTLAYOUT_H__

#include "bindable.h"
#include <vector>

class InputLayout : public Bindable
{
public:
	InputLayout( D3dClass& d3d,
		const std::vector<D3D11_INPUT_ELEMENT_DESC>& layout,
		ID3DBlob* pVertexShaderBytecode );
	void Bind( D3dClass& d3d ) noexcept override;
protected:
	Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;
};


#endif // __INPUTLAYOUT_H__