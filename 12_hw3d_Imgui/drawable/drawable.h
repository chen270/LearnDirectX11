#ifndef __DRAWABLE_H__
#define __DRAWABLE_H__

#include "d3dclass.h"
#include <DirectXMath.h>
#include <vector>
#include <memory>

class Bindable;

class Drawable
{
public:
	Drawable() = default;
	Drawable( const Drawable& ) = delete;

	virtual DirectX::XMMATRIX GetTransformXM() const noexcept = 0;
	virtual void Update(float dt) noexcept = 0;

	void Draw( D3dClass& d3d ) const noexcept(!IS_DEBUG);
	void AddBind( std::unique_ptr<Bindable> bind ) noexcept(!IS_DEBUG);
	void AddIndexBuffer( std::unique_ptr<class IndexBuffer> ibuf ) noexcept;
	virtual ~Drawable() = default;

private:
	virtual const std::vector<std::unique_ptr<Bindable>>& GetStaticBinds() const noexcept = 0;

private:
	std::vector<std::unique_ptr<Bindable>> binds;

protected:
	const IndexBuffer* pIndexBuffer = nullptr;

};


#endif // __DRAWABLE_H__