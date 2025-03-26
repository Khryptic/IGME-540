
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
	Light lights[5] : LIGHT;
};

// Calculates the Diffusion Factor of Point and Spot Lights
float3 CalculateDiffuse(Light light, VertexToPixel input, float3 direction)
{
    return dot(input.normal, -direction) *
		   light.Color * light.Intensity;
}

// Calculates the linear falloff of a Spot Light
float CalculateFalloff(Light light, VertexToPixel input)
{
	// Get cos(angle) between pixel and light direction
    float pixelAngle = saturate(dot(light.Position - input.worldPosition, light.Direction));
	
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

// Calculates the Specularity of the pixel
float3 CalculateSpecular(Light light, VertexToPixel input, float3 direction)
{
	float3 view = normalize(cameraPosition - input.worldPosition);
	float3 reflection = reflect(direction, input.normal);
	float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
	
	return pow(max(dot(reflection, view), 0.0f), specExponent);
}

// Calculates the total light hitting the pixel
float3 CalculateLightingTotal(VertexToPixel input, float4 surfaceColor)
{
	float3 total = ambient;
		
	for (int i = 0; i < 1; i++)
	{
		float3 diffuse = { 0, 0, 0 };
		float3 specular = { 0, 0, 0 };
        float3 attenuation = { 0, 0, 0 };
        float fallOff = 0;
			
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
                float3 direction = normalize(lights[i].Position - input.worldPosition);
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
                total += (diffuse + specular) * attenuation * fallOff;
				break;
		}

	}
	
    return total;
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
	input.normal = normalize(input.normal);
	float4 carpetColor = Carpet.Sample(Simple, input.uv);
	float4 iceColor = Ice.Sample(Simple, input.uv);
	float4 surfaceColor = colorTint * abs(iceColor - carpetColor);
	float3 totalLight = CalculateLightingTotal(input, surfaceColor);
	
	return float4(totalLight, 1);
}