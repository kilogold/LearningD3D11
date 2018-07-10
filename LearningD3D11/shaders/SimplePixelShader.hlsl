struct PixelShaderInput
{
    float4 color : COLOR;
    float3 normalWS : WS_NORMAL;
    float4 positionWS : WS_POSTION;
};

Texture2D Texture : register(t0);
sampler Sampler : register(s0);
 
float4 SimplePixelShader( PixelShaderInput IN ) : SV_TARGET
{
    //return Texture.Sample(Sampler, IN.texcoord) * IN.color;
    return IN.color;
}