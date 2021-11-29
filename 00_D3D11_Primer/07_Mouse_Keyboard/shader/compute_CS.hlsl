// 这是一个计算着色器程序
 
struct BufType
{
    int i;
    float f;
};
 
// 对应于主机端的constant buffer
cbuffer cbNeverChanges : register(b0)
{
    int cValue0;
    int cValue1;
};
 
// 对应于主机端的Shader Resource View
StructuredBuffer<BufType> buffer0 : register(t0);
StructuredBuffer<BufType> buffer1 : register(t1);
 
// 对应于主机端的Unordered Access View
RWStructuredBuffer<BufType> bufferOut : register(u0);
RWStructuredBuffer<int> srcdstBuffer : register(u1);
 
// Direct3D中，一个线程组(threadgroup)最多允许1024个线程
[numthreads(1024, 1, 1)]
void main(uint3 groupID : SV_GroupID, uint3 tid : SV_DispatchThreadID, 
    uint3 localTID : SV_GroupThreadID, uint gIdx : SV_GroupIndex)
{
    /************************/
    snorm float tmp = 15;
    float4 vec1 = { 1.0f, 2.0f, 3.0f, 4.0f };
    float sum = vec1.x +vec1.y + vec1.z + vec1.w;
    
    float2x2 mat1 =
    {
      1.3f, 2.5f,
      3.4f, 4.6f
    };
    
    float f2 = mat1._12;
    /************************/
    
    
    
    
    const int index = tid.x;
    const int cValue = cValue1 / cValue0; //2
    //int resValue = (buffer0[index].i + buffer1[index].i) * cValue - srcdstBuffer[index]; // (buffer0[i] + buffer0[i]) * 2 - localBuffer[i]
    int resValue = f2;
    bufferOut[index].i = resValue;
    bufferOut[index].f = f2;
 
    srcdstBuffer[index] = resValue;
}