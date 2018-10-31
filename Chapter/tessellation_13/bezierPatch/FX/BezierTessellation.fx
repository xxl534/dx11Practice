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
};

Texture2D gDiffuseMap;

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;
	
	AddressU =WRAP;
	AddressV = WRAP;
};

struct VertexIn
{
	float3 PosL : POSITION;
};

struct VertexOut
{
	float3 PosL : POSITION;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	vout.PosL = vin.PosL;
	return vout;
}

struct PatchTess
{
	float EdgeTess[4] : SV_TessFactor;
	float InsideTess[2] : SV_InsideTessFactor;
};

PatchTess ConstantHS(InputPatch<VertexOut,16> patch, uint patchID : SV_PrimitiveID)
{
	PatchTess pt;
	
	float tess = 25.f;
		
	pt.EdgeTess[0] = tess;
	pt.EdgeTess[1] = tess;
	pt.EdgeTess[2] = tess;
	pt.EdgeTess[3] = tess;
	
	pt.InsideTess[0] = tess;
	pt.InsideTess[1] = tess;
	
	return pt;
}

struct HullOut
{
	float3 PosL : POSITION;
};

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(16)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
HullOut HS(InputPatch<VertexOut,16> p,
			uint i : SV_OutputControlPointID,
			uint patchId : SV_PrimitiveID)
{
	HullOut hout;
	hout.PosL = p[i].PosL;
	return hout;
}

struct DomainOut
{
	float4 PosH : SV_POSITION;
};

float4 BernsteinBasis(float t)
{
	float invT = 1.f - t;
	return float4( invT*invT*invT,
					3.f*t*invT*invT,
					3.f*t*t*invT,
					t*t*t);
}

float3 CubicBezierSum( const OutputPatch<HullOut,16> bezpatch, float4 basicU, float4 basicV)
{
	float3 sum = float3(0.f,0.f,0.f);
	sum = basicV.x * (basicU.x*bezpatch[0].PosL+basicU.y*bezpatch[1].PosL +basicU.z*bezpatch[2].PosL +basicU.w*bezpatch[3].PosL);
	sum += basicV.y * (basicU.x*bezpatch[4].PosL+basicU.y*bezpatch[5].PosL +basicU.z*bezpatch[6].PosL +basicU.w*bezpatch[7].PosL);
	sum += basicV.z * (basicU.x*bezpatch[8].PosL+basicU.y*bezpatch[9].PosL +basicU.z*bezpatch[10].PosL +basicU.w*bezpatch[11].PosL);
	sum += basicV.w * (basicU.x*bezpatch[12].PosL+basicU.y*bezpatch[13].PosL +basicU.z*bezpatch[14].PosL +basicU.w*bezpatch[15].PosL);
	return sum;
}

float4 dBernsteinBasis(float t)
{
	float invT = 1.f - t;
	return float4( - 3.f * invT * invT,
					3.f * invT * invT - 6.f * t * invT,
					6.f * t * invT - 3.f * t * t,
					3.f * t * t );
}
[domain("quad")]
DomainOut DS(PatchTess patchTess,
			float2 uv : SV_DomainLocation,
			const OutputPatch<HullOut,16> bezPatch )
{
	DomainOut dout;
	
	float4 basisU = BernsteinBasis(uv.x);
	float4 basisV = BernsteinBasis(uv.y);
	
	float3 p = CubicBezierSum( bezPatch, basisU, basisV );
	
	dout.PosH = mul(float4(p,1.f),gWorldViewProj);
	return dout;
}

float4 PS(DomainOut pin) : SV_Target
{
	return float4(1.f,1.f,1.f,1.f);
}

technique11 Tess
{
	Pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetHullShader( CompileShader( hs_5_0, HS() ));
		SetDomainShader( CompileShader( ds_5_0, DS()));
		SetPixelShader( CompileShader( ps_5_0, PS()));
	}
}
