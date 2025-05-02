
#include "GGPShadersInclude.hlsli"

cbuffer ExternalData : register(b0)
{
    matrix view;
    matrix projection;
};

SkyVertexToPixel main(VertexShaderInput input)
{
	// Set up output struct
    SkyVertexToPixel output;
    
    // Make copy of view matrix but with no translation
    matrix viewUntranslated = view;
    viewUntranslated._14 = 0.0f;
    viewUntranslated._24 = 0.0f;
    viewUntranslated._34 = 0.0f;
    
    // Apply projection and view matrices to input
    output.position = mul(projection, mul(viewUntranslated, float4(input.localPosition, 1.0f)));
    
    // Ensure output depth is 1.0
    output.position.z = output.position.w;
    
    // Get sample direction for this vertex from center of the object
    output.sampleDir = input.localPosition;
    
    return output;
}