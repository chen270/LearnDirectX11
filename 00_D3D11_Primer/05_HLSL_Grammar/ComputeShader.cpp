#include "ComputeShader.h"
#include <d3dcompiler.h>
#include <direct.h>
#include "misc/DXTrace.h"

#define  NUM_ELEMENTS   2048
static struct BufType
{
	int i;
	float f;
} s_vBuf0[NUM_ELEMENTS], s_vBuf1[NUM_ELEMENTS];

ComputeShader::ComputeShader(ComPtr<ID3D11Device> _pDevice, ComPtr<ID3D11DeviceContext>_pContext)
	: pDevice(_pDevice), pContext(_pContext)
{
}

ComputeShader::~ComputeShader()
{
}

void ComputeShader::InitComputeShader()
{
	uint32_t dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined(DEBUG) | defined(_DEBUG)
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	// We generally prefer to use the higher CS shader profile when possible as CS 5.0 is better performance on 11-class hardware
	ComPtr<ID3DBlob>pErrorBlob;
	ComPtr<ID3DBlob>computeShader;

	HR(D3DCompileFromFile(L"../shader/compute_CS.hlsl", nullptr, nullptr, "main", "cs_5_0", dwShaderFlags, 0,
		&computeShader, &pErrorBlob));

	HR(pDevice->CreateComputeShader(computeShader->GetBufferPointer(),
		computeShader->GetBufferSize(), NULL, m_ComputeShader.GetAddressOf()));
}

void ComputeShader::InitComputeBuffer(void* pInitData, uint32_t elementSize, uint32_t uCount, ComPtr<ID3D11Buffer>& ppBufferOut)
{
	ppBufferOut.Reset();

	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	desc.ByteWidth = elementSize * uCount;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = elementSize;

	if (pInitData != NULL)
	{
		D3D11_SUBRESOURCE_DATA InitData = { 0 };
		InitData.pSysMem = pInitData;
		HR(pDevice->CreateBuffer(&desc, &InitData, ppBufferOut.GetAddressOf()));
	}
	else
		HR(pDevice->CreateBuffer(&desc, NULL, ppBufferOut.GetAddressOf()));
}

//ShaderResouceView
void ComputeShader::InitComputeShaderResouceView(ComPtr<ID3D11Buffer> pBuffer, ComPtr<ID3D11ShaderResourceView>& m_ComputeSRV)
{
	D3D11_BUFFER_DESC descBuf;
	ZeroMemory(&descBuf, sizeof(descBuf));
	pBuffer->GetDesc(&descBuf);

	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	desc.BufferEx.FirstElement = 0;

	//假定这是个structure buffer
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.BufferEx.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;

	HR(pDevice->CreateShaderResourceView((ID3D11Resource*)pBuffer.Get(), &desc, m_ComputeSRV.GetAddressOf()));
}


//UnorderedAccessView
void ComputeShader::InitComputeUnorderedAccessView(ComPtr<ID3D11Buffer> pBuffer, ComPtr<ID3D11UnorderedAccessView>& m_ComputeUAV)
{
	D3D11_BUFFER_DESC descBuf = {};
	pBuffer->GetDesc(&descBuf);

	D3D11_UNORDERED_ACCESS_VIEW_DESC desc = {};
	desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	desc.Buffer.FirstElement = 0;

	//假设这是一个structure buffer
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.Buffer.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;

	HR(pDevice->CreateUnorderedAccessView((ID3D11Resource*)pBuffer.Get(), &desc, m_ComputeUAV.GetAddressOf()));
}


//运行Shader Compute程序
void ComputeShader::RunGPUCompute()
{
	//各个Buffer指针变量
	ComPtr<ID3D11Buffer>srcBuffer0;
	ComPtr<ID3D11Buffer>srcBuffer1;
	ComPtr<ID3D11Buffer>resultBuffer;
	ComPtr<ID3D11Buffer>srcDstBuffer;
	ComPtr<ID3D11Buffer>constBuffer;

	//读写上面buffer的ID3D11ShaderResourceView和UnorderedAccessView接口
	ComPtr<ID3D11ShaderResourceView>srcBuf0SRV;
	ComPtr<ID3D11ShaderResourceView>srcBuf1SRV;
	ComPtr<ID3D11UnorderedAccessView>resBufUAV;
	ComPtr<ID3D11UnorderedAccessView>srcdstBufUAV;

	int localBuffer[NUM_ELEMENTS];
	for (int i = 0; i < NUM_ELEMENTS; i++)
		localBuffer[i] = i + 1;

	//1. 初始化 compute shader
	InitComputeShader();

	//2. 初始化计算数据
	for (int i = 0; i < NUM_ELEMENTS; i++)
	{
		s_vBuf0[i].i = i;
		s_vBuf0[i].f = (float)i;

		s_vBuf1[i].i = i;
		s_vBuf1[i].f = (float)i;
	}

	//3. 为CPU中的数组创建GPU中相应Buffer
	InitComputeBuffer(s_vBuf0, sizeof(struct BufType), NUM_ELEMENTS, srcBuffer0);
	InitComputeBuffer(s_vBuf1, sizeof(struct BufType), NUM_ELEMENTS, srcBuffer1);
	InitComputeBuffer(nullptr, sizeof(struct BufType), NUM_ELEMENTS, resultBuffer);
	InitComputeBuffer(localBuffer, sizeof(localBuffer[0]), _countof(localBuffer), srcDstBuffer);


	//4. 在D3D11中，常量缓存至少需要4个int元素
	int value[4] = { 10, 20 };
	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.ByteWidth = sizeof(value);
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = value;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;
	pDevice->CreateBuffer(&desc, &initData, constBuffer.GetAddressOf());


	//5. 绑定常量缓存
	pContext->CSSetConstantBuffers(0, 1, constBuffer.GetAddressOf());


	//6. 为buffer创建相应的shader resource view与unordered access view
	InitComputeShaderResouceView(srcBuffer0, srcBuf0SRV);
	InitComputeShaderResouceView(srcBuffer1, srcBuf1SRV);
	InitComputeUnorderedAccessView(resultBuffer, resBufUAV);
	InitComputeUnorderedAccessView(srcDstBuffer, srcdstBufUAV);


	//7. 运行Shader Compute程序
	ID3D11ShaderResourceView* shaderResourceViews[] = { srcBuf0SRV.Get(), srcBuf1SRV.Get() };
	ID3D11UnorderedAccessView* unorderedAccessViews[] = { resBufUAV.Get(), srcdstBufUAV.Get() };

	pContext->CSSetShader(m_ComputeShader.Get(), NULL, 0);
	pContext->CSSetShaderResources(0, _countof(shaderResourceViews), shaderResourceViews);
	pContext->CSSetUnorderedAccessViews(0, _countof(unorderedAccessViews), unorderedAccessViews, NULL);
	pContext->Dispatch(NUM_ELEMENTS, 1, 1);

	//8. 清空Shader和各个Shader Resource View、Unordered Access View以及一些Constant Buffer
	pContext->CSSetShader(NULL, NULL, 0);

	ID3D11UnorderedAccessView* ppUAViewNULL[] = { NULL, NULL };
	pContext->CSSetUnorderedAccessViews(0, 2, ppUAViewNULL, NULL);

	ID3D11ShaderResourceView* ppSRVNULL[2] = { NULL,NULL };
	pContext->CSSetShaderResources(0, 2, ppSRVNULL);

	ID3D11Buffer* ppCBNULL[1] = { NULL };
	pContext->CSSetConstantBuffers(0, 1, ppCBNULL);


	//9.将GPU计算的结果写回CPU
	ComPtr<ID3D11Buffer> debugBuf;
	desc = {};
	resultBuffer->GetDesc(&desc);
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.MiscFlags = 0;

	//9.1 先查看resultBuffer中的内容
	HR(pDevice->CreateBuffer(&desc, NULL, &debugBuf));
	THROW_D3D_EXCEPTION(pContext->CopyResource((ID3D11Resource*)debugBuf.Get(),
		(ID3D11Resource*)resultBuffer.Get()));//取到 debugBuf 中

	//9.2 拿到 CPU 内存中
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	pContext->Map((ID3D11Resource*)debugBuf.Get(), 0, D3D11_MAP_READ, 0, &mappedResource);
	struct BufType* p = (struct BufType*)mappedResource.pData;

	puts("Output GPU resultBuffer results, first ten:");
	for (int i = 0; i < 10; i++)
		printf("i: %d, f: %.1f\n", p[i].i, p[i].f);

	puts("last ten:");
	for (int i = NUM_ELEMENTS - 10; i < NUM_ELEMENTS; i++)
		printf("i: %d, f: %.1f\n", p[i].i, p[i].f);

	pContext->Unmap((ID3D11Resource*)debugBuf.Get(), 0);


	//9.3 再查看srcdstBuffer中的内容
	D3D11_BUFFER_DESC desc2 = {};
	srcDstBuffer->GetDesc(&desc2);
	desc2.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc2.Usage = D3D11_USAGE_STAGING;
	desc2.BindFlags = 0;
	desc2.MiscFlags = 0;

	debugBuf.Reset();
	HR(pDevice->CreateBuffer(&desc2, NULL, &debugBuf));
	THROW_D3D_EXCEPTION(pContext->CopyResource((ID3D11Resource*)debugBuf.Get(),
		(ID3D11Resource*)srcDstBuffer.Get()));//取到 debugBuf 中

	D3D11_MAPPED_SUBRESOURCE mappedResource2;
	pContext->Map((ID3D11Resource*)debugBuf.Get(), 0, D3D11_MAP_READ, 0, &mappedResource2);
	int* q = (int*)mappedResource2.pData;

	puts("Output GPU srcDstBuffer results, first ten:");
	for (int i = 0; i < 10; i++)
		printf("[%d] = %d\n", i, q[i]);

	puts("last ten:");
	for (int i = NUM_ELEMENTS - 10; i < NUM_ELEMENTS; i++)
		printf("[%d] = %d\n", i, q[i]);

	pContext->Unmap((ID3D11Resource*)debugBuf.Get(), 0);
}
