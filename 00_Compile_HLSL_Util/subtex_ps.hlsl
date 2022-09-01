Texture2D tex;
SamplerState samplerType;
 
struct pixelInputType
{
    float4 pos : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

float4 main(pixelInputType input) : SV_TARGET
{
    float4 texColor = tex.Sample(samplerType, input.texcoord);
    if (texColor.a < 0.1 && texColor.r > 0.1 && texColor.g > 0.1 && texColor.b > 0.1)
      discard;
    texColor.a = 1.0;
    return texColor;
}