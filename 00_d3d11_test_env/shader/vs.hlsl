//输入: 顶点数据
//输出: 顶点位置，以便在光栅化该位置
//注:Position为语义，名称不固定,标记为输入; 
//SV_Position标记为输出
float4 main(float3 pos : POSITIONT ): SV_Position
{
    return float4(pos, 1.0f);
}

////教程用法
//float4 main_lite(float2 pos : POSITIONT) : SV_Position
//{
//    return float4(pos, 0.0f, 1.0f);
//}