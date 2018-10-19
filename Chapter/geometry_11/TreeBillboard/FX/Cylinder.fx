#include "LightHelper.fx"

cbuffer cbPerFrame
{
	DirectionalLight gDirLights[3];
	float3 gEyePosW;
	
	float gFogStart;
	float gFogRange;
	float gFogColor;
};

cbuffer cbPerObject
{
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldViewProj;
	float4x4 gTexTransform;
	float 	 gHeight;
	Material gMaterial;
};

cbuffer cbFixed
{
	float gTexV[4] = 
	{
		1.f, 1.f, 0.f, 0.f
	};
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
	float3 PosL 	: POSITION;
	float3 NormalL 	: NORMAL;
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
	vout.NormalL = vin.NormalL;
	vout.Tex = vin.Tex;
	return vout;
}

[maxvertexcount(4)]
void GS(line VertexOut gin[2],
		inout TriangleStream<GeoOut> triStream )
{
	float4 pos_local[4]; 
	pos_local[0] = float4( gin[0].PosL + float3( 0.f, gHeight, 0.f ), 1.f );
	pos_local[1] = float4( gin[1].PosL + float3( 0.f, gHeight, 0.f ), 1.f );
	pos_local[2] = float4( gin[0].PosL, 1.f );
	pos_local[3] = float4( gin[1].PosL, 1.f );
	
	float tex_u[4];
	tex_u[0] = gin[0].Tex.x;
	tex_u[1] = gin[1].Tex.x;
	tex_u[2] = tex_u[0];
	tex_u[3] = tex_u[1];
	
	float3 normal[4];
	normal[0] = mul(gin[0].NormalL, (float3x3)gWorldInvTranspose);
	normal[1] = mul(gin[1].NormalL, (float3x3)gWorldInvTranspose);
	normal[2] = gin[0].NormalL;
	normal[3] = gin[1].NormalL;
	
	GeoOut gout;
	[unroll]
	for( int i = 0; i < 4; ++i )
	{
		gout.PosW = mul(pos_local[i], gWorld).xyz;
		gout.PosH = mul(pos_local[i], gWorldViewProj);
		gout.NormalW = normal[i];
		gout.Tex = float2(tex_u[i], gTexV[i]);
		triStream.Append(gout);
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