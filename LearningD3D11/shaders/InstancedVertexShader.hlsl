cbuffer PerFrame : register( b0 )
{
    matrix viewProjectionMatrix;
}

struct AppData
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 color : COLOR;

    matrix worldMatrix : WORLDMATRIX;
    matrix inverseTransposeWorldMatrix : INVERSETRANSPOSEWORLDMATRIX;

};

struct VertexShaderOutput
{
    float2 texcoord : TEXCOORD;
    float4 color : COLOR;
    float3 normalWS : WS_NORMAL;
    float4 positionWS : WS_POSTION;
    float4 position : SV_POSITION;
};

VertexShaderOutput InstancedVertexShader( AppData IN )
{
    VertexShaderOutput OUT;
 
    matrix MVP = mul(viewProjectionMatrix, IN.worldMatrix);

    OUT.texcoord = float2(0, 0);
    OUT.color = float4( IN.color, 1.0f );
    OUT.normalWS = mul((float3x3) IN.inverseTransposeWorldMatrix, IN.normal);
    OUT.positionWS = mul(IN.worldMatrix, float4(IN.position, 1));
    OUT.position = mul(MVP, float4(IN.position, 1.0f));
    return OUT;
}
