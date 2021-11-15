#include "texture.h"
#include "misc/DXTrace.h"
#include "misc/DirectXTex/WICTextureLoader11.h"

Texture::Texture(D3dClass& d3d)
{
	CreateSampler(d3d);
#if 0
	// create texture resource
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = s.GetWidth();
	textureDesc.Height = s.GetHeight();
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = s.GetBufferPtr();
	sd.SysMemPitch = s.GetWidth() * sizeof(Surface::Color);
	wrl::ComPtr<ID3D11Texture2D> pTexture;
	GFX_THROW_INFO(GetDevice(gfx)->CreateTexture2D(
		&textureDesc, &sd, &pTexture
	));

	// create the resource view on the texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	GFX_THROW_INFO(GetDevice(gfx)->CreateShaderResourceView(
		pTexture.Get(), &srvDesc, &pTextureView
	));
#endif
}

Texture::Texture(D3dClass& d3d, std::wstring picturePath)
{
	CreateSampler(d3d);
	auto postfix = ( std::wstring(picturePath.end() - 3, picturePath.end()) ).c_str();
	if (0 == wcscmp(postfix, L"jpg"))
	{
		//从文件创建一个渲染视图：
		ID3D11Resource* tex;
		HR(DirectX::CreateWICTextureFromFile(
			GetDevice(d3d),
			picturePath.c_str(),
			&tex, pTextureView.GetAddressOf()));
	}
	else if (0 == wcscmp(postfix, L"dds"))
	{

	}
	else
	{
		int nResult = MessageBoxW(GetForegroundWindow(), L"纹理图片格式错误", L"错误", MB_YESNO | MB_ICONERROR);
		if (nResult == IDYES)
			DebugBreak();
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

	HR(GetDevice(d3d)->CreateSamplerState(&samplerDesc, &pSamplerState));
}

void Texture::Bind(D3dClass& d3d) noexcept
{
	THROW_D3D_EXCEPTION( GetContext(d3d)->PSSetShaderResources(0, 1, pTextureView.GetAddressOf()));
	THROW_D3D_EXCEPTION( GetContext(d3d)->PSSetSamplers(0, 1, pSamplerState.GetAddressOf()) );
}