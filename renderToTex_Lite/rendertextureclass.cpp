////////////////////////////////////////////////////////////////////////////////
// Filename: rendertextureclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "rendertextureclass.h"


RenderTextureClass::RenderTextureClass()
{
	m_renderTargetTexture = 0;
	m_renderTargetView = 0;
	m_shaderResourceView = 0;
}


RenderTextureClass::RenderTextureClass(const RenderTextureClass& other)
{
}


RenderTextureClass::~RenderTextureClass()
{
}

void test(ID3D11Device* Device)
{
	ID3D11Texture2D* Texture = nullptr;
	ID3D11Texture2D* Staging = nullptr;

	D3D11_TEXTURE2D_DESC TexDesc;
	TexDesc.Width = 16;
	TexDesc.Height = 16;
	TexDesc.MipLevels = 1;
	TexDesc.ArraySize = 1;
	TexDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	TexDesc.SampleDesc.Count = 1;
	TexDesc.SampleDesc.Quality = 0;
	TexDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	TexDesc.MiscFlags = 0;
	TexDesc.Usage = D3D11_USAGE_DEFAULT;
	TexDesc.CPUAccessFlags = 0;

	HRESULT result = Device->CreateTexture2D(&TexDesc, NULL, &Texture);

	TexDesc.Usage = D3D11_USAGE_STAGING;
	TexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	TexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	result = Device->CreateTexture2D(&TexDesc, NULL, &Staging);

	return;
}

bool RenderTextureClass::Initialize(ID3D11Device* device, int textureWidth, int textureHeight)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT result;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;


	// Initialize the render target texture description.
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	// Setup the render target texture description.
	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0; // D3D11_CPU_ACCESS_READ;
    textureDesc.MiscFlags = 0;

	// Create the render target texture.
	result = device->CreateTexture2D(&textureDesc, NULL, &m_renderTargetTexture);
	if(FAILED(result))
		return false;

	// Setup the description of the render target view.
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	result = device->CreateRenderTargetView(m_renderTargetTexture, &renderTargetViewDesc, &m_renderTargetView);
	if(FAILED(result))
		return false;

	// Setup the description of the shader resource view.
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	result = device->CreateShaderResourceView(m_renderTargetTexture, &shaderResourceViewDesc, &m_shaderResourceView);
	if(FAILED(result))
		return false;


	//for test
	ID3D11Texture2D* m_renderTargetTexture_stage;

#if 0 //使用原来的 desc
	textureDesc.BindFlags = 0;//必须为0
	textureDesc.Usage = D3D11_USAGE_STAGING;
	textureDesc.CPUAccessFlags =  D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
		
	// Create the render target texture.
	result = device->CreateTexture2D(&textureDesc, NULL, &m_renderTargetTexture_stage);

#else //读取原来的描述 

	// 读取创建好的纹理信息
	D3D11_TEXTURE2D_DESC textureDescOrigin;
	m_renderTargetTexture->GetDesc(&textureDescOrigin);
	textureDescOrigin.BindFlags = 0;//必须为0
	textureDescOrigin.Usage = D3D11_USAGE_STAGING;
	textureDescOrigin.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;

	// Create the render target texture.
	result = device->CreateTexture2D(&textureDescOrigin, NULL, &m_renderTargetTexture_stage);
#endif
	if (FAILED(result))
		return false;

	ID3D11DeviceContext* ctx = NULL;
	device->GetImmediateContext(&ctx);
	ctx->CopyResource(m_renderTargetTexture_stage, m_renderTargetTexture);


	D3D11_MAPPED_SUBRESOURCE lock;
	HRESULT hr = ctx->Map((ID3D11Resource*)m_renderTargetTexture_stage, 0, D3D11_MAP_READ, 0, &lock);

	if (FAILED(hr)) {
		perror("Failed to map staging texture\n");
		return false;
	}

	ctx->Unmap((ID3D11Resource*)m_renderTargetTexture_stage, 0);
	ctx->Release();


	return true;
}




void RenderTextureClass::Shutdown()
{
	if(m_shaderResourceView)
	{
		m_shaderResourceView->Release();
		m_shaderResourceView = 0;
	}

	if(m_renderTargetView)
	{
		m_renderTargetView->Release();
		m_renderTargetView = 0;
	}

	if(m_renderTargetTexture)
	{
		m_renderTargetTexture->Release();
		m_renderTargetTexture = 0;
	}

	return;
}


void RenderTextureClass::SetRenderTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* depthStencilView)
{
	// Bind the render target view and depth stencil buffer to the output render pipeline.
	deviceContext->OMSetRenderTargets(1, &m_renderTargetView, depthStencilView);
	
	return;
}


void RenderTextureClass::ClearRenderTarget(ID3D11DeviceContext* deviceContext, ID3D11DepthStencilView* depthStencilView, 
										   float red, float green, float blue, float alpha)
{
	float color[4];


	// Setup the color to clear the buffer to.
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	// Clear the back buffer.
	deviceContext->ClearRenderTargetView(m_renderTargetView, color);
    
	// Clear the depth buffer.
	deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	return;
}


ID3D11ShaderResourceView* RenderTextureClass::GetShaderResourceView()
{
	return m_shaderResourceView;
}