#include "Triangle.hlsli"

cbuffer CBuf
{
	/*row_major*/ matrix transform;
}

// 顶点着色器
VertexOut main(VertexIn vIn)
{
	VertexOut vOut;
	vOut.posH = float4(vIn.pos, 1.0f);
	vOut.color = vIn.color; // 这里alpha通道的值默认为1.0
	return vOut;
}