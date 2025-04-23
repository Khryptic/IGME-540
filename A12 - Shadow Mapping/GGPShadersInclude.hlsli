#ifndef __GGP_LIGHTING_SHADER_INCLUDES__ // Each .hlsli file needs a unique identifier!
#define __GGP_LIGHTING_SHADER_INCLUDES__

#define LIGHT_TYPE_DIRECTION 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

#define MAX_SPECULAR_EXPONENT 256.0f

// Struct representing a single vertex worth of data
// - This should match the vertex definition in our C++ code
// - By "match", I mean the size, order and number of members
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float3 localPosition : POSITION; // XYZ position
    float2 uv : TEXCOORD; // UV position
    float3 normal : NORMAL; // Normal position
    float3 tangent : TANGENT; // Tangent vector
};

// Struct to hold lighting data
struct Light
{
    int Type; // Which kind of light? Direction, Point, Spot?
    float3 Direction; // Directional and Spot lights need a direction
    float Range; // Point and Spot lights have a max range for attenuation
    float3 Position; // Point and Spot lights have a position in space
    float Intensity; // All lights need an intensity
    float3 Color; // All lights need a color
    float SpotInnerAngle; // Inner cone angle (in radians) – Inside this, full light!
    float SpotOuterAngle; // Outer cone angle (radians) – Outside this, no light!
    float2 Padding; // Purposefully padding to hit the 16-byte boundary
};

// Struct representing the data we're sending down the pipeline 
// - The name of the struct itself is unimportant, but should be descriptive
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
    float3 worldPosition : POSITION;
    float3 tangent : TANGENT;
    float4 shadowMapPosition : SHADOW_POSITION;
};

// Struct representing the data we're sending down the pipeline for the Skybox
struct SkyVertexToPixel
{
    // Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float4 position : SV_POSITION;
    float3 sampleDir : DIRECTION;
};

#endif