#ifndef BASICLIGHTPIXEL_FX
#define BASICLIGHTPIXEL_FX
#include "LightHelper.fx"
#include "BasicStruct.fx"

cbuffer cbLight
{
	DirectionalLight gDirLights[3];
};
cbuffer cbFog
{
	float gFogStart;
	float gFogRange;
	float4 gFogColor;
};
cbuffer cbObjectMaterial
{
	Material gMaterial;
};

SamplerState texSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	
	AddressU = WRAP;
	AddressV = WRAP;
};

Texture2D gDiffuseMap;
TextureCube gCubeMap;
#ifdef VERTEX_TANGENT
Texture2D gNormalMap;
#endif

float4 BasicLightPS(PixelIn pin, 
		  uniform int gLightCount, 
		  uniform bool gUseTexture, 
		  uniform bool gAlphaClip, 
		  uniform bool gFogEnabled,
		  uniform bool gReflectionEnabled) : SV_Target
{
	pin.NormalW = normalize(pin.NormalW);
	
	float3 toEye = gEyePosition - pin.PosW;
	
	float distToEye = length(toEye);
	
	toEye /= distToEye;
	
	float4 texColor = float4(1.f, 1.f, 1.f, 1.f);
	if( gUseTexture )
	{
		texColor = gDiffuseMap.Sample( texSampler, pin.Tex);
		if( gAlphaClip )
		{
			clip( texColor.a - 0.05f );
		}
	}
		
	//
	// Normal mapping
	//
	
	float3 normal = pin.NormalW;

#ifdef VERTEX_TANGENT	
	float3 normalMapSample = gNormalMap.Sample(texSampler, pin.Tex).rgb;
	normal = NormalSampleToWorldSpace(normalMapSample, pin.NormalW, pin.TangentW);
#endif
	
	
	float4 litColor = texColor;
	if( gLightCount > 0 )
	{
		float4 ambient = float4( 0.f, 0.f, 0.f, 0.f );
		float4 diffuse = float4( 0.f, 0.f, 0.f, 0.f );
		float4 spec = float4( 0.f, 0.f, 0.f, 0.f );
		
		[unroll]
		for(int i = 0; i < gLightCount; ++i )
		{
			float4 A, D, S;
			ComputeDirectionalLight(gMaterial, gDirLights[i], normal, toEye, A, D, S);
			ambient += A;
			diffuse += D;
			spec += S;
		}
		litColor = texColor * (ambient + diffuse) + spec;
		if( gReflectionEnabled )
		{
			float3 incident = -toEye;
			float3 reflectionVector = reflect(incident, pin.NormalW);
			float4 reflectionColor  = gCubeMap.Sample(texSampler, reflectionVector);

			litColor += gMaterial.Reflect*reflectionColor;
		}
	}
	
	if( gFogEnabled )
	{
		float fogLerp = saturate( ( distToEye - gFogStart ) / gFogRange );
		litColor = lerp( litColor, gFogColor, fogLerp);
	}
	
	litColor.a = gMaterial.Diffuse.a * texColor.a;
	return litColor;
}

#endif