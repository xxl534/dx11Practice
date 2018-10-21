#include "LightHelper.fx"

cbuffer cbPerFrame
{
	DirectionalLight gDirLights[3];
	float3 gEyePosW;
	
	float 	gFogStart;
	float 	gFogRange;
	float4 	gFogColor;
};

cbuffer cbPerObject
{
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldViewProj;
	float4x4 gTexTransform;
	Material gMaterial;
	float 	 gTime;
	float 	 gSpeed;
};

Texture2D gDiffuseMap;

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;
	
	AddressU = WRAP;
	AddressV = WRAP;
};

struct VertexIn
{
	float3 PosL 	: POSITION;
	float3 NormalL 	: NORMAL;
	float2 Tex 		: TEXCOORD;
};

struct VertexOut
{
	float3 PosL		: POSITION;
	float3 NormalL	: NORMAL;
	float2 Tex 		: TEXCOORD;
};

struct GeoOut
{
	float4 PosH		: SV_POSITION;
	float3 PosW 	: POSITION;
	float3 NormalW 	: NORMAL;
	float2 Tex 		: TEXCOORD;
};

VertexOut VS( VertexIn vin )
{
	VertexOut vout;
	vout.PosL = vin.PosL;
	vout.NormalL = vin.NormalL;
	vout.Tex = mul( float4(vin.Tex, 0.f,1.f), gTexTransform ).xy;
	return vout;
}

GeoOut VS_Test(VertexIn vin)
{
	GeoOut vout;
	vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.NormalW = mul(vin.PosL, (float3x3)gWorldInvTranspose);
	
	vout.PosH = mul(float4(vin.PosL,1.0f), gWorldViewProj);
	vout.Tex = mul(float4(vin.Tex, 0.f, 1.f), gTexTransform).xy;
	
	return vout;
}

[maxvertexcount(3)]
void GS(triangle VertexOut gin[3],
		uint primID : SV_PrimitiveID,
		inout TriangleStream<GeoOut> triStream )
{
	float f = (float)primID;
	int unuse = 0;
	float speed = gSpeed * modf( (7.f * f * f + 13.f * f + 19.f) / 23.f, unuse );
	float3 normal = normalize( cross( gin[1].PosL - gin[0].PosL, gin[2].PosL - gin[1].PosL ) );
	float3 offset = normal * speed * gTime;
	//offset = normal * gTime;
	GeoOut gout;
	[unroll]
	for( int i = 0; i < 3; ++i )
	{
		float3 posL = gin[i].PosL + offset;
		gout.PosW = mul(float4(posL, 1.f), gWorld ).xyz;
		gout.PosH = mul(float4(posL, 1.f), gWorldViewProj);
		gout.NormalW = mul(normal,(float3x3)gWorldInvTranspose);
		gout.Tex = gin[i].Tex;
		triStream.Append(gout);
	}
}

[maxvertexcount(3)]
void GSTest(triangle VertexOut gin[3],
		inout TriangleStream<GeoOut> triStream )
{
	GeoOut vout;
	[unroll]
	for( int i = 0; i < 3; ++i )
	{
		vout.PosW = mul(float4(gin[i].PosL, 1.0f), gWorld).xyz;
		vout.NormalW = mul(gin[i].PosL, (float3x3)gWorldInvTranspose);
	
		vout.PosH = mul(float4(gin[i].PosL,1.0f), gWorldViewProj);
		vout.Tex = mul(float4(gin[i].Tex, 0.f, 1.f), gTexTransform).xy;
		triStream.Append(vout);
	}
}

float4 PS(GeoOut pin, uniform int gLightCount, uniform bool gUseTexture, uniform bool gAlphaClip, uniform bool gFogEnabled):SV_Target
{
	pin.NormalW = normalize(pin.NormalW);
	
	float3 toEye = gEyePosW - pin.PosW;
	
	float distToEye = length(toEye);
	
	toEye /= distToEye;
	
	float4 texColor = float4(1.f, 1.f, 1.f, 1.f);
	if( gUseTexture )
	{
		texColor = gDiffuseMap.Sample( samAnisotropic, pin.Tex);
		if( gAlphaClip )
		{
			clip( texColor.a - 0.05f );
		}
	}
	
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
			ComputeDirectionalLight(gMaterial, gDirLights[i], pin.NormalW, toEye, A, D, S);
			ambient += A;
			diffuse += D;
			spec += S;
		}
		
		litColor = texColor * (ambient + diffuse) + spec;
	}
	
	if( gFogEnabled )
	{
		float fogLerp = saturate( ( distToEye - gFogStart ) / gFogRange );
		litColor = lerp( litColor, gFogColor, fogLerp);
	}
	
	litColor.a = gMaterial.Diffuse.a * texColor.a;
	return litColor;
}

technique11 Test
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS_Test() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(3, false, false, false) ) );
    }
}

technique11 Test2
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( CompileShader( gs_5_0, GSTest() ) );
        SetPixelShader( CompileShader( ps_5_0, PS(3, false, false, false) ) );
    }
}

technique11 Light0
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( CompileShader( gs_5_0, GS() ) );
        SetPixelShader( CompileShader( ps_5_0, PS(0, false, false, false) ) );
    }
}

technique11 Light3
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( CompileShader( gs_5_0, GS() ) );
        SetPixelShader( CompileShader( ps_5_0, PS(3, false, false, false) ) );
    }
}

technique11 Light3Tex
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( CompileShader( gs_5_0, GS() ) );
		SetPixelShader( CompileShader( ps_5_0, PS( 3, true, false, false ) ) );
	}
}

technique11 Light3TexAlphaClip
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( CompileShader( gs_5_0, GS() ) );
        SetPixelShader( CompileShader( ps_5_0, PS(3, true, true, false) ) );
    }
}
    
technique11 Light3Fog
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( CompileShader( gs_5_0, GS() ) );
        SetPixelShader( CompileShader( ps_5_0, PS(3, false, false, true) ) );
    }
}

technique11 Light3TexFog
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( CompileShader( gs_5_0, GS() ) );
        SetPixelShader( CompileShader( ps_5_0, PS(3, true, false, true) ) );
    }
}

technique11 Light3TexAlphaClipFog
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( CompileShader( gs_5_0, GS() ) );
        SetPixelShader( CompileShader( ps_5_0, PS(3, true, true, true) ) );
    }
}