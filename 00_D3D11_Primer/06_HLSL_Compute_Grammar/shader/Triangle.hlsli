// Triangle.hlsli

struct VertexIn
{
    float3 pos : POSITION;
    float4 color : COLOR;
};

struct VertexOut
{
    float4 posH : SV_POSITION; //作为光栅化时最终确定的像素位置
    float4 color : COLOR;
};
