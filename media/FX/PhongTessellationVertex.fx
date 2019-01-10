#ifndef PHONGTESSELLATIONVERTEX_FX
#define PHONGTESSELLATIONVERTEX_FX
#include "BasicStruct.fx"

cbuffer cbTess
{
	float gMaxTessDistance;
	float gMinTessDistance;
	float gMinTessFactor;
	float gMaxTessFactor;
};

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

struct TesselIn
{
	float3 PosW     : POSITION;
    float3 NormalW  : NORMAL;
#ifdef VERTEX_TANGENT
	float3 TangentW : TANGENT;
#endif
	float2 Tex      : TEXCOORD;
	float  TessFactor : TESS;
};

struct DomainIn
{
	float3 PosW     : POSITION;
    float3 NormalW  : NORMAL;
#ifdef VERTEX_TANGENT	
	float3 TangentW : TANGENT;
#endif
	float2 Tex      : TEXCOORD;
};

struct PatchTessTri
{
	float EdgeTess[3] : SV_TessFactor;
	float InsideTess  : SV_InsideTessFactor;
};

cbuffer cbFixed
{
	float gShapeFactor = 0.75;
};

struct PatchTessQuad
{
	float EdgeTess[4] 	: SV_TessFactor;
	float InsideTess[2]	: SV_InsideTessFactor;
};
TesselIn PhongTessVS(VertexIn vin)
{
	TesselIn vout;
	
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

	vout.PosW = mul(float4(posL, 1.f ), gWorld).xyz;
	vout.NormalW = mul(normalL, (float3x3)gWorldInvTranspose);
#ifdef VERTEX_TANGENT	
	vout.TangentW = mul(tangentL, (float3x3)gWorld);
#endif
	
	// Output vertex attributes for interpolation across triangle.
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;
	
	
	float3 toEye = gEyePosition - vout.PosW;
	float d = length(toEye);
	toEye *= ( 1 / d );
	
	float tess = saturate( (gMinTessDistance - d ) / ( gMinTessDistance - gMaxTessDistance ) ) *( 1 - dot( vout.NormalW, toEye ) );
	
	vout.TessFactor = gMinTessFactor + tess * ( gMaxTessFactor - gMinTessFactor );
	
	return vout;
}

PatchTessTri PatchTriConstantHS(InputPatch<TesselIn,3> patch,
					uint patchID : SV_PrimitiveID)
{
	PatchTessTri pt;
	
	// Average tess factors along edges, and pick an edge tess factor for 
	// the interior tessellation.  It is important to do the tess factor
	// calculation based on the edge properties so that edges shared by 
	// more than one triangle will have the same tessellation factor.  
	// Otherwise, gaps can appear.
	pt.EdgeTess[0] = 0.5f*(patch[1].TessFactor + patch[2].TessFactor);
	pt.EdgeTess[1] = 0.5f*(patch[2].TessFactor + patch[0].TessFactor);
	pt.EdgeTess[2] = 0.5f*(patch[0].TessFactor + patch[1].TessFactor);
	pt.InsideTess  = (pt.EdgeTess[0] + pt.EdgeTess[1] + pt.EdgeTess[2])/3;
	
	return pt;
}

PatchTessQuad PatchQuadConstantHS(InputPatch<TesselIn,4> patch,
					uint patchID : SV_PrimitiveID )
{
	PatchTessQuad pt;
	pt.EdgeTess[0] = 0.5f*(patch[0].TessFactor + patch[1].TessFactor);
	pt.EdgeTess[1] = 0.5f*(patch[1].TessFactor + patch[2].TessFactor);
	pt.EdgeTess[2] = 0.5f*(patch[2].TessFactor + patch[3].TessFactor);
	pt.EdgeTess[3] = 0.5f*(patch[3].TessFactor + patch[0].TessFactor);
	
	pt.InsideTess[0]  = 0.5 * (pt.EdgeTess[1] + pt.EdgeTess[3] );
	pt.InsideTess[1]  = 0.5 * (pt.EdgeTess[0] + pt.EdgeTess[2] );
	
	return pt;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchTriConstantHS")]
[maxtessfactor(64.0f)]
DomainIn TriHS(InputPatch<TesselIn,3> p,
				uint i : SV_OutputControlPointID,
				uint patchId : SV_PrimitiveID )
{
	DomainIn hout;
	
	// Pass through shader.
	hout.PosW     = p[i].PosW;
	hout.NormalW  = p[i].NormalW;	
#ifdef VERTEX_TANGENT	
	hout.TangentW = p[i].TangentW;
#endif
	hout.Tex      = p[i].Tex;
	
	return hout;
}

[domain("Quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("PatchQuadConstantHS")]
[maxtessfactor(64.0f)]
DomainIn QuadHS(InputPatch<TesselIn,4> p,
				uint i : SV_OutputControlPointID,
				uint patchId : SV_PrimitiveID )
{
	DomainIn hout;
	
	// Pass through shader.
	hout.PosW     = p[i].PosW;
	hout.NormalW  = p[i].NormalW;
#ifdef VERTEX_TANGENT		
	hout.TangentW = p[i].TangentW;
#endif
	hout.Tex      = p[i].Tex;
	
	return hout;
}

float3 project(float3 p, float3 planePoint, float3 planeNormal )
{
	return p - dot( p - planePoint, planeNormal) * planeNormal;
}
[domain("tri")]
PixelIn PhongTessTriDS( PatchTessTri patchTess,
				float3 bary : SV_DomainLocation,
				const OutputPatch<DomainIn,3> tri)
{
	PixelIn dout;
	
	float3 pos = bary.x*tri[0].PosW     + bary.y*tri[1].PosW     + bary.z*tri[2].PosW;
	
	//phong Tessellation "GPU PRO 1-As Simple as Possible Tessellation for Interactive Application"
	float3 c0 = project(pos, tri[0].PosW, tri[0].NormalW );
	float3 c1 = project(pos, tri[1].PosW, tri[1].NormalW );
	float3 c2 = project(pos, tri[2].PosW, tri[2].NormalW );
	float3 q = bary.x*c0 + bary.y*c1 + bary.z*c2;
	dout.PosW = lerp( pos, q, gShapeFactor );
	
	dout.NormalW  = bary.x*tri[0].NormalW  + bary.y*tri[1].NormalW  + bary.z*tri[2].NormalW;
#ifdef VERTEX_TANGENT	
	dout.TangentW = bary.x*tri[0].TangentW + bary.y*tri[1].TangentW + bary.z*tri[2].TangentW;
#endif
	dout.Tex      = bary.x*tri[0].Tex      + bary.y*tri[1].Tex      + bary.z*tri[2].Tex;
	
	dout.NormalW = normalize(dout.NormalW);
	dout.PosH = mul(float4(dout.PosW, 1.0f), gViewProj );
	return dout;
}

[domain("quad")]
PixelIn PhongTessQuadDS( PatchTessQuad patchTess, 
				float2 uv : SV_DomainLocation, 
				const OutputPatch<DomainIn,4> quad)
{
	PixelIn dout;
	
	float oneMinusU = 1 - uv.x;
	float oneMinusV = 1 - uv.y;
	float f0 = oneMinusU * oneMinusV;
	float f1 = uv.x * oneMinusV;
	float f2 = oneMinusU * uv.y;
	float f3 = uv.x * uv.y;
	
	float3 pos = f0 * quad[0].PosW + f1 * quad[1].PosW + f2 * quad[2].PosW + f3 * quad[3].PosW;
	float3 c0 = project(pos, quad[0].PosW, quad[0].NormalW );
	float3 c1 = project(pos, quad[1].PosW, quad[1].NormalW );
	float3 c2 = project(pos, quad[2].PosW, quad[2].NormalW );
	float3 c3 = project(pos, quad[3].PosW, quad[3].NormalW );
	float3 q = f0 * c0 + f1 * c1 + f2 * c2 + f3 * c3;
	dout.PosW = lerp( pos, q, gShapeFactor );
	dout.NormalW = f0 * quad[0].NormalW + f1 * quad[1].NormalW + f2 * quad[2].NormalW + f3 * quad[3].NormalW;
#ifdef VERTEX_TANGENT	
	dout.TangentW = f0 * quad[0].TangentW + f1 * quad[1].TangentW + f2 * quad[2].TangentW + f3 * quad[3].TangentW;
#endif
	dout.Tex = f0 * quad[0].Tex + f1 * quad[1].Tex + f2 * quad[2].Tex + f3 * quad[3].Tex;
	
	dout.NormalW = normalize(dout.NormalW);
	dout.PosH = mul(float4(dout.PosW, 1.0f), gViewProj );
	return dout;
}
#endif