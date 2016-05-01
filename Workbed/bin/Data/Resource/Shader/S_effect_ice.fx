#include "S_variables_pbl.fx"
#include "S_light_directional.fx"



#define SPLAT_BASE 0
#define SPLAT_RED 1
#define SPLAT_GREEN 2
#define SPLAT_BLUE 3
#define SPLAT_ALPHA 4

Texture2D myAlbedoTextures[2];
Texture2D myMetalnessTextures[2];
Texture2D myRoughnessTextures[2];
Texture2D myNormalTextures[2];
Texture2D myAmbientOcclusionTextures[2];

PS_INPUT_POS_NORM_TEX_BI_TANG VS(VS_INPUT_POS_NORM_TEX_BI_TANG input)
{
	PS_INPUT_POS_NORM_TEX_BI_TANG output = (PS_INPUT_POS_NORM_TEX_BI_TANG)0;

	output.Position = mul(input.Position, World);
	output.Position = mul(output.Position, ViewProjection);

	output.Tex = input.Tex;

	output.Normal = mul(float4(input.Normal, 0), World).xyz;
	output.BiNormal = input.BiNormal;
	output.Tangent = mul(float4(input.Tangent, 0), World).xyz;

	output.WorldPosition = mul(input.Position, World);


	return output;
}

float4 PS(PS_INPUT_POS_NORM_TEX_BI_TANG input) : SV_Target
{
	float4 influenceColor = AlbedoTexture.Sample(linearSampling, input.Tex);

	float redWeight = influenceColor.r / (influenceColor.r + influenceColor.g + influenceColor.b + 1.f);
	float baseWeight = saturate(1.f - (influenceColor.r + influenceColor.g + influenceColor.b + influenceColor.a)) / (influenceColor.r + influenceColor.g + influenceColor.b + 1.f);

	int tileAmount = 20;
	float4 splatR = myAlbedoTextures[SPLAT_RED].Sample(linearSampling, input.Tex * tileAmount);
	float4 splatBase = myAlbedoTextures[SPLAT_BASE].Sample(linearSampling, input.Tex * tileAmount);

	float alphaR = myAlbedoTextures[SPLAT_RED].Sample(linearSampling, input.Tex * tileAmount).a;
	float alphaBase = myAlbedoTextures[SPLAT_BASE].Sample(linearSampling, input.Tex * tileAmount).a;

	float blendThreshold = 0.15;

	float lerp4Last = baseWeight * alphaBase + baseWeight;
	float lerp4Current = redWeight * alphaR + redWeight;
	float lerp4Compare = saturate((lerp4Last - lerp4Current) / blendThreshold);
	float3 lerp4 = lerp(splatR.rgb, splatBase.rgb, lerp4Compare);

	float3 finalAlbedo = lerp4;

		//METALNESS
	splatR = myMetalnessTextures[SPLAT_RED].Sample(linearSampling, input.Tex * tileAmount);
	splatBase = myMetalnessTextures[SPLAT_BASE].Sample(linearSampling, input.Tex * tileAmount);

	lerp4Last = baseWeight * alphaBase + baseWeight;
	lerp4Current = redWeight * alphaR + redWeight;
	lerp4Compare = saturate((lerp4Last - lerp4Current) / blendThreshold);
	lerp4 = lerp(splatR.rgb, splatBase.rgb, lerp4Compare);


	float3 finalMetalness = lerp4;

		//ROUGHNESS
	splatR = myRoughnessTextures[SPLAT_RED].Sample(linearSampling, input.Tex * tileAmount);
	splatBase = myRoughnessTextures[SPLAT_BASE].Sample(linearSampling, input.Tex * tileAmount);

	lerp4Last = baseWeight * alphaBase + baseWeight;
	lerp4Current = redWeight * alphaR + redWeight;
	lerp4Compare = saturate((lerp4Last - lerp4Current) / blendThreshold);
	lerp4 = lerp(splatR.rgb, splatBase.rgb, lerp4Compare);


	float3 finalRoughness = lerp4;


	//NORMALMAP
	splatR = myNormalTextures[SPLAT_RED].Sample(linearSampling, input.Tex * tileAmount);
	splatBase = myNormalTextures[SPLAT_BASE].Sample(linearSampling, input.Tex * tileAmount);

	lerp4Last = baseWeight * alphaBase + baseWeight;
	lerp4Current = redWeight * alphaR + redWeight;
	lerp4Compare = saturate((lerp4Last - lerp4Current) / blendThreshold);
	lerp4 = lerp(splatR.rgb, splatBase.rgb, lerp4Compare);

	float3 finalNormal = lerp4;

	//AO
	splatR = myAmbientOcclusionTextures[SPLAT_RED].Sample(linearSampling, input.Tex * tileAmount);
	splatBase = myAmbientOcclusionTextures[SPLAT_BASE].Sample(linearSampling, input.Tex * tileAmount);

	lerp4Last = baseWeight * alphaBase + baseWeight;
	lerp4Current = redWeight * alphaR + redWeight;
	lerp4Compare = saturate((lerp4Last - lerp4Current) / blendThreshold);
	lerp4 = lerp(splatR.rgb, splatBase.rgb, lerp4Compare);

	float3 finalAO = lerp4;



	//Pixeldata skall vara outputen från alla lerps
	PixelData pixData = (PixelData)0;

	pixData.myNormal = finalNormal * 2 - 1;

	float3x3 tangentSpaceMatrix = float3x3(input.Tangent, input.BiNormal, input.Normal);
		pixData.myNormal = normalize(mul(pixData.myNormal, tangentSpaceMatrix));

	//Calculate Albedo and Metalness
	pixData.myAlbedo = finalAlbedo;
	pixData.myMetalness = finalMetalness;
	pixData.myMetalnessAlbedo = pixData.myAlbedo - pixData.myAlbedo * pixData.myMetalness;

	//Calculate Substance
	pixData.mySubstance = (0.04f - 0.04f * pixData.myMetalness) + pixData.myAlbedo * pixData.myMetalness;

	//Calculate AO
	pixData.myAmbientOcclusion = finalAO;

	//Calculate Roughness
	pixData.myRoughness = finalRoughness.x;
	pixData.myRoughnessOffsetted = pow(8192, pixData.myRoughness);


	float3 viewPos = cameraPosition;
	float3 toEye = normalize(viewPos - input.WorldPosition.xyz);

	float3 reflectionFresnel = ReflectionFresnel(pixData.mySubstance, pixData.myNormal.xyz,
		toEye, 1 - pixData.myRoughness);
	float3 ambientDiffuse = CubeMap.SampleLevel(linearSampling, pixData.myNormal.xyz, 9).xyz
		* pixData.myAmbientOcclusion * pixData.myMetalnessAlbedo * (1 - reflectionFresnel);
	float3 reflectionVector = reflect(toEye, pixData.myNormal.xyz);

	float fakeLysSpecularPower = RoughToSPow(pixData.myRoughness);
	float lysMipMap = GetSpecPowToMip(fakeLysSpecularPower, 12);

	float3 ambientSpec = CubeMap.SampleLevel(linearSampling, reflectionVector, lysMipMap).xyz
		* pixData.myAmbientOcclusion * reflectionFresnel;

	float3 ambient = saturate(ambientDiffuse);
	float3 finalColor = ambient.xyz;

	float3 directSpec = float3(0, 0, 0);


		//DirectionalLight
		for (int i = 0; i < 1; ++i)
		{
		//finalColor += CalculateDirectionalLight(DirectionalLights[i], input.Normal);

		float3 lightDir = -normalize(DirectionalLights[i].Direction.xyz);
		float3 halfVec = normalize(lightDir + toEye.xyz);

		float3 F = Fresnel(pixData.mySubstance, lightDir, halfVec);
		float D = D_GGX(dot(halfVec, pixData.myNormal), pixData.myRoughnessOffsetted);
		float V = V_SchlickForGGX(pixData.myRoughnessOffsetted, dot(pixData.myNormal, toEye), dot(pixData.myNormal, lightDir));
		directSpec += saturate(((F * D * V * dot(pixData.myNormal, lightDir)) + (1.f - F) / 3.14159f * dot(pixData.myNormal, lightDir) * pixData.myAlbedo) * DirectionalLights[i].Color);

		}
	//finalColor = saturate(finalColor * ambient);
	finalColor = saturate(float4(finalColor + directSpec + ambientSpec, 1).rgb);

	return float4(finalColor, 1.f);
}

technique11 Render
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}