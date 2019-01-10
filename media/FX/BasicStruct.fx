#ifndef BASICSTRUCT_FX
#define BASICSTRUCT_FX
cbuffer cbWorld
{
	float3 		gEyePosition;
	float4x4  	gWorld;
	float4x4 	gWorldInvTranspose;
	float4x4 	gWorldViewProj;
	float4x4 	gViewProj;
};
struct VertexIn
{
	float3 PosL     : POSITION;
	float3 NormalL  : NORMAL;
	float2 Tex      : TEXCOORD;
#ifdef VERTEX_TANGENT
	float3 TangentL : TANGENT;
#endif
#ifdef SKINNED_MESH
	float3 Weights 	: WEIGHTS;
	uint4 BoneIndices : BONEINDICES;
#endif
};

struct PixelIn
{
	float4 PosH     : SV_POSITION;
    float3 PosW     : POSITION;
    float3 NormalW  : NORMAL;
#ifdef VERTEX_TANGENT
	float3 TangentW : TANGENT;
#endif
	float2 Tex      : TEXCOORD;
};
#endif