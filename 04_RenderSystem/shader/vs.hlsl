
struct VSOut
{
    float4 color : COLOR;
    float4 pos : SV_Position;
};

cbuffer CBuf
{
    /*row_major*/ matrix transform;
}

VSOut main(float3 pos : POSITION, float4 color : COLOR)
{
    VSOut vso;
    vso.pos = float4(pos, 1.0f);
    //vso.pos = mul(float4(pos, 1.0f), transform);
    vso.color = color;
    return vso;
}

////教程用法
//float4 main_lite(float2 pos : POSITIONT) : SV_Position
//{
//    return float4(pos, 0.0f, 1.0f);
//}