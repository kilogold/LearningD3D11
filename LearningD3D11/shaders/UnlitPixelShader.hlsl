struct PixelShaderInput
{
    float2 texcoord : TEXCOORD;
    float4 color : COLOR;
    float3 normalWS : WS_NORMAL;
    float4 positionWS : WS_POSTION;
};

float4 main(PixelShaderInput IN) : SV_TARGET
{
    return float4(1, 1, 1, 1);
}