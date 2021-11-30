// Cube_PS.hlsl
#include "Cube.hlsli"

float4 main(VertexOut pIn) : SV_Target
{
    return pIn.color;   
}