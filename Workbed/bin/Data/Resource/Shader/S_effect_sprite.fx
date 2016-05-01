#include "S_variables_shader.fx"

float4 SpritePositionAndScale;

Matrix SpriteOrientation;

SamplerState linearSampling
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};


PS_INPUT_POS_TEX VS(VS_INPUT_POS_TEX input)
{
	PS_INPUT_POS_TEX output = (PS_INPUT_POS_TEX)0;

	float4 pos = input.Pos;
	pos = mul(pos, SpriteOrientation);
	pos.x *= SpritePositionAndScale.z;
	pos.y *= SpritePositionAndScale.w;
	pos.x += SpritePositionAndScale.x;
	pos.y += SpritePositionAndScale.y;
	output.Pos = mul(pos, Projection);
	output.Pos.x -= 1;
	output.Pos.y -= 1;
	output.Tex = input.Tex;

	return output;
}

float4 PS(PS_INPUT_POS_TEX input) : SV_Target
{
	float4 color = DiffuseTexture.Sample(linearSampling, input.Tex);
	color *= Color;
	return color;
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

BlendState NoBlend
{
	AlphaToCoverageEnable = FALSE;
	BlendEnable[0] = FALSE;
};

DepthStencilState DisableDepthWrites
{
	DepthEnable = TRUE;
	DepthWriteMask = ZERO;
};

technique11 Render
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
		SetBlendState(Blend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		//SetDepthStencilState(DisableDepthWrites, 0);
	}
}
