#ifndef __INDEXBUFFER_H__
#define __INDEXBUFFER_H__

#include "bindable.h"
#include <vector>

class IndexBuffer : public Bindable
{
public:
	IndexBuffer( D3dClass& d3d,const std::vector<unsigned short>& indices );
	void Bind( D3dClass& d3d ) noexcept override;
	UINT GetCount() const noexcept;
protected:
	UINT count;
	Microsoft::WRL::ComPtr<ID3D11Buffer> pIndexBuffer;
};


#endif // __INDEXBUFFER_H__