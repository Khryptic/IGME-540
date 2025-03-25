
#include "GGPShadersInclude.hlsli"

// Texture and Sampler Registers
Texture2D Carpet : register(t0); // Registers for Textures
Texture2D Ice : register(t1); // Registers for Textures
SamplerState Simple : register(s0); // Registers for Samplers

// New constant buffer data for use with SimpleShaders
cbuffer ExternalData : register(b0)
{
    float4 colorTint	 : COLOR;
    float2 scale		 : TEXCOORD;
    float2 offset	     : TEXCOOD;
    float3 cameraPosition : POSITION;
    float roughness : SCALAR;
    float3 ambient : COLOR;
};

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
    // Return the input with the colorTint from the constant buffer
	// Pass the color through 
	// - The values will be interpolated per-pixel by the rasterizer
    input.uv = input.uv * scale + offset;
    float4 carpetColor = Carpet.Sample(Simple, input.uv);
    float4 iceColor = Ice.Sample(Simple, input.uv);
    float4 surfaceColor = mul(colorTint, float4(ambient, 1));
	
    //return abs(carpetColor - iceColor) * surfaceColor;
    return float4(input.normal, 1);

}