#include "texture.h"
#include "misc/DXTrace.h"
#include "misc/DirectXTex/WICTextureLoader11.h"
#include "misc/DirectXTex/DDSTextureLoader11.h"

Texture::Texture(D3dClass& d3d)
{
	CreateSampler(d3d);
}

Texture::Texture(D3dClass& d3d, std::wstring picturePath)
{
	CreateSampler(d3d);
	const WCHAR* postfix = (std::wstring(picturePath.end() - 3, picturePath.end()) ).c_str();
	ID3D11Resource* tex;
	//从文件创建一个渲染视图：
	if (0 == wcscmp(postfix, L"dds"))
	{
		HR(DirectX::CreateDDSTextureFromFile(
			GetDevice(d3d),
			picturePath.c_str(),
			&tex, pTextureView.GetAddressOf()));
	}
	else
	{
		HR(DirectX::CreateWICTextureFromFile(
			GetDevice(d3d),
			picturePath.c_str(),
			&tex, pTextureView.GetAddressOf()));
	}

}

void Texture::CreateSampler(D3dClass& d3d)
{
	//创建一个采样器描述
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	HR(GetDevice(d3d)->CreateSamplerState(&samplerDesc, pSamplerState.GetAddressOf()));
}

void Texture::Bind(D3dClass& d3d) noexcept
{
	THROW_D3D_EXCEPTION( GetContext(d3d)->PSSetShaderResources(0, 1, pTextureView.GetAddressOf()));
	THROW_D3D_EXCEPTION( GetContext(d3d)->PSSetSamplers(0, 1, pSamplerState.GetAddressOf()) );
}