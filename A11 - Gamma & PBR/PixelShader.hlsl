
#include "GGPShadersInclude.hlsli"

// Texture and Sampler Registers
Texture2D Albedo : register(t0);		// Registers for the Textures
Texture2D NormalMap : register(t1);		
Texture2D RoughnessMap : register(t2);
Texture2D MetalnessMap : register(t3);
SamplerState Sample : register(s0);		// Registers for Samplers

//Constants
// A constant Fresnel value for non-metals (glass and plastic have values of about 0.04)
static const float F0_NON_METAL = 0.04f;
	
// Minimum roughness for when spec distribution function denominator goes to zero
static const float MIN_ROUGHNESS = 0.0000001f; // 6 zeros after decimal
static const float PI = 3.14159265359f;

// New constant buffer data for use with SimpleShaders
cbuffer ExternalData : register(b0)
{
	float4 colorTint		: COLOR;
	float2 scale			: TEXCOORD;
	float2 offset			: TEXCOOD;
	float3 cameraPosition	: POSITION;
	float roughness			: SCALAR;
    float3 ambient          : COLOR;
	Light lights[5]			: LIGHT;
};

// Lambert diffuse BRDF - Same as the basic lighting diffuse calculation!
// - NOTE: this function assumes the vectors are already NORMALIZED!
float DiffusePBR(float3 normal, float3 dirToLight)
{
    return saturate(dot(normal, dirToLight));
}



// Calculates diffuse amount based on energy conservation
//
// diffuse   - Diffuse amount
// F         - Fresnel result from microfacet BRDF
// metalness - surface metalness amount 
float3 DiffuseEnergyConserve(float3 diffuse, float3 F, float metalness)
{
    return diffuse * (1 - F) * (1 - metalness);
}
 


// Normal Distribution Function: GGX (Trowbridge-Reitz)
//
// a - Roughness
// h - Half vector
// n - Normal
// 
// D(h, n, a) = a^2 / pi * ((n dot h)^2 * (a^2 - 1) + 1)^2
float D_GGX(float3 n, float3 h, float roughness)
{
	// Pre-calculations
    float NdotH = saturate(dot(n, h));
    float NdotH2 = NdotH * NdotH;
    float a = roughness * roughness;
    float a2 = max(a * a, MIN_ROUGHNESS); // Applied after remap!

	// ((n dot h)^2 * (a^2 - 1) + 1)
	// Can go to zero if roughness is 0 and NdotH is 1
    float denomToSquare = NdotH2 * (a2 - 1) + 1;

	// Final value
    return a2 / (PI * denomToSquare * denomToSquare);
}



// Fresnel term - Schlick approx.
// 
// v - View vector
// h - Half vector
// f0 - Value when l = n
//
// F(v,h,f0) = f0 + (1-f0)(1 - (v dot h))^5
float3 F_Schlick(float3 v, float3 h, float3 f0)
{
	// Pre-calculations
    float VdotH = saturate(dot(v, h));

	// Final value
    return f0 + (1 - f0) * pow(1 - VdotH, 5);
}



// Geometric Shadowing - Schlick-GGX
// - k is remapped to a / 2, roughness remapped to (r+1)/2 before squaring!
//
// n - Normal
// v - View vector
//
// G_Schlick(n,v,a) = (n dot v) / ((n dot v) * (1 - k) * k)
//
// Full G(n,v,l,a) term = G_SchlickGGX(n,v,a) * G_SchlickGGX(n,l,a)
float G_SchlickGGX(float3 n, float3 v, float roughness)
{
	// End result of remapping:
    float k = pow(roughness + 1, 2) / 8.0f;
    float NdotV = saturate(dot(n, v));

	// Final value
	// Note: Numerator should be NdotV (or NdotL depending on parameters).
	// However, these are also in the BRDF's denominator, so they'll cancel!
	// We're leaving them out here AND in the BRDF function as the
	// dot products can get VERY small and cause rounding errors.
    return 1 / (NdotV * (1 - k) + k);
}

// Cook-Torrance Microfacet BRDF (Specular)
//
// f(l,v) = D(h)F(v,h)G(l,v,h) / 4(n dot l)(n dot v)
// - parts of the denominator are canceled out by numerator (see below)
//
// D() - Normal Distribution Function - Trowbridge-Reitz (GGX)
// F() - Fresnel - Schlick approx
// G() - Geometric Shadowing - Schlick-GGX
float3 MicrofacetBRDF(float3 n, float3 l, float3 v, float roughness, float3 f0, out float3 F_out)
{
	// Other vectors
    float3 h = normalize(v + l);

	// Run numerator functions
    float D = D_GGX(n, h, roughness);
    float3 F = F_Schlick(v, h, f0);
    float G = G_SchlickGGX(n, v, roughness) * G_SchlickGGX(n, l, roughness);
	
	// Pass F out of the function for diffuse balance
    F_out = F;

	// Final specular formula
	// Note: The denominator SHOULD contain (NdotV)(NdotL), but they'd be
	// canceled out by our G() term.  As such, they have been removed
	// from BOTH places to prevent floating point rounding errors.
    float3 specularResult = (D * F * G) / 4;

	// One last non-obvious requirement: According to the rendering equation,
	// specular must have the same NdotL applied as diffuse!  We'll apply
	// that here so that minimal changes are required elsewhere.
    return specularResult * max(dot(n, l), 0);
}

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
float3 CalculateLightingTotal(VertexToPixel input, float4 surfaceColor, float3 specularColor, float metalness, float roughness)
{
    float3 total = ambient;
		
    for (int i = 0; i < 5; i++)
    {
        float3 diffuse = { 0, 0, 0 };
        float3 specular = { 0, 0, 0 };
        float3 attenuation = { 0, 0, 0 };
        float fallOff = 0;
        float3 F;
        float3 direction;
        float3 balancedDiff;
			
        switch (lights[i].Type)
        {
			// Directional Light
            case LIGHT_TYPE_DIRECTION:
            
                direction = normalize(lights[i].Direction);
			
                // Calculate the light amounts
                diffuse = DiffusePBR(input.normal, direction);
                specular = MicrofacetBRDF(input.normal, direction, cameraPosition, roughness, specularColor, F);
                
                // Calculate diffuse with energy conservation, including cutting diffuse for metals
                balancedDiff = DiffuseEnergyConserve(diffuse, F, metalness);
                
                // Combine the final diffuse and specular values for this light
                total += (balancedDiff * surfaceColor.rgb + specular) * lights[i].Intensity * lights[i].Color;
                break;
			
			// Point Light
            case LIGHT_TYPE_POINT:
                direction = normalize(input.worldPosition - lights[i].Position);
			
                // Calculate the light amounts
                diffuse = DiffusePBR(input.normal, direction);
                specular = MicrofacetBRDF(input.normal, direction, cameraPosition, roughness, specularColor, F);
                
                // Calculate diffuse with energy conservation, including cutting diffuse for metals
                balancedDiff = DiffuseEnergyConserve(diffuse, F, metalness);
                attenuation = CalculateAttenuation(lights[i], input);
                
                // Combine the final diffuse and specular values for this light
                total += (balancedDiff * surfaceColor.rgb + specular) * lights[i].Intensity * lights[i].Color * attenuation;
                break;
			
			// Spot Light
            case LIGHT_TYPE_SPOT:
                
                direction = normalize(lights[i].Direction);
            
                // Calculate the light amounts
                diffuse = DiffusePBR(input.normal, direction);
                specular = MicrofacetBRDF(input.normal, direction, cameraPosition, roughness, specularColor, F);
                
                // Calculate diffuse with energy conservation, including cutting diffuse for metals
                balancedDiff = DiffuseEnergyConserve(diffuse, F, metalness);
                attenuation = CalculateAttenuation(lights[i], input);
                fallOff = CalculateFalloff(lights[i], input);
                
                // Combine the final diffuse and specular values for this light
                total += ((balancedDiff * surfaceColor.rgb + specular) * lights[i].Intensity * lights[i].Color * attenuation) * fallOff;
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
    input.uv = input.uv * scale + offset;
	
	// Roughness and Metalness
    float roughness = RoughnessMap.Sample(Sample, input.uv).r;
    float metalness = MetalnessMap.Sample(Sample, input.uv).r;
	
	
	//input.normal = normalize(input.normal);	Older normal calculation
	// Unpack Normal Map
    float3 unpackedNormal = NormalMap.Sample(Sample, input.uv).rgb * 2 - 1;
    unpackedNormal = normalize(unpackedNormal);
    input.normal = TransformNormal(input, unpackedNormal);
	
	// Calculate Albedo Color
    float3 albedoColor = pow(Albedo.Sample(Sample, input.uv).rgb, 2.2f);
	
	// Specular color determination 
    float3 specularColor = lerp(F0_NON_METAL, albedoColor.rgb, metalness);
	
    float4 surfaceColor = colorTint * float4(albedoColor, 1);
	
	// Old Fresnel calculation
	float3 totalLight = CalculateLightingTotal(input, surfaceColor, specularColor, metalness, roughness);
	
	// Return pixel color
    return float4(pow(totalLight, 1.0f / 2.2f), 1);

}