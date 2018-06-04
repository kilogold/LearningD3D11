struct PixelShaderInput
{
	float2 texcoord: TEXCOORD;
    float4 color : COLOR;
};

Texture2D Texture : register(t0);
sampler Sampler : register(s0);
 
float4 SimplePixelShader( PixelShaderInput IN ) : SV_TARGET
{
    return Texture.Sample(Sampler, IN.texcoord) * IN.color;
    //return IN.color;
}