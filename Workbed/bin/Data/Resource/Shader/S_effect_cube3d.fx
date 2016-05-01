#include "S_variables_shader.fx"

SamplerState linearSampling
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};


struct VS_INPUT_CUBE3D
{
	float4 Position : POSITION;
	float4 Normal : NORMAL;
	float4 Color : COLOR;
};

struct PS_INPUT_CUBE3D
{
	float4 Position : SV_POSITION;
	float4 Normal : NORMAL;
};

PS_INPUT_CUBE3D VS(VS_INPUT_CUBE3D input)
{
	PS_INPUT_CUBE3D output = (PS_INPUT_CUBE3D)0;

	output.Position = input.Position * float4(Scale, 1);

	output.Position = mul(output.Position, World);
	output.Position = mul(output.Position, View);
	output.Position = mul(output.Position, Projection);

	return output;
}

float4 PS(PS_INPUT_CUBE3D input) : SV_Target
{
	
	return Color;
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
