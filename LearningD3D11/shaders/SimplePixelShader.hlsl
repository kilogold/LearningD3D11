#define MAX_LIGHTS 8
 
// Light types.
#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

// Shading types.
#define PHONG_SHADING 0
#define BLINN_PHONG_SHADING 1

struct _Material
{
    float4 Emissive;        // 16 bytes
    //----------------------------------- (16 byte boundary)
    float4 Ambient;         // 16 bytes
    //------------------------------------(16 byte boundary)
    float4 Diffuse;         // 16 bytes
    //----------------------------------- (16 byte boundary)
    float4 Specular;        // 16 bytes
    //----------------------------------- (16 byte boundary)
    float SpecularPower;    // 4 bytes
    bool UseTexture;        // 4 bytes
    float2 Padding;         // 8 bytes
    //----------------------------------- (16 byte boundary)
}; // Total:                // 80 bytes ( 5 * 16 )

struct Light
{
    float4 Position; // 16 bytes
    //----------------------------------- (16 byte boundary)
    float4 Direction; // 16 bytes
    //----------------------------------- (16 byte boundary)
    float4 Color; // 16 bytes
    //----------------------------------- (16 byte boundary)
    float SpotAngle; // 4 bytes
    float ConstantAttenuation; // 4 bytes
    float LinearAttenuation; // 4 bytes
    float QuadraticAttenuation; // 4 bytes
    //----------------------------------- (16 byte boundary)
    int LightType; // 4 bytes
    bool Enabled; // 4 bytes
    int2 Padding; // 8 bytes
    //----------------------------------- (16 byte boundary)
}; // Total:                           // 80 bytes (5 * 16)

cbuffer MaterialProperties : register(b0)
{
    _Material Material;
};

cbuffer LightProperties : register(b1)
{
    float4 EyePosition;       // 16 bytes
    //----------------------------------- (16 byte boundary)
    float4 GlobalAmbient;     // 16 bytes
    //----------------------------------- (16 byte boundary)
    Light Lights[MAX_LIGHTS]; // 80 * MAX_LIGHTS(8) = 640 bytes
    //----------------------------------- (16 byte boundary)
    int PhongShadingMode;     // 4 bytes
    int3 Padding;             // 12 bytes
    //----------------------------------- (16 byte boundary)
};  // Total:                 // 688 bytes (43 * 16)

struct PixelShaderInput
{
    float4 color : COLOR;
    float3 normalWS : WS_NORMAL;
    float4 positionWS : WS_POSTION;
};

struct LightingResult
{
    float4 Diffuse;
    float4 Specular;
};

Texture2D Texture : register(t0);
sampler Sampler : register(s0);
 
float4 DoDiffuse(Light light, float3 surfaceToLightVector, float3 normal)
{
    float dotLightAndNormal = max(0, dot(surfaceToLightVector, normal)); // Keep the value positive.
    return light.Color * dotLightAndNormal;
}

float4 DoSpecular(Light light, float3 surfaceToLightVector, float3 eyeVector, float3 normal, int phongMode)
{
    if (phongMode == PHONG_SHADING)
    {
        float3 reflectedLightVector = normalize(reflect(surfaceToLightVector, normal));
        float dotEyeAndReflectedLight = max(0, dot(eyeVector, reflectedLightVector));
        return light.Color * pow(dotEyeAndReflectedLight, Material.SpecularPower);
    }
    else
    {
        float3 halfAngleVector = normalize(surfaceToLightVector + eyeVector);
        float dotHalfAngleAndNormal = max(0, dot(normal, halfAngleVector));
        return light.Color * pow(dotHalfAngleAndNormal, Material.SpecularPower);
    }
}

float DoAttenuation(Light light, float distance)
{
    return 1.0f / (light.ConstantAttenuation + 
                    light.LinearAttenuation * distance + 
                    light.QuadraticAttenuation * distance * distance);
}

LightingResult DoPointLight(Light light, float3 eyeVector, float4 surfacePosition, float3 normal)
{
    LightingResult result;
    
    float3 surfaceToLightVector = (light.Position - surfacePosition).xyz;
    float distance = length(surfaceToLightVector);
    surfaceToLightVector = surfaceToLightVector / distance; //normalize.
    
    float attenuation = DoAttenuation(light, distance);
    
    result.Diffuse = DoDiffuse(light, surfaceToLightVector, normal) * attenuation;
    result.Specular = DoSpecular(light, eyeVector, surfaceToLightVector, normal, PHONG_SHADING) * attenuation;
    
    return result;
}

LightingResult ComputeLighting(float4 surfacePosition, float3 normal)
{
        
    if (!Lights[0].Enabled)
    {
        LightingResult result = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };
        return result;
    }
    else
    {
        return DoPointLight(Lights[0], EyePosition.xyz, surfacePosition, normal);
    }
}

float4 SimplePixelShader( PixelShaderInput IN ) : SV_TARGET
{
    LightingResult lit = ComputeLighting(IN.positionWS, normalize(IN.normalWS));

    float4 emissive = Material.Emissive;
    float4 ambient = Material.Ambient * GlobalAmbient;
    float4 diffuse = Material.Diffuse * lit.Diffuse;
    float4 specular = Material.Specular * lit.Specular;
    
    float4 texColor = { 1, 1, 1, 1 };
    
    float4 finalColor = (emissive + ambient + diffuse + specular) * texColor;
    
    return finalColor;
}