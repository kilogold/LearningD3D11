cbuffer PerObject : register( b0 )
{
    matrix worldMatrix;
    matrix inverseTransposeWorldMatrix;
    matrix worldViewProjectMatrix;
}

struct AppData
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 color: COLOR;
};

struct VertexShaderOutput
{
    float4 color : COLOR;
    float3 normalWS : WS_NORMAL;
    float4 positionWS : WS_POSTION;
    float4 position : SV_POSITION;
};

VertexShaderOutput SimpleVertexShader( AppData IN )
{
    VertexShaderOutput OUT;
 
    OUT.color = float4( IN.color, 1.0f );
    OUT.normalWS = mul((float3x3)inverseTransposeWorldMatrix, IN.normal);
    OUT.positionWS = mul(worldMatrix, float4(IN.position, 1));
    OUT.position = mul(worldViewProjectMatrix, float4(IN.position, 1.0f));
    return OUT;
}
