// Triangle.hlsli

struct VertexIn
{
    float3 pos : POSITION;
    float4 color : COLOR;
};

struct VertexOut
{
    float4 posH : SV_POSITION; //��Ϊ��դ��ʱ����ȷ��������λ��
    float4 color : COLOR;
};
