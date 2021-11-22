#ifndef __COMPUTESHADER_H__
#define __COMPUTESHADER_H__

#include <windows.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

class ComputeShader
{
public:
	// 使用模板别名(C++11)简化类型名
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	ComputeShader(ComPtr<ID3D11Device> _pDevice, ComPtr<ID3D11DeviceContext>_pContext);
	~ComputeShader();

	void RunGPUCompute();

private:
	ComPtr<ID3D11Device>			pDevice = nullptr;//分配内存，创建资源
	ComPtr<ID3D11DeviceContext>	    pContext = nullptr;//发布渲染命令

	//Compute Shader
	ComPtr<ID3D11ComputeShader> m_ComputeShader;

	void InitComputeShader();
	void InitComputeBuffer(void* pInitData, uint32_t elementSize, uint32_t uCount, ComPtr<ID3D11Buffer>& ppBufferOut);
	void InitComputeShaderResouceView(ComPtr<ID3D11Buffer> pBuffer, ComPtr<ID3D11ShaderResourceView>& m_ComputeSRV);//ShaderResouceView
	void InitComputeUnorderedAccessView(ComPtr<ID3D11Buffer> pBuffer, ComPtr<ID3D11UnorderedAccessView>& m_ComputeUAV);//UnorderedAccessView
};




#endif // __COMPUTESHADER_H__