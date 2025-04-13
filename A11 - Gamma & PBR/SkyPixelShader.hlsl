
#include "GGPShadersInclude.hlsli"

// Texture Variables
TextureCube SkyTexture : register(t0);
SamplerState SkySampler : register(s0);

float4 main(SkyVertexToPixel input) : SV_TARGET
{
       return SkyTexture.Sample(SkySampler, input.sampleDir);
}