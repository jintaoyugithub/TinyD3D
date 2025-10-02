struct VSInput
{
    float3 position : POSITION; // Vertex position from vertex buffer
    float4 color : COLOR; // Vertex color
};

struct PSInput
{
    float4 position : SV_POSITION; // Transformed position (clip space)
    float4 color : COLOR; // Pass-through color
};

PSInput VSMain(VSInput input)
{
    PSInput output;

    // Convert 3D position to 4D homogeneous clip space
    output.position = float4(input.position, 1.0f);

    // Pass the color directly to the pixel shader
    output.color = input.color;

    return output;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    // Just output the interpolated vertex color
    return input.color;
}
