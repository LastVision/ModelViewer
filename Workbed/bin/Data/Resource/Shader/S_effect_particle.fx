matrix World;
matrix View;
matrix Projection;

float Time; //Delta

Texture2D DiffuseTexture;

shared SamplerState Sampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct VS_INPUT
{
	float3 Pos		: POSITION;
	float3 Color	: COLOR;
	float Alpha		: ALPHA;
	float Size		: SIZE;
	float Time		: TIME;
	float Rotation 	: ROTATION;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float3 Normal : NORMAL;
	float2 Uv : TEXCOORD;
	float Size : SIZE;
	float Alpha : ALPHA;
	float Time : TIME;
	float3 Color : COLOR;
	float Rotation : ROTATION;
};

RasterizerState MyWireframe
{
	CullMode = NONE;
};



PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.Pos = float4(input.Pos, 1);
	output.Size = input.Size;
	output.Alpha = input.Alpha;
	output.Time = input.Time;
	output.Pos = mul(output.Pos, World);
	output.Pos = mul(output.Pos, View);
	output.Time = input.Time;
	output.Color = input.Color;
	output.Rotation = input.Rotation;
	return output;
}

cbuffer cbImmutable
{
	float4 g_positions[4] =
	{
		float4(-1, 1, 0, 0),
		float4(1, 1, 0, 0),
		float4(-1, -1, 0, 0),
		float4(1, -1, 0, 0),
	};
	float2 g_texcoords[4] =
	{
		float2(0, 1),
		float2(1, 1),
		float2(0, 0),
		float2(1, 0),
	};
};


[maxvertexcount(4)]
void GS (point PS_INPUT input[1], inout TriangleStream<PS_INPUT> triStream)
{
	PS_INPUT output = (PS_INPUT)0;

	for (int i = 0; i < 4; ++i)
	{
		float3 rotation = 0;
		rotation.x = g_positions[i].x * cos(input[0].Rotation) - sin (input[0].Rotation) * g_positions[i].y;
		rotation.y = sin(input[0].Rotation) * g_positions[i].x + cos(input[0].Rotation) * g_positions[i].y;

		output.Pos = float4(float4(input[0].Size * rotation, 0) + input[0].Pos);
		output.Pos = mul(output.Pos, Projection);
		output.Uv = g_texcoords[i];
		output.Alpha = input[0].Alpha;
		output.Color = input[0].Color;
		triStream.Append(output);
	}

	triStream.RestartStrip();
}


float4 PS(PS_INPUT input) : SV_Target
{
	float4 color = DiffuseTexture.Sample(Sampler, input.Uv);
	float4 currCol = float4(color.rgb * input.Color.rgb, color.a);
	currCol.a = currCol.a * input.Alpha;
	return currCol;
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
		SetGeometryShader(CompileShader(gs_5_0, GS()));
		SetPixelShader(CompileShader(ps_5_0, PS()));
		SetBlendState(Blend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetDepthStencilState(DisableDepthWrites, 0);
	}
}
