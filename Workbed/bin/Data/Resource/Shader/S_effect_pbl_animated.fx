#include "S_variables_pbl.fx"
#include "S_light_directional.fx"
#include "S_light_point.fx"
#include "S_light_spot.fx"

float4 AmbientHue;

matrix Bones[64];

struct VS_ANIMATION_INPUT
{
	float4 Position : POSITION;
	float3 Normal : NORMAL;
	float2 Tex : TEXCOORD;
	float3 BiNormal : BINORMAL;
	float3 Tangent : TANGENT;
	uint4 Bones : BONES;
	float4 Weights : WEIGHTS;
};

struct SkinnedVert
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float2 Tex : TEXCOORD;
	float3 BiNormal : BINORMAL;
	float3 Tangent : TANGENT;
	float4 WorldPosition : POSITION;
};

SkinnedVert VS(VS_ANIMATION_INPUT input)
{
	SkinnedVert output = (SkinnedVert)0;

	input.Position.xyz *= Scale;
	input.Position.w = 1.0f;
	
	output.Tex = input.Tex;

	float4 pos = input.Position;
	float3 norm = input.Normal;
	
	uint iBone;
	float fWeight;

	// Bone 0
	iBone = input.Bones.x;
	fWeight = input.Weights.x;

	output.Position += fWeight * mul(pos, Bones[iBone]);
	output.Normal += fWeight * mul(norm, (float3x3)Bones[iBone]);

	// Bone 1
	iBone = input.Bones.y;
	fWeight = input.Weights.y;

	output.Position += fWeight * mul(pos, Bones[iBone]);
	output.Normal += fWeight * mul(norm, (float3x3)Bones[iBone]);

	// Bone 2
	iBone = input.Bones.z;
	fWeight = input.Weights.z;

	output.Position += fWeight * mul(pos, Bones[iBone]);
	output.Normal += fWeight * mul(norm, (float3x3)Bones[iBone]);

	// Bone 3
	iBone = input.Bones.w;
	fWeight = input.Weights.w;

	output.Position += fWeight * mul(pos, Bones[iBone]);
	output.Normal += fWeight * mul(norm, (float3x3)Bones[iBone]);


	output.Position = mul(output.Position, World);
	output.Position = mul(output.Position, ViewProjection);

	output.Normal = mul(float4(input.Normal, 0), World).xyz;
	output.BiNormal = input.BiNormal;
	output.Tangent = mul(float4(input.Tangent, 0), World).xyz;

	output.WorldPosition = mul(input.Position, World);

	return output;
}

float4 PS_FUNCTION(SkinnedVert input)
{
	PixelData pixData = CalculatePixelData(input);

	float3 viewPos = cameraPosition;
	float3 toEye = normalize(viewPos - input.WorldPosition.xyz);

	float3 reflectionFresnel = ReflectionFresnel(pixData.mySubstance, pixData.myNormal.xyz,
		toEye, 1 - pixData.myRoughnessOffsetted);
	float3 ambientDiffuse = CubeMap.SampleLevel(linearSampling, pixData.myNormal.xyz, 9).xyz
		* pixData.myAmbientOcclusion * pixData.myMetalnessAlbedo * (1 - reflectionFresnel);
	float3 reflectionVector = reflect(toEye, pixData.myNormal.xyz);

	float fakeLysSpecularPower = RoughToSPow(pixData.myRoughness);
	float lysMipMap = GetSpecPowToMip(fakeLysSpecularPower, 12);

	float3 ambientSpec = CubeMap.SampleLevel(linearSampling, reflectionVector, lysMipMap).xyz
		* pixData.myAmbientOcclusion * reflectionFresnel;

	float3 ambient = saturate(ambientDiffuse);
	float3 finalColor = ambient.xyz * 0.8f;

	float3 directSpec = float3(0, 0, 0);


	//DirectionalLight
	//int i = 0;
	for(int i = 0; i < 1; ++i)
	{
		float3 lightDir = -normalize(DirectionalLights[i].Direction.xyz);

		float3 halfVec = normalize(lightDir + toEye.xyz);

		//float NdotL = saturate(dot(pixData.myNormal, lightDir));
		//float HdotN = saturate(dot(halfVec, pixData.myNormal));

		//float3 F = saturate(Fresnel(pixData.mySubstance, lightDir, halfVec));
		//float D = saturate(D_GGX(HdotN, (pixData.myRoughness + 1) / 2));
		//float V = saturate(V_SchlickForGGX((pixData.myRoughness + 1) / 2, dot(pixData.myNormal, toEye), NdotL));

		//directSpec += (((D * V * F) / 3.14159 + (1 - F) * NdotL * pixData.myMetalnessAlbedo * pixData.myAmbientOcclusion)) * DirectionalLights[i].Color.xyz * 2;

		float3 F = Fresnel(pixData.mySubstance, lightDir, halfVec);
		float D = D_GGX(dot(halfVec, pixData.myNormal), pixData.myRoughnessOffsetted);
		float V = V_SchlickForGGX(pixData.myRoughnessOffsetted, dot(pixData.myNormal, toEye), dot(pixData.myNormal, lightDir));
		directSpec += saturate(((F * D * V * dot(pixData.myNormal, lightDir)) + (1.f - F) / 3.14159f * dot(pixData.myNormal, lightDir) * pixData.myAlbedo) * DirectionalLights[i].Color);
	}
	
	//PointLight
	/*for(int i = 0; i < 1; ++i)
	{
		float3 toLight = PointLights[i].Position - input.WorldPosition;
		float3 lightDir = -normalize(toLight);

		float3 halfVec = normalize(lightDir + -toEye.xyz);

		float attenuation = Attenuation(toLight, 1000);

		float NdotL = saturate(dot(pixData.myNormal, toLight));
		float HdotN = saturate(dot(halfVec, pixData.myNormal));

		float3 F = saturate(Fresnel(pixData.mySubstance, lightDir, halfVec));
		float D = saturate(D_GGX(HdotN, (pixData.myRoughness + 1) / 2));
		float V = saturate(V_SchlickForGGX((pixData.myRoughness + 1) / 2, dot(pixData.myNormal, toEye), NdotL));

		directSpec += (((D * V * F) / 3.14159 + (1 - F) * NdotL * pixData.myMetalnessAlbedo * pixData.myAmbientOcclusion)) * PointLights[i].Color.xyz * attenuation;

		float lambert = dot(-lightDir, pixData.myNormal);
		finalColor += saturate(lambert * PointLights[i].Color.xyz * attenuation);
	}
	
	//SpotLight
	for(int i = 0; i < 0; ++i)
	{
		float3 toLight = SpotLights[i].Position - input.WorldPosition;
		float3 toLightDir = normalize(toLight);

		float3 halfVec = normalize(toLightDir + -toEye.xyz);

		float attenuation = saturate(Attenuation(toLightDir, SpotLights[i].Range));
		float angularAttenuation = saturate(AngularAttenuation(toLightDir, SpotLights[i].Direction, SpotLights[i].Cone));
		float totalAtt = saturate(attenuation * angularAttenuation);

		float NdotL = saturate(dot(pixData.myNormal, toLightDir));
		float HdotN = saturate(dot(halfVec, pixData.myNormal));

		float3 F = saturate(Fresnel(pixData.mySubstance, toLightDir, halfVec));
		float D = saturate(D_GGX(HdotN, (pixData.myRoughness + 1) / 2));
		float V = saturate(V_SchlickForGGX((pixData.myRoughness + 1) / 2, dot(pixData.myNormal, toEye), NdotL));

		directSpec += (((D * V * F) / 3.14159 + (1 - F) * NdotL * pixData.myMetalnessAlbedo * pixData.myAmbientOcclusion)* SpotLights[i].Color.xyz * totalAtt);

		float lambert = dot(toLightDir, pixData.myNormal);
		finalColor += saturate(lambert * SpotLights[i].Color.xyz * totalAtt);
	}*/

	finalColor = saturate(float4(finalColor + directSpec + ambientSpec, 1));
	
	/*const float minFullColorDistance = 2500.f;
	const float maxGrayDistance = 5500.f;
	float4 moddedGrayColor = float4(finalColor.xyz, 1);
	moddedGrayColor = float4(CubeMap.SampleLevel(linearSampling, normalize(cameraPosition - input.WorldPosition), 8).xyz, 1.f);

	float distanceToPixel = min(length(cameraPosition - input.WorldPosition), maxGrayDistance);
	distanceToPixel -= minFullColorDistance;
	distanceToPixel = max(distanceToPixel, 0.f);

	float grayDistance = min(distanceToPixel / (maxGrayDistance - minFullColorDistance), 0.60f);
	finalColor = lerp(finalColor, moddedGrayColor, grayDistance);*/
	
	return float4(finalColor, 1.f);
}

float4 PS(PS_INPUT_POS_NORM_TEX_BI_TANG input) : SV_Target
{
	return PS_FUNCTION(input);
}


float4 PS_EMISSIVE(PS_INPUT_POS_NORM_TEX_BI_TANG input) : SV_Target
{
	float4 finalColor = PS_FUNCTION(input);
	finalColor += EmissiveTexture.Sample(linearSampling, input.Tex);
	return saturate(finalColor);
}

float4 PS_NONE(PS_INPUT_POS_NORM_TEX_BI_TANG input) : SV_Target
{
	return float4(1, 1, 1, 1);
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

technique11 Render_Emissive
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_EMISSIVE()));
	}
}

technique11 Render_No_Pixel_Shader
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_NONE()));
	}
}