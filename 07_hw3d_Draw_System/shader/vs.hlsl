
struct VSOut
{
    float4 color : COLOR;
    float4 pos : SV_Position;
};

VSOut main(float3 pos : POSITIONT, float4 color : COLOR)
{
    VSOut vso;
    vso.pos = float4(pos, 1.0f);
    vso.color = color;
    return vso;
}

////教程用法
//float4 main_lite(float2 pos : POSITIONT) : SV_Position
//{
//    return float4(pos, 0.0f, 1.0f);
//}