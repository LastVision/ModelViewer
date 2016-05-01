#include "S_variables_shader.fx"
#include "S_light_directional.fx"

float4 AmbientHue;

SamplerState linearSampling
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

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
	float4 returnColor = float4(0,0,1,0);
	float4 finalColor;
	finalColor = AlbedoTexture.Sample(linearSampling, input.Tex);

	returnColor.w = AmbientHue.w * finalColor.w;
	return returnColor;
}

BlendState Blend
{
	BlendEnable[0] = TRUE;
	SrcBlend = SRC_ALPHA;
	DestBlend = INV_SRC_ALPHA;
	BlendOp = ADD;
	SrcBlendAlpha = ONE;
	DestBlendAlpha = ONE;
	BlendOpAlpha = ADD;
	RenderTargetWriteMask[0] = 0x0F;
};

technique11 Render_1UVSET
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
		SetBlendState(Blend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
	}
}