#ifndef BASICVERTEX_FX
#define BASICVERTEX_FX

#include "BasicStruct.fx"

cbuffer cbTexTrans
{
	float4x4 gTexTransform;
};

#ifdef SKINNED_MESH
cbuffer cbSkinned
{
	float4x4 gBoneTransforms[96];
};
#endif

PixelIn BasicVS(VertexIn vin)
{
	PixelIn vout;

#ifdef SKINNED_MESH
	float weights[4] = { 0.f, 0.f, 0.f,0.f,};
	weights[0] = vin.Weights.x;
	weights[1] = vin.Weights.y;
	weights[2] = vin.Weights.z;
	weights[3] = 1.f - weights[0] - weights[1] - weights[2];
	float3 posL = float3(0.f,0.f,0.f);
	float3 normalL = float3(0.f,0.f,0.f);
	#ifdef VERTEX_TANGENT	
		float3 tangentL = float3(0.f,0.f,0.f);
	#endif
	[unroll]
	for( int i = 0; i < 4; ++i)
	{
	    // Assume no nonuniform scaling when transforming normals, so 
		// that we do not have to use the inverse-transpose.
		
		posL += weights[i]*mul(float4(vin.PosL,1.f), gBoneTransforms[vin.BoneIndices[i]]).xyz;
		normalL += weights[i]*mul(vin.NormalL, (float3x3)gBoneTransforms[vin.BoneIndices[i]]);
	#ifdef VERTEX_TANGENT			
			tangentL += weights[i]*mul(vin.TangentL, (float3x3)gBoneTransforms[vin.BoneIndices[i]]);
	#endif
	}
#else
	float3 posL = vin.PosL;
	float3 normalL = vin.NormalL;
	#ifdef VERTEX_TANGENT	
		float3 tangentL = vin.TangentL;
	#endif
#endif	
	// Transform to world space space.
	vout.PosW     = mul(float4(posL, 1.0f), gWorld).xyz;
	vout.NormalW  = mul(normalL, (float3x3)gWorldInvTranspose);
	vout.TangentW = mul(tangentL, (float3x3)gWorld);

	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(posL, 1.0f), gWorldViewProj);
	
	// Output vertex attributes for interpolation across triangle.
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;
	
	return vout;
}

#endif