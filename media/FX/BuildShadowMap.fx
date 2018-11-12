cbuffer cbPerFrame
{
	float3 gEyePosW;
	
	float gHeightScale;
	float gMaxTessDistance;
	float gMinTessDistance;
	float gMinTessFactor;
	float gMaxTessFactor;
};

cbuffer cbPerObject
{
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gViewProj;
	float4x4 gWorldViewProj;
	float4x4 gTexTransform;
};

cbuffer cbSkinned
{
	float4x4 gBoneTransforms[96];
};

Texture2D gDiffuseMap;
Texture2D gNormalMap;

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV =	WRAP;
};

struct VertexIn
{
	float3 PosL 	: POSITION;
	float3 NormalL 	: NORMAL;
	float2 Tex 		: TEXCOORD;
};

struct SkinnedVertexIn
{
	float3 PosL       : POSITION;
	float3 NormalL    : NORMAL;
	float2 Tex        : TEXCOORD;
	float3 TangentL   : TANGENT;
	float3 Weights    : WEIGHTS;
	uint4 BoneIndices : BONEINDICES;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float2 Tex 	: TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	vout.PosH = mul(float4(vin.PosL, 1.f), gWorldViewProj);
	vout.Tex = mul(float4(vin.Tex, 0.f,1.f), gTexTransform).xy;
	
	return vout;
}

VertexOut SkinnedVS(SkinnedVertexIn vin)
{
	VertexOut vout;
	
	float weights[4] = { 0.f, 0.f, 0.f,0.f,};
	weights[0] = vin.Weights.x;
	weights[1] = vin.Weights.y;
	weights[2] = vin.Weights.z;
	weights[3] = 1.f - weights[0] - weights[1] - weights[2];
	
	float3 posL = float3(0.f,0.f,0.f);
	[unroll]
	for( int i = 0; i < 4; ++i)
	{
	    // Assume no nonuniform scaling when transforming normals, so 
		// that we do not have to use the inverse-transpose.
		
		posL += weights[i]*mul(float4(vin.PosL,1.f), gBoneTransforms[vin.BoneIndices[i]]).xyz;
	}
	
	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(posL, 1.0f), gWorldViewProj);
	// Output vertex attributes for interpolation across triangle.
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;
	
	return vout;
}

struct TessVertexOut
{
	float3 PosW 		: POSITION;
	float3 NormalW 		: NORMAL;
	float2 Tex 			: TEXCOORD;
	float  TessFactor 	: TESS;
};

TessVertexOut TessVS(VertexIn vin)
{
	TessVertexOut vout;
	
	vout.PosW = mul(float4(vin.PosL, 1.f), gWorld).xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorldInvTranspose);
	vout.Tex = mul(float4(vin.Tex, 0.f, 1.f), gTexTransform).xy;
	
	float d = distance(vout.PosW, gEyePosW);
	
	float tess = saturate( ( gMinTessDistance - d ) / ( gMinTessDistance - gMaxTessDistance ));
	
	vout.TessFactor = gMinTessFactor + tess*(gMaxTessFactor - gMinTessFactor);
	
	return vout;
}

struct PatchTess
{
	float EdgeTess[3] : SV_TessFactor;
	float InsideTess : SV_InsideTessFactor;
};

PatchTess PatchHS(InputPatch<TessVertexOut,3> patch,
					uint patchID : SV_PrimitiveID )
{
	PatchTess pt;
	
	pt.EdgeTess[0] = 0.5f * (patch[1].TessFactor + patch[2].TessFactor);
	pt.EdgeTess[1] = 0.5f * (patch[2].TessFactor + patch[0].TessFactor);
	pt.EdgeTess[2] = 0.5f * (patch[0].TessFactor + patch[1].TessFactor);
	pt.InsideTess = pt.EdgeTess[0];
	
	return pt;
}

struct HullOut
{
	float3 PosW 	: POSITION;
	float3 NormalW 	: NORMAL;
	float2 Tex 		: TEXCOORD;
};

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchHS")]
HullOut HS(InputPatch<TessVertexOut,3> p, 
		uint i : SV_OutputControlPointID,
		uint patchId : SV_PrimitiveID)
{
	HullOut hout;
	
	hout.PosW = p[i].PosW;
	hout.NormalW = p[i].NormalW;
	hout.Tex = p[i].Tex;
	
	return hout;
}

struct DomainOut
{
	float4 PosH 	: SV_POSITION;
	float3 PosW 	: POSITION;
	float3 NormalW 	: NORMAL;
	float2 Tex 		: TEXCOORD;
};

[domain("tri")]
DomainOut DS(PatchTess patchTess,
			float3 bary : SV_DomainLocation,
			const OutputPatch<HullOut,3> tri)
{
	DomainOut dout;
	
	dout.PosW = bary.x*tri[0].PosW + bary.y*tri[1].PosW + bary.z*tri[2].PosW;
	dout.NormalW = bary.x*tri[0].NormalW + bary.y*tri[1].NormalW + bary.z*tri[2].NormalW;
	dout.Tex = bary.x*tri[0].Tex + bary.y*tri[1].Tex + bary.z*tri[2].Tex;
	
	dout.NormalW = normalize(dout.NormalW);
	
	//
	// Displacement mapping.
	//
	
	// Choose the mipmap level based on distance to the eye; specifically, choose
	// the next miplevel every MipInterval units, and clamp the miplevel in [0,6].
	const float MipInterval = 20.f;
	float mipLevel = clamp((distance(dout.PosW, gEyePosW) - MipInterval) / MipInterval, 0.f, 6.f);
	
	float h = gNormalMap.SampleLevel(samLinear, dout.Tex, mipLevel).a;
	dout.PosW += (gHeightScale*(h-1.0))*dout.NormalW;
	
	dout.PosH = mul(float4(dout.PosW, 1.f), gViewProj);
	
	return dout;
}

void PS(VertexOut pin)
{
	float4 diffuse = gDiffuseMap.Sample(samLinear, pin.Tex);
	
	clip(diffuse.a - 0.05f);
}

void TessPS(DomainOut pin)
{
	float4 diffuse = gDiffuseMap.Sample(samLinear, pin.Tex);
	clip(diffuse.a - 0.05f);
}

RasterizerState Depth
{
	// [From MSDN]
	// If the depth buffer currently bound to the output-merger stage has a UNORM format or
	// no depth buffer is bound the bias value is calculated like this: 
	//
	// Bias = (float)DepthBias * r + SlopeScaledDepthBias * MaxDepthSlope;
	//
	// where r is the minimum representable value > 0 in the depth-buffer format converted to float32.
	// [/End MSDN]
	// 
	// For a 24-bit depth buffer, r = 1 / 2^24.
	//
	// Example: DepthBias = 100000 ==> Actual DepthBias = 100000/2^24 = .006

	// You need to experiment with these values for your scene.
	DepthBias = 100000;
    DepthBiasClamp = 0.0f;
	SlopeScaledDepthBias = 1.0f;
};

technique11 BuildShadowMapTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( NULL );

		SetRasterizerState(Depth);
    }
}

technique11 BuildShadowMapAlphaClipTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS() ) );
    }
}

technique11 BuildShadowMapSkinnedTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, SkinnedVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( NULL );

		SetRasterizerState(Depth);
    }
}

technique11 BuildShadowMapAlphaClipSkinnedTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, SkinnedVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS() ) );
    }
}

technique11 TessBuildShadowMapTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, TessVS() ) );
		SetHullShader( CompileShader( hs_5_0, HS() ) );
        SetDomainShader( CompileShader( ds_5_0, DS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( NULL );

		SetRasterizerState(Depth);
    }
}

technique11 TessBuildShadowMapAlphaClipTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, TessVS() ) );
		SetHullShader( CompileShader( hs_5_0, HS() ) );
        SetDomainShader( CompileShader( ds_5_0, DS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, TessPS() ) );
    }
}