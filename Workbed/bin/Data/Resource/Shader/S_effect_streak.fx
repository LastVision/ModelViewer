matrix World;
matrix View;
matrix Projection;

float StreakAlphaDelta;
float StreakStartAlpha;
float StreakSizeDelta;

Texture2D DiffuseTexture;

shared SamplerState Sampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct VS_INPUT
{
	float3 Pos : POSITION;
	float3 Velocity : VELOCITY;
	float Size : SIZE;
	float Alpha : ALPHA;
	float Time : TIME;
	float Rotation : ROTATION;
	float RotationDelta : ROTATION_DELTA;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	//float3 Normal : NORMAL;
	float2 Uv : TEXCOORD;
	float Size : SIZE;
	float Alpha : ALPHA;
	float Time : TIME;
	//float Rotation : ROTATION;
};

RasterizerState CullNothing
{
//	Fillmode = Wireframe;
	CullMode = NONE;
};



PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.Pos = float4(input.Pos,1);
	output.Size = input.Size;
	output.Alpha = input.Alpha + input.Time * StreakAlphaDelta;
	//output.Time = input.Time;
	output.Pos = mul(output.Pos, World);
	output.Pos = mul(output.Pos, View);
	//output.Pos = mul(output.Pos, Projection);
	output.Time = input.Time;
	//output.Rotation = input.Rotation;
	return output;
}

cbuffer cbImmutable
{
	float3 g_positions[4] =
	{
		float3(-1, 1, 0),
		float3(1, 1, 0),
		float3(-1, -1, 0),
		float3(1, -1, 0),
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
void GS
(lineadj PS_INPUT input[4],
inout TriangleStream<PS_INPUT> triStream)
{
	//float4 p0 = input[1].Pos;
	//float4 p1 = input[2].Pos;


	//float w0 = p0.w;
	//float w1 = p1.w;

	//p0.xyz /= w0;
	//p1.xyz /= w1;

	//float3 toEye = float3(0, 0, -1);
	float3 toEye = float3(View._41, View._42, View._43) - input[1].Pos.xyz;

	float3 line02 = input[2].Pos.xyz - input[0].Pos.xyz;
	float3 dir1 = normalize(line02);

	float3 normal1 = normalize(cross(normalize(toEye), dir1));

	float3 line13 = input[3].Pos.xyz - input[1].Pos.xyz;
	float3 dir2 = normalize(line13);

	float3 normal2 = normalize(cross(normalize(toEye), dir2));

	float uvMult = 5;
	float uvSegment = 1;
	PS_INPUT output[4];// = (PS_INPUT)0;
	
	output[0].Size = 1; //dummy value
	output[1].Size = 1; //dummy value
	output[2].Size = 1; //dummy value
	output[3].Size = 1; //dummy value

	output[0].Pos = float4(input[1].Pos.xyz - normal1 * input[1].Size, 1);
	output[0].Pos = mul(output[0].Pos, Projection);
	output[0].Uv = float2(0, 1);
	output[0].Alpha = input[1].Alpha;
	output[0].Time = input[1].Time;

	output[1].Pos = float4(input[2].Pos.xyz - normal2 * input[2].Size, 1);
	output[1].Pos = mul(output[1].Pos, Projection);
	output[1].Uv = float2(0 + uvSegment, 1);
	output[1].Alpha = input[2].Alpha;
	output[1].Time = input[2].Time;

	output[2].Pos = float4(input[1].Pos.xyz + normal1 * input[1].Size, 1);
	output[2].Pos = mul(output[2].Pos, Projection);
	output[2].Uv = float2(0, 0);
	output[2].Alpha = input[1].Alpha;
	output[2].Time = input[1].Time;

	output[3].Pos = float4(input[2].Pos.xyz + normal2 * input[2].Size, 1);
	output[3].Pos = mul(output[3].Pos, Projection);
	output[3].Uv = float2(0 + uvSegment, 0);
	output[3].Alpha = input[2].Alpha;
	output[3].Time = input[2].Time;


	triStream.Append(output[0]);
	triStream.Append(output[1]);
	triStream.Append(output[2]);
	triStream.Append(output[3]);

	triStream.RestartStrip();
}


float4 PS(PS_INPUT input) : SV_Target
{
	float4 color = DiffuseTexture.Sample(Sampler, input.Uv);
	color.w = color.w * input.Alpha;
	//color.w = 1.f;
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

technique10 Render
{
	pass P0
	{
		SetRasterizerState(CullNothing);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(CompileShader(gs_5_0, GS()));
		SetPixelShader(CompileShader(ps_5_0, PS()));
		SetBlendState(Blend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetDepthStencilState(DisableDepthWrites, 0);
		//SetBlendState(NoBlend, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
	}
}



