//Cube.hlsli

cbuffer ConstantBuffer : register(b0)
{
    matrix World; 
    matrix View;  
    matrix Proj;  
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