
#include "GGPShadersInclude.hlsli"

// Texture and Sampler Registers
Texture2D Rock : register(t0); // Registers for Textures
Texture2D Ice : register(t1); // Registers for Textures
SamplerState Sample : register(s0); // Registers for Samplers
Texture2D NormalMap : register(t2); // Registers for the Normal Map

// New constant buffer data for use with SimpleShaders
cbuffer ExternalData : register(b0)
{
	float4 colorTint	 : COLOR;
	float2 scale		 : TEXCOORD;
	float2 offset	     : TEXCOOD;
	float3 cameraPosition : POSITION;
	float roughness : SCALAR;
	float3 ambient : COLOR;
	Light lights[5] : LIGHT;
};

// Calculates the Diffusion Factor of Point and Spot Lights
float3 CalculateDiffuse(Light light, VertexToPixel input, float3 direction)
{
    return saturate(dot(input.normal, -direction)) *
		   light.Color * light.Intensity;
}

// Calculates the Specularity of the pixel
float3 CalculateSpecular(Light light, VertexToPixel input, float3 direction)
{
    float3 view = normalize(cameraPosition - input.worldPosition);
    float3 reflection = reflect(direction, input.normal);
    float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
	
    return pow(max(dot(reflection, view), 0.0f), specExponent);
}

// Calculates the linear falloff of a Spot Light
float CalculateFalloff(Light light, VertexToPixel input)
{
	// Get cos(angle) between pixel and light direction
    float pixelAngle = saturate(dot(normalize(light.Position - input.worldPosition), -light.Direction));
	
	// Get cosines of angles and calculate range
    float cosOuter = cos(light.SpotOuterAngle);
    float cosInner = cos(light.SpotInnerAngle);
    float falloffRange = cosOuter - cosInner;
	
	// Linear falloff over the range, clamp 0-1, apply to light calc
    return saturate((cosOuter - pixelAngle) / falloffRange);
}

// Calculates the Attenuation Factor for Point and Spot Lights
float3 CalculateAttenuation(Light light, VertexToPixel input)
{
	float dist = distance(light.Position, input.worldPosition);
	float att = saturate(1.0f - (dist * dist / (light.Range * light.Range)));
	return att * att;
}

// Calculates the total light hitting the pixel
float3 CalculateLightingTotal(VertexToPixel input, float4 surfaceColor)
{
	float3 total = ambient;
		
	for (int i = 0; i < 5; i++)
	{
		float3 diffuse = { 0, 0, 0 };
		float3 specular = { 0, 0, 0 };
        float3 attenuation = { 0, 0, 0 };
        float fallOff = 0;
        float3 direction = normalize(lights[i].Direction);
			
		switch (lights[i].Type)
		{  
			// Directional Light
			case LIGHT_TYPE_DIRECTION:
				diffuse = CalculateDiffuse(lights[i], input, lights[i].Direction);
                specular = CalculateSpecular(lights[i], input, lights[i].Direction);
				total += diffuse + specular;
				break;
			
			// Point Light
			case LIGHT_TYPE_POINT:
                direction = normalize(input.worldPosition - lights[i].Position);
			
				diffuse = CalculateDiffuse(lights[i], input, direction);
                specular = CalculateSpecular(lights[i], input, direction);
                attenuation = CalculateAttenuation(lights[i], input);
                total += (diffuse + specular) * attenuation;
				break;
			
			// Spot Light
			case LIGHT_TYPE_SPOT:
                diffuse = CalculateDiffuse(lights[i], input, lights[i].Direction);
                specular = CalculateSpecular(lights[i], input, lights[i].Direction);
                fallOff = CalculateFalloff(lights[i], input);
                attenuation = CalculateAttenuation(lights[i], input);
                total += ((diffuse + specular) * attenuation) * fallOff;
				break;
		}
		
		// Cut the specular if the diffuse contribution is zero
		// - any() returns 1 if any component of the param is non-zero
		// - In other words:
		// - If the diffuse amount is 0, any(diffuse) returns 0
		// - If the diffuse amount is != 0, any(diffuse) returns 1
		// - So when diffuse is 0, specular becomes 0
        specular *= any(diffuse);

	}
	
    return total * surfaceColor.xyz;
}

float3 TransformNormal(VertexToPixel input, float3 unpackedNormal)
{
    float3 N = normalize(input.normal);
    float3 T = normalize(input.tangent);
    T = normalize(T - N * dot(T, N)); // Gram-Schmidt assumes that T & N are normalized
    float3 B = cross(T, N);
    float3x3 TBN = float3x3(T, B, N);
	
    return mul(unpackedNormal, TBN); // Order of Multiplication is important
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
	// Return the input with the colorTint from the constant buffer
	// Pass the color through 
	// - The values will be interpolated per-pixel by the rasterizer
	input.uv = input.uv * scale + offset;
	
	//input.normal = normalize(input.normal);	Older normal calculation
	// Unpack Normal Map
    float3 unpackedNormal = NormalMap.Sample(Sample, input.uv).rgb * 2 - 1;
    unpackedNormal = normalize(unpackedNormal);
    input.normal = TransformNormal(input, unpackedNormal);
	
	// Calculate Lighting
	float4 rockColor = Rock.Sample(Sample, input.uv);
	float4 iceColor = Ice.Sample(Sample, input.uv);
	float4 surfaceColor = colorTint * abs(iceColor - rockColor);
	float3 totalLight = CalculateLightingTotal(input, surfaceColor);
	
	// Return pixel color
	return float4(totalLight, 1);
}