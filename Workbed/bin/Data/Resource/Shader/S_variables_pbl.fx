#include "S_variables_shader.fx"

//Variabels from Lys
static const float k0 = 0.00098f;
static const float k1 = 0.9921f;
static const float fakeLysMaxSpecularPower = (2.f / (0.0014f * 0.0014f)) - 2.f;
static const float fMaxT = (exp2(-10.f / sqrt((2.f / (0.0014f * 0.0014f)) - 2.f)) - 0.00098f) / 0.9921f;

float3 cameraPosition;

SamplerState linearSampling
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct PBLVertexShaderData
{
	float4 Position;
	float3 Normal;
	float2 Tex;
	float3 BiNormal;
	float3 Tangent;
	float4 WorldPosition;
};

PBLVertexShaderData CalcVertexShaderData(float4 aPosition, float3 aNormal
, float2 aTex, float3 aBiNormal, float3 aTangent)
{
	PBLVertexShaderData output = (PBLVertexShaderData)0;
	aPosition.xyz *= Scale;
	aPosition.w = 1.0f;

	Matrix mat = mul(World, ViewProjection);
	output.Position = mul(aPosition, mat);
	output.Tex = aTex;
	
	output.Normal = mul(float4(aNormal, 0), World).xyz;
	output.BiNormal = aBiNormal;
	output.Tangent = mul(float4(aTangent, 0), World).xyz;
	output.WorldPosition = mul(aPosition, World);

	return output;
}

struct PixelData
{
	float3 myAlbedo;
	float3 myMetalness;
	float3 myMetalnessAlbedo;
	float3 mySubstance;
	float3 myNormal;
	float3 myAmbientOcclusion;
	float myRoughness;
	float myRoughnessOffsetted;
};

struct PixelDataA
{
	float4 myAlbedo;
	float3 myMetalness;
	float3 myMetalnessAlbedo;
	float3 mySubstance;
	float3 myNormal;
	float3 myAmbientOcclusion;
	float myRoughness;
	float myRoughnessOffsetted;
};

PixelDataA CalculatePixelData1UV(PS_INPUT_1UVSET aInput)
{
	PixelDataA newData;

	//Calculate Normal
	newData.myNormal = NormalTexture.Sample(linearSampling, aInput.Tex) * 2 - 1;

	aInput.Normal = normalize(aInput.Normal);
	aInput.BiNormal = normalize(aInput.BiNormal);
	aInput.Tangent = normalize(aInput.Tangent);

	float3x3 tangentSpaceMatrix = float3x3(aInput.Tangent, aInput.BiNormal, aInput.Normal);
		newData.myNormal = normalize(mul(newData.myNormal, tangentSpaceMatrix));


	//Calculate Albedo and Metalness
	newData.myAlbedo = AlbedoTexture.Sample(linearSampling, aInput.Tex);
	newData.myMetalness = MetalnessTexture.Sample(linearSampling, aInput.Tex);
	newData.myMetalnessAlbedo = newData.myAlbedo - newData.myAlbedo * newData.myMetalness;

	//Calculate Substance
	newData.mySubstance = (0.04 - 0.04 * newData.myMetalness) + newData.myAlbedo * newData.myMetalness;

	//Calculate AO
	newData.myAmbientOcclusion = AOTexture.Sample(linearSampling, aInput.Tex);

	//Calculate Roughness
	newData.myRoughness = RoughnessTexture.Sample(linearSampling, aInput.Tex).x;
	newData.myRoughnessOffsetted = pow(8192, newData.myRoughness);

	return newData;
}

PixelData CalculatePixelData2UV(PS_INPUT_2UVSET aInput)
{
	PixelData newData;

	//Calculate Normal
	newData.myNormal = NormalTexture.Sample(linearSampling, aInput.Tex) * 2 - 1;

	aInput.Normal = normalize(aInput.Normal);
	aInput.BiNormal = normalize(aInput.BiNormal);
	aInput.Tangent = normalize(aInput.Tangent);

	float3x3 tangentSpaceMatrix = float3x3(aInput.Tangent, aInput.BiNormal, aInput.Normal);
		newData.myNormal = normalize(mul(newData.myNormal, tangentSpaceMatrix));


	//Calculate Albedo and Metalness
	newData.myAlbedo = AlbedoTexture.Sample(linearSampling, aInput.Tex);
	newData.myMetalness = MetalnessTexture.Sample(linearSampling, aInput.Tex);
	newData.myMetalnessAlbedo = newData.myAlbedo - newData.myAlbedo * newData.myMetalness;

	//Calculate Substance
	newData.mySubstance = (0.04 - 0.04 * newData.myMetalness) + newData.myAlbedo * newData.myMetalness;

	//Calculate AO
	newData.myAmbientOcclusion = AOTexture.Sample(linearSampling, aInput.Tex2);

	//Calculate Roughness
	newData.myRoughness = RoughnessTexture.Sample(linearSampling, aInput.Tex).x;
	newData.myRoughnessOffsetted = pow(8192, newData.myRoughness);

	return newData;
}

PixelData CalculatePixelData1UVColor(PS_INPUT_1UVSET_COLOR aInput)
{
	PixelData newData;

	//Calculate Normal
	newData.myNormal = NormalTexture.Sample(linearSampling, aInput.Tex) * 2 - 1;

	aInput.Normal = normalize(aInput.Normal);
	aInput.BiNormal = normalize(aInput.BiNormal);
	aInput.Tangent = normalize(aInput.Tangent);

	float3x3 tangentSpaceMatrix = float3x3(aInput.Tangent, aInput.BiNormal, aInput.Normal);
		newData.myNormal = normalize(mul(newData.myNormal, tangentSpaceMatrix));


	//Calculate Albedo and Metalness
	newData.myAlbedo = AlbedoTexture.Sample(linearSampling, aInput.Tex);

	float3 snowColor = float3(0.9f,0.9f,0.9f);
	float lerpMix = 0.f;
	float3 worldNormal = float3(aInput.Normal.y, aInput.Normal.y, aInput.Normal.y);
	float3 vertexColor = aInput.Color.rgb;
	lerpMix = (vertexColor.x + worldNormal) * (1 - vertexColor.y);

	newData.myAlbedo = lerp(newData.myAlbedo, snowColor, lerpMix);

	newData.myMetalness = MetalnessTexture.Sample(linearSampling, aInput.Tex);
	newData.myMetalnessAlbedo = newData.myAlbedo - newData.myAlbedo * newData.myMetalness;

	//Calculate Substance
	newData.mySubstance = (0.04 - 0.04 * newData.myMetalness) + newData.myAlbedo * newData.myMetalness;

	//Calculate AO
	newData.myAmbientOcclusion = AOTexture.Sample(linearSampling, aInput.Tex);

	//Calculate Roughness
	newData.myRoughness = RoughnessTexture.Sample(linearSampling, aInput.Tex).x;
	newData.myRoughnessOffsetted = pow(8192, newData.myRoughness);

	return newData;
}

PixelData CalculatePixelData2UVColor(PS_INPUT_2UVSET_COLOR aInput)
{
	PixelData newData;

	//Calculate Normal
	newData.myNormal = NormalTexture.Sample(linearSampling, aInput.Tex) * 2 - 1;

	aInput.Normal = normalize(aInput.Normal);
	aInput.BiNormal = normalize(aInput.BiNormal);
	aInput.Tangent = normalize(aInput.Tangent);

	float3x3 tangentSpaceMatrix = float3x3(aInput.Tangent, aInput.BiNormal, aInput.Normal);
		newData.myNormal = normalize(mul(newData.myNormal, tangentSpaceMatrix));


	//Calculate Albedo and Metalness
	newData.myAlbedo = AlbedoTexture.Sample(linearSampling, aInput.Tex);
	newData.myMetalness = MetalnessTexture.Sample(linearSampling, aInput.Tex);
	newData.myMetalnessAlbedo = newData.myAlbedo - newData.myAlbedo * newData.myMetalness;

	//Calculate Substance
	newData.mySubstance = (0.04 - 0.04 * newData.myMetalness) + newData.myAlbedo * newData.myMetalness;

	//Calculate AO
	newData.myAmbientOcclusion = AOTexture.Sample(linearSampling, aInput.Tex);

	//Calculate Roughness
	newData.myRoughness = RoughnessTexture.Sample(linearSampling, aInput.Tex).x;
	newData.myRoughnessOffsetted = pow(8192, newData.myRoughness);

	return newData;
}

PixelData CalculatePixelData(PS_INPUT_POS_NORM_TEX_BI_TANG aInput)
{
	PixelData newData;

	//Calculate Normal
	newData.myNormal = NormalTexture.Sample(linearSampling, aInput.Tex) * 2 - 1;

	aInput.Normal = normalize(aInput.Normal);
	aInput.BiNormal = normalize(aInput.BiNormal);
	aInput.Tangent = normalize(aInput.Tangent);

	float3x3 tangentSpaceMatrix = float3x3(aInput.Tangent, aInput.BiNormal, aInput.Normal);
		newData.myNormal = normalize(mul(newData.myNormal, tangentSpaceMatrix));


	//Calculate Albedo and Metalness
	newData.myAlbedo = AlbedoTexture.Sample(linearSampling, aInput.Tex);
	newData.myMetalness = MetalnessTexture.Sample(linearSampling, aInput.Tex);
	newData.myMetalnessAlbedo = newData.myAlbedo - newData.myAlbedo * newData.myMetalness;

	//Calculate Substance
	newData.mySubstance = (0.04 - 0.04 * newData.myMetalness) + newData.myAlbedo * newData.myMetalness;

	//Calculate AO
	newData.myAmbientOcclusion = AOTexture.Sample(linearSampling, aInput.Tex);

	//Calculate Roughness
	newData.myRoughness = RoughnessTexture.Sample(linearSampling, aInput.Tex).x;
	newData.myRoughnessOffsetted = pow(8192, newData.myRoughness);

	return newData;
}

float3 ReflectionFresnel(const float3 aSubstance, const float3 aLightDir, const float3 aHalfVec, float aRoughness)
{
	return (aSubstance + (1.f - aSubstance) * pow((1.f - saturate(dot(aLightDir, aHalfVec))), 5) / (4 - 3 * aRoughness));
}

float RoughToSPow(float fRoughness)
{
	return (2.f / (fRoughness * fRoughness)) - 2.f;
}

float GetSpecPowToMip(float fSpecPow, int nMips)
{
	float fSmulMaxT = (exp2(-10.0 / sqrt(fSpecPow)) - k0) / k1;

	return float(nMips - 1 - 0) * (1.0 - clamp(fSmulMaxT / fMaxT, 0.0, 1.0));
}





float3 Fresnel(const float3 aSubstance, const float3 aLightDir, const float3 aHalfVec)
{
	return (aSubstance + (1.f - aSubstance) * pow((1.f - saturate(dot(aLightDir, aHalfVec))), 5));
}

float V_SchlickForGGX(float aRoughness, float aNdotV, float aNdotL)
{
	float k = aRoughness * aRoughness * 0.5f;
	float G1V = aNdotV * (1.f - k) + k;
	float G1L = aNdotL * (1.f - k) + k;

	return 0.25f / (G1V * G1L);
}

float D_GGX(float aHdotN, float aRoughness)
{
	float m = aRoughness*aRoughness;
	float m2 = m*m;
	float Denominator = aHdotN*aHdotN*(m2 - 1) + 1;
	float D = m2 / (3.14159*Denominator*Denominator);
	return D;
}

float GGXifyColor(float3 aLightVec, float3 aViewDir, float3 aLambert, PixelData pixData)
{
	float3 halfVec = (aLightVec + aViewDir) / 2.f;
	float3 normal = pixData.myNormal;
	float NdotL = dot(pixData.myNormal, aLightVec);

	//F(l, h)
	float3 F = Fresnel(pixData.mySubstance, aLightVec, halfVec);


	//G(l, v, h)
	float V = V_SchlickForGGX(pixData.myRoughnessOffsetted, dot(pixData.myNormal, aViewDir), NdotL);


	//D(h)
	float D = D_GGX(dot(halfVec, pixData.myNormal), pixData.myRoughnessOffsetted);


	return ((F * D * V * NdotL) + (1.f - F) / 3.14159f * NdotL * pixData.myAlbedo);
}