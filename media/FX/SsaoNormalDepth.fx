cbuffer cbPerObject
{
	float4x4 gWorldView;
	float4x4 gWorldInvTransposeView;
	float4x4 gWorldViewProj;
	float4x4 gTexTransform;
};

cbuffer cbSkinned
{
	float4x4 gBoneTransforms[96];
};

Texture2D gDiffuseMap;

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

struct VertexIn
{
	float3 PosL :POSITION;
	float3 NormalL : NORMAL;
	float2 Tex 	: TEXCOORD;
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
	float3 PosV : POSITION;
	float3 NormalV : NORMAL;
	float2 Tex 	:TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	vout.PosV = mul(float4(vin.PosL, 1.f), gWorldView).xyz;
	vout.NormalV = mul(vin.NormalL, (float3x3)gWorldInvTransposeView);
	
	vout.PosH = mul(float4(vin.PosL, 1.f), gWorldViewProj);
	
	vout.Tex = mul(float4(vin.Tex, 0.f, 1.f), gTexTransform).xy;
	
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
	float3 normalL = float3(0.f,0.f,0.f);
	[unroll]
	for( int i = 0; i < 4; ++i)
	{
	    // Assume no nonuniform scaling when transforming normals, so 
		// that we do not have to use the inverse-transpose.
		
		posL += weights[i]*mul(float4(vin.PosL,1.f), gBoneTransforms[vin.BoneIndices[i]]).xyz;
		normalL += weights[i]*mul(vin.NormalL, (float3x3)gBoneTransforms[vin.BoneIndices[i]]);
	}
	
	// Transform to view space.
	vout.PosV = mul(float4(posL, 1.f), gWorldView).xyz;
	vout.NormalV = mul(normalL, (float3x3)gWorldInvTransposeView);

	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(posL, 1.0f), gWorldViewProj);
	
	// Output vertex attributes for interpolation across triangle.
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;
	
	return vout;
}

float4 PS(VertexOut pin, uniform bool gAlphaClip):SV_Target
{
	if(gAlphaClip)
	{
		float4 texColor = gDiffuseMap.Sample( samLinear, pin.Tex);
		clip(texColor.a - 0.015f);
	}
	
	float3 normal = normalize(pin.NormalV);
	return float4( normal, pin.PosV.z);
}

technique11 NormalDepth
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(false) ) );
    }
}

technique11 NormalDepthAlphaClip
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(true) ) );
    }
}

technique11 NormalDepthSkinned
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, SkinnedVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(false) ) );
    }
}

technique11 NormalDepthAlphaClipSkinned
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, SkinnedVS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(true) ) );
    }
}