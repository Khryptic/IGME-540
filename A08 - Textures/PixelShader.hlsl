
// Texture and Sampler Registers
Texture2D Carpet : register(t0); // Registers for Textures
Texture2D Ice : register(t1); // Registers for Textures
SamplerState Simple : register(s0); // Registers for Samplers

// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 screenPosition	: SV_POSITION;
	float2 uv				: TEXCOORD;
	float3 normal			: NORMAL;
};

// New constant buffer data for use with SimpleShaders
cbuffer ExternalData : register(b0)
{
    float4 colorTint	 : COLOR;
    float2 scale		 : TEXCOORD;
    float2 offset	     : TEXCOOD;
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
	
    return abs(carpetColor - iceColor) * colorTint;
}