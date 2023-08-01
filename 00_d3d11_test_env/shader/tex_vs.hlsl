// file : triangleVertexShader.hlsl
struct vertexInputType {
	float4 pos : POSITION;
	float2 texcoord : TEXCOORD0;
};

struct pixelInputType {
	float4 pos : SV_POSITION;
	float2 texcoord : TEXCOORD0;
};

pixelInputType main(vertexInputType input)
{
	pixelInputType output;
	output.pos = input.pos;
	output.texcoord = input.texcoord;
	return output;
}
////教程用法
//float4 main_lite(float2 pos : POSITIONT) : SV_Position
//{
//    return float4(pos, 0.0f, 1.0f);
//}