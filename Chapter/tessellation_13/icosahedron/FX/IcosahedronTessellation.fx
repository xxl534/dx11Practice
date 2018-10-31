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
	float EdgeTess[3] : SV_TessFactor;
	float InsideTess : SV_InsideTessFactor;
};

PatchTess ConstantHS(InputPatch<VertexOut,3> patch, uint patchID : SV_PrimitiveID)
{
	PatchTess pt;
	float3 centerL = 0.25f*(patch[0].PosL+patch[1].PosL+patch[2].PosL);
	float3 centerW = mul(float4(centerL,1.f),gWorld).xyz;
	
	float d = distance(centerW,gEyePosW);
	
	const float d0 = 10.f;
	const float d1 = 100.f;
	float tess = 15.f * saturate( (d1-d)/(d1-d0)) + 1.f;
		
	pt.EdgeTess[0] = tess;
	pt.EdgeTess[1] = tess;
	pt.EdgeTess[2] = tess;
	
	pt.InsideTess = tess;
	
	return pt;
}

struct HullOut
{
	float3 PosL : POSITION;
};

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
HullOut HS(InputPatch<VertexOut,3> p,
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

[domain("tri")]
DomainOut DS(PatchTess patchTess,
			float3 uvw : SV_DomainLocation,
			const OutputPatch<HullOut,3> tri )
{
	DomainOut dout;
	
	float3 p = tri[0].PosL * uvw.x + tri[1].PosL * uvw.y + tri[2].PosL * uvw.z;
	
	p = normalize(p);
	
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
