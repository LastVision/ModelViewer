#include "S_variables_shader.fx"


float4 SpritePositionAndScale;

SamplerState linearSampling
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Clamp;
	AddressV = Clamp;
};

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


PS_INPUT_POS_TEX VS(VS_INPUT_POS_TEX input)
{
	PS_INPUT_POS_TEX output = (PS_INPUT_POS_TEX)0;
	
	float4 pos = input.Pos;
	pos.x *= SpritePositionAndScale.z;
	pos.y *= SpritePositionAndScale.w;
	pos.x += SpritePositionAndScale.x;
	pos.y += SpritePositionAndScale.y;
	output.Pos = mul(pos, Projection);
	output.Pos.x -= 1;
	output.Pos.y += 1;
	output.Tex = input.Tex;

	return output;
}

float4 PS(PS_INPUT_POS_TEX input) : SV_Target
{
	float delta = 0.3f; // Testa ändra detta
	float rawAlpha = DiffuseTexture.Sample(linearSampling, input.Tex).a; // Viktigt med en kanal här. R går bra
	float4 color = DiffuseTexture.Sample(linearSampling, input.Tex); // Viktigt med en kanal här. R går bra
	float4 finalColor = float4(1, 1, 1, smoothstep(0.5f-delta,0.5f+delta,rawAlpha) ); // Smoothstep är den magiska funktionen
	//finalColor.a *= 0.5;
	finalColor.rgb = color.rgb;
	//finalColor.a *= input.color.a;
	return finalColor * Color;

	/*
	//somewhat working below
	float rawAlpha = DiffuseTexture.Sample(linearSampling, input.Tex).r;
	return float4(1, 1, 0, rawAlpha);
	float4 finalColor = float4(1, 1, 1, 1);// smoothstep(0.5f-delta,0.5f+delta,rawAlpha) ); // Smoothstep är den magiska funktionen
	

	if( rawAlpha < 0.5 )  
		finalColor.a = 0.0;
	else  
		finalColor.a = 1.0;

	finalColor.a *= smoothstep(0.25, 0.75, rawAlpha);

	float delta = 0.3f; // Testa ändra detta
	
	//finalColor.a *= input.color.a;
	
	//finalColor.a = rawAlpha;

	float4 OUTER_GLOW_COLOR = float4(0, 1, 0, 1);
	float onePixelDistance = 1.0/255.0;//value determined by distance field generation tool
	float SOFT_EDGE_MIN = 0.5 - onePixelDistance * 3.0;
	float SOFT_EDGE_MAX = 0.5 + onePixelDistance * 3.0;
	finalColor.a *= smoothstep( SOFT_EDGE_MIN, SOFT_EDGE_MAX, finalColor.a );
	//return finalColor;
	//float4 glowTexel = DiffuseTexture.Sample(linearSampling, input.Tex + GLOW_UV_OFFSET);
	float4 glowc = OUTER_GLOW_COLOR * smoothstep(SOFT_EDGE_MIN, SOFT_EDGE_MAX, rawAlpha);
	//finalColor = lerp(glowc, finalColor, finalColor.a);

	return finalColor;
	*/
}

technique11 Render
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
		SetBlendState(Blend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
	}
}