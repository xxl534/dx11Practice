#include "LightHelper.fx"

cbuffer cbPerFrame
{
	DirectionalLight gDirLights[3];
	float3 gEyePosW;
	
	float gFogStart;
	float gFogRange;
	float4 gFogColor;
};

cbuffer cbPerObject
{
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldViewProj;
	float4x4 gTexTransform;
	Material gMaterial;
	int 	 gIteration;
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
	float2 Tex 		: TEXCOORD;
};

struct VertexOut
{
	float3 PosL 	: POSITION;
	float2 Tex 		: TEXCOORD;
};

struct GeoOut
{
	float4 PosH 	: SV_POSITION;
	float3 PosW		: POSITION;
	float3 NormalW 	: NORMAL;
	float2 Tex 		: TEXCOORD;
};

VertexOut VS( VertexIn vin )
{
	VertexOut vout;
	vout.PosL = vin.PosL;
	vout.Tex = mul(float4(vin.Tex,0.f,1.f), gTexTransform ).xy;
	return vout;
}

void Subdivide(VertexOut vin0, VertexOut vin1, VertexOut vin2, out VertexOut outVert[6])
{
	VertexOut m[3];
	
	m[0].PosL =  normalize( 0.5f * (vin0.PosL + vin1.PosL) );
	m[1].PosL =  normalize( 0.5f * (vin1.PosL + vin2.PosL) );
	m[2].PosL =  normalize( 0.5f * (vin2.PosL + vin0.PosL) );
	
	m[0].Tex = 0.5f * (vin0.Tex + vin1.Tex);
	m[1].Tex = 0.5f * (vin1.Tex + vin2.Tex);
	m[2].Tex = 0.5f * (vin2.Tex + vin0.Tex);
	
	outVert[0] = vin0;
	outVert[1] = m[0];
	outVert[2] = m[2];
	outVert[3] = m[1];
	outVert[4] = vin2;
	outVert[5] = vin1;
}

void OutputSubdivision(VertexOut v[6], inout TriangleStream<GeoOut> triStream )
{
	GeoOut gout[6];
	[unroll]
	for( int i = 0; i < 6; ++i )
	{
		gout[i].PosW = mul(float4(v[i].PosL, 1.f), gWorld ).xyz;
		gout[i].PosH = mul(float4(v[i].PosL, 1.f), gWorldViewProj);
		gout[i].NormalW = mul(v[i].PosL, (float3x3)gWorldInvTranspose ).xyz;
		gout[i].Tex = v[i].Tex;
	}
	
	
	triStream.RestartStrip();
	[unroll]
	for( int j = 0; j < 5; ++j )
	{
		triStream.Append(gout[j]);
	}
	
	triStream.RestartStrip();
	triStream.Append(gout[1]);
	triStream.Append(gout[5]);
	triStream.Append(gout[3]);
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


[maxvertexcount(32)]
void GS(triangle VertexOut gin[3],
		inout TriangleStream<GeoOut> triStream )
{
	int iteration = gIteration;
		
	GeoOut gout;	
	if( iteration == 0 )
	{	
		triStream.RestartStrip();
		for(int i = 0; i < 3; ++i)
		{
			gout.PosW = mul(float4(gin[i].PosL, 1.f), gWorld ).xyz;
			gout.PosH = mul(float4(gin[i].PosL, 1.f), gWorldViewProj );
			gout.NormalW = mul(gin[i].PosL, (float3x3)gWorldInvTranspose );
			gout.Tex = gin[i].Tex;
			triStream.Append(gout);
		}
	}
	else
	{
		VertexOut v[6];
		Subdivide(gin[0], gin[1], gin[2], v);
		if( iteration == 1 )
		{
			OutputSubdivision( v, triStream );
		}
		else
		{
			VertexOut v2[6];
			Subdivide(v[0], v[1], v[2], v2 );
			OutputSubdivision( v2, triStream );
			
			Subdivide(v[1], v[3], v[2], v2 );
			OutputSubdivision( v2, triStream );
			
			Subdivide(v[2], v[3], v[4], v2 );
			OutputSubdivision( v2, triStream );
			
			Subdivide(v[1], v[5], v[3], v2 );
			OutputSubdivision( v2, triStream );
		}
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