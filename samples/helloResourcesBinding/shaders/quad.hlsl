/*
* Register space overview
* t for SRV
* s for sampler
* u for UAV
* b for CBV
*
* Resources syntax in HLSL: 
* SM 5.0: https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/d3d11-graphics-reference-sm5-objects
* SM 5.1: https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/shader-model-5-1-objects and ConstantBuffer<> template
*/

/*
* Questions:
* 1. can I put the structs in a header file which are both used in the hlsl code and cpp code?
*/

/*
* TODO:
* 1. specifying root signature in HLSL
*/

//#include "test.hlsli"
#include "shaderio.h"

/* --- Uploaded resources --- */
/// Textures
Texture2D<float4> checkboard : register(t0, space0);
RWTexture2D<float4> randomRWTex : register(u1, space0);

/// Buffers
ConstantBuffer<DrawConstants> drawConstants : register(b0, space0);
ConstantBuffer<LightConstants> lightConstants : register(b1, space0);
RWStructuredBuffer<GPUParticle> gpuParticles : register(u0, space0);

/// Sampler
SamplerState checkBoardSampler : register(s0, space0);

struct VSInput
{
    float3 pos : POSITION;
    float4 color : COLOR;
};

struct PSInput
{
    float4 pos : SV_Position;
    float4 color : COLOR;
};

PSInput VSMain(VSInput input)
{
    PSInput output;

    // Convert 3D position to 4D homogeneous clip space
    output.pos = float4(input.pos, 1.0f);

    // Pass the color directly to the pixel shader
    output.color = input.color;

    return output;
    return output;
}

float4 PSMain(PSInput input) : SV_Target
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
