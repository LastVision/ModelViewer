struct POINT_LIGHT
{
	float4 Position;
	float4 Color;
	float Range;
};

POINT_LIGHT PointLights[4];


float4 CalculatePointLight(POINT_LIGHT aLight, float3 aNormal, float4 aPixelWorldPosition)
{
	//Vector from light to pixel
	float4 lightVec = aLight.Position - aPixelWorldPosition;

	//Dist betweem light and pixel
	float distance = length(lightVec);

	//normalize lightvector
	lightVec = normalize(lightVec);


	//attenuation
	float attenuation = 1.f / (1.f + 0.1f * distance + 0.01f * distance * distance);
	float fallOff = 1.f - (distance / (aLight.Range + 0.00001f));
	float totalAttenuation = attenuation * fallOff;


	//add color to finalColor
	float lambert = dot((float3)lightVec, aNormal);
	float4 lightColor = saturate(lambert * aLight.Color);

	return lightColor * totalAttenuation;
}