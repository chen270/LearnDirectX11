#include "drawable.h"
#include "indexBuffer.h"
#include <cassert>
#include <typeinfo>

void Drawable::Draw( D3dClass& d3d ) const noexcept(!IS_DEBUG)
{
	for( auto& b : binds )
	{
		b->Bind( d3d );
	}
	for (auto& b : GetStaticBinds())
	{
		b->Bind(d3d);
	}
	d3d.DrawIndexed( pIndexBuffer->GetCount() );
}

//可以绑定多个内容
void Drawable::AddBind( std::unique_ptr<Bindable> bind ) noexcept(!IS_DEBUG)
{
	assert( "*Must* use AddIndexBuffer to bind index buffer" && typeid(*bind) != typeid(IndexBuffer) );
	binds.push_back( std::move( bind ) );
}

//只绑定Index buffer用来绘制图形
void Drawable::AddIndexBuffer( std::unique_ptr<IndexBuffer> ibuf ) noexcept
{
	assert( "Attempting to add index buffer a second time" && pIndexBuffer == nullptr );
	pIndexBuffer = ibuf.get();
	binds.push_back( std::move( ibuf ) );
}
