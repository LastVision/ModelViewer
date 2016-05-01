struct SPOT_LIGHT
{
	float4 Position;
	float4 Direction;
	float4 Color;
	float Range;
	float Cone;
};

SPOT_LIGHT SpotLights[3];


float4 CalculateSpotLight(SPOT_LIGHT aLight, float3 aNormal, float4 aPixelWorldPosition)
{
	//Vector from light to pixel
	float4 lightVec = aLight.Position - aPixelWorldPosition;
	
	//Dist betweem light and pixel
	float distance = length(lightVec);
	
	//normalize lightvector
	lightVec = normalize(lightVec);
	
	float attenuation = 1.f / (1.f + 0.1f * distance + 0.01f * distance * distance);

	//attenuation
	float angularAttenuation = dot(-lightVec, aLight.Direction);
	angularAttenuation -= 1.f - aLight.Cone;
	angularAttenuation *= 1.f / aLight.Cone;


	//add color to finalColor
	float lambert = dot((float3)lightVec, aNormal);
	float4 lightColor = saturate(lambert * aLight.Color * attenuation  * angularAttenuation * 3.f);
	
	return lightColor;
}