#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "bindable.h"
#include <string>
class Texture : public Bindable
{
public:
	Texture(D3dClass& d3d);
	Texture(D3dClass& d3d, std::wstring picturePath);

	~Texture() = default;
	void Bind(D3dClass& d3d) noexcept override;

private:
	void CreateSampler(D3dClass& d3d);

protected:
	ComPtr<ID3D11ShaderResourceView> pTextureView;
	ComPtr<ID3D11SamplerState> pSamplerState;
};



#endif // __TEXTURE_H__