//Cube.hlsli

cbuffer ConstantBuffer : register(b0)
{
    matrix gWorld; 
    matrix gView;  
    matrix gProj;  
}


struct VertexIn
{
    float3 posL : POSITION;
    float4 color : COLOR;
};

struct VertexOut
{
    float4 posH : SV_POSITION;
    float4 color : COLOR;
};