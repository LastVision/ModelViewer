#include "S_variables_shader.fx"
#include "S_light_directional.fx"
#include "S_light_point.fx"
#include "S_light_spot.fx"

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
};

SamplerState linearSampling
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

SkinnedVert VS(VS_ANIMATION_INPUT input)
{
	SkinnedVert output = (SkinnedVert)0;

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

	return output;
}

float4 PS(SkinnedVert input) : SV_Target
{
	float ambient = 0.3f;
	float4 finalColor = float4(ambient, ambient, ambient, 1.f);
	return finalColor;
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