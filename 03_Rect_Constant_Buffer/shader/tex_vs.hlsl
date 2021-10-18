
struct VSOut
{
    float4 pos : SV_Position;
    float2 texcoord : TEXCOORD0;
};

struct pixelInputType
{
    float4 pos : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};


VSOut main(float3 pos : POSITIONT, float2 tex : TEXCOORD0)
{
    pixelInputType vso;
    vso.pos = float4(pos, 1.0f);
    vso.texcoord = tex;
    return vso;
}

////教程用法
//float4 main_lite(float2 pos : POSITIONT) : SV_Position
//{
//    return float4(pos, 0.0f, 1.0f);
//}