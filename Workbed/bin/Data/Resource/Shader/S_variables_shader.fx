Matrix World;
Matrix View;
Matrix Projection;
Matrix ViewProjection;

Texture2D DiffuseTexture;

Texture2D AlbedoTexture;
Texture2D NormalTexture;
Texture2D RoughnessTexture;
Texture2D MetalnessTexture;
Texture2D AOTexture;
Texture2D EmissiveTexture;

TextureCube CubeMap;

float3 Scale;
float4 Color;

struct VS_INPUT_POS_NORM_TEX
{
	float4 Pos : POSITION;
	float3 Norm : NORMAL;
	float2 Tex : TEXCOORD;
};

struct PS_INPUT_POS_NORM_TEX
{
	float4 Pos : SV_POSITION;
	float3 Norm : NORMAL;
	float2 Tex : TEXCOORD;
};

struct VS_INPUT_1UVSET
{
	float4 Position : POSITION;
	float3 Normal : NORMAL;
	float2 Tex : TEXCOORD;
	float3 BiNormal : BINORMAL;
	float3 Tangent : TANGENT;
};

struct PS_INPUT_1UVSET
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float2 Tex : TEXCOORD;
	float3 BiNormal : BINORMAL;
	float3 Tangent : TANGENT;
	float4 WorldPosition : POSITION;
};

struct VS_INPUT_1UVSET_COLOR
{
	float4 Position : POSITION;
	float3 Normal : NORMAL;
	float2 Tex : TEXCOORD;
	float3 BiNormal : BINORMAL;
	float3 Tangent : TANGENT;
	float4 Color : COLOR;
};

struct PS_INPUT_1UVSET_COLOR
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float2 Tex : TEXCOORD;
	float3 BiNormal : BINORMAL;
	float3 Tangent : TANGENT;
	float4 Color : COLOR;
	float4 WorldPosition : POSITION;
};

struct VS_INPUT_2UVSET
{
	float4 Position : POSITION;
	float3 Normal : NORMAL;
	float2 Tex : TEXCOORD0;
	float2 Tex2 : TEXCOORD1;
	float3 BiNormal : BINORMAL;
	float3 Tangent : TANGENT;
};

struct PS_INPUT_2UVSET
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float2 Tex : TEXCOORD0;
	float2 Tex2 : TEXCOORD1;
	float3 BiNormal : BINORMAL;
	float3 Tangent : TANGENT;
	float4 WorldPosition : POSITION;
};

struct VS_INPUT_2UVSET_COLOR
{
	float4 Position : POSITION;
	float3 Normal : NORMAL;
	float2 Tex : TEXCOORD0;
	float2 Tex2 : TEXCOORD1;
	float3 BiNormal : BINORMAL;
	float3 Tangent : TANGENT;
	float4 Color : COLOR;
};

struct PS_INPUT_2UVSET_COLOR
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float2 Tex : TEXCOORD0;
	float2 Tex2 : TEXCOORD1;
	float3 BiNormal : BINORMAL;
	float3 Tangent : TANGENT;
	float4 Color : COLOR;
	float4 WorldPosition : POSITION;
};

struct VS_INPUT_POS_NORM_TEX_BI_TANG
{
	float4 Position : POSITION;
	float3 Normal : NORMAL;
	float2 Tex : TEXCOORD;
	float3 BiNormal : BINORMAL;
	float3 Tangent : TANGENT;
};

struct PS_INPUT_POS_NORM_TEX_BI_TANG
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float2 Tex : TEXCOORD;
	float3 BiNormal : BINORMAL;
	float3 Tangent : TANGENT;
	float4 WorldPosition : POSITION;
};

struct VS_INPUT_POS_TEX
{
	float4 Pos : POSITION;
	float2 Tex : TEXCOORD;
};

struct PS_INPUT_POS_TEX
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
};



struct VS_INPUT_POS_COL
{
	float4 Pos : POSITION;
	float4 Color : COLOR;
};

struct PS_INPUT_POS_COL
{
	float4 Pos : SV_POSITION;
	float4 Color : COLOR;
};



struct VS_INPUT_POS_NORM_COL
{
	float4 Pos : POSITION;
	float3 Norm : NORMAL;
	float4 Color : COLOR;
};

struct PS_INPUT_POS_NORM_COL
{
	float4 Pos : SV_POSITION;
	float3 Norm : NORMAL;
	float4 Color : COLOR;
};


struct VS_INPUT_POS_COL_UV
{
	float4 Pos : POSITION;
	float4 Color : COLOR;
	float2 Tex : TEXCOORD;
};

struct PS_INPUT_POS_COL_UV
{
	float4 Pos : SV_POSITION;
	float4 Color : COLOR;
	float2 Tex : TEXCOORD;
};


float3 Lambert(float3 aLightDirction, float3 aNormal, float3 aColor)
{
	float lambert = dot(aLightDirction, aNormal);
	float3 lightColor = saturate(lambert * aColor);

	return lightColor;
}

float Attenuation(float3 aLightVec, float aRange)
{
	float distance = length(aLightVec);
	float attenuation = 1.f / (1.f + 0.1f * distance + 0.01f * distance * distance);
	float fallOff = 1.f - (distance / (aRange + 0.00001f));
	return attenuation * fallOff;
}

float AngularAttenuation(float3 aLightVec, float3 aLightDirection, float aLightCone)
{
	float angularAttenuation = dot(-aLightVec, aLightDirection);
	angularAttenuation -= 1.f - aLightCone;
	angularAttenuation *= 1.f / aLightCone;

	return angularAttenuation;
}