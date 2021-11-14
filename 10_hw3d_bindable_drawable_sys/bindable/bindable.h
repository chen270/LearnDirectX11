#ifndef __BINDABLE_H__
#define __BINDABLE_H__

#include "d3dclass.h"

class Bindable
{
public:
	virtual void Bind( D3dClass& d3d ) noexcept = 0;
	virtual ~Bindable() = default;
protected:
	static ID3D11DeviceContext* GetContext( D3dClass& d3d ) noexcept;
	static ID3D11Device* GetDevice( D3dClass& d3d ) noexcept;


protected:
	// 使用模板别名(C++11)简化类型名
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
};


#endif // __BINDABLE_H__