
// Texture and Sampler Registers
Texture2D Ice : register(t0); // Registers for Textures
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
    float4 screenPosition : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

// New constant buffer data for use with SimpleShaders
cbuffer ExternalData : register(b0)
{
    float4 colorTint : COLOR;
    float2 scale     : TEXCOORD;
    float2 offset    : TEXCOOD;
};

// Pseudo-Random Noise Function
float random(float2 s)
{
    return frac(cos(tan(dot(s, float2(25.9898, 48.233))) * 445385.512354) * 123.0431);
}

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
    input.uv = input.uv * scale + offset;
    
    // New Color Based on Noise
    float4 newColor = colorTint * float4(random(input.screenPosition.yz), random(input.screenPosition.xy), random(input.screenPosition.xz), 1);
    float4 IceColor = Ice.Sample(Simple, input.uv);
    float4 TotalColor = IceColor * newColor;
	       
    return TotalColor;
}