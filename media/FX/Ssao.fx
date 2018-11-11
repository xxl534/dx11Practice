cbuffer cbPerFrame
{
	float4x4 gViewToTexSpace;
	float4 gOffsetVectors[14];
	float4 gFrustumCorners[4];
	
	float gOcclusionRadius = 0.5f;
	float gOcclusionFadeStart = 0.2f;
	float gOcclusionFadeEnd = 2.f;
	float gSurfaceEpsilon = 0.05f;
};

Texture2D gNormalDepthMap;
Texture2D gRandomVecMap;

SamplerState samNormalDepth
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;
	
	//Set a very far depth value if sampling outside of the NormalDepth map
	// so ww do not get false occlusions.
	AddressU = BORDER;
	AddressV = BORDER;
	BorderColor = float4(0.f,0.f,0.f,1e5f);
};

SamplerState samRandomVec
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;
	AddressU = WRAP;
	AddressV = WRAP;
};

struct VertexIn
{
	float3 PosL 	: POSITION;
	float3 ToFarPlaneIndex : NORMAL;
	float2 Tex : TEXCOORD;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 ToFarPlane : TEXCOORD0;
	float2 Tex 	: TEXCOORD1;
};

VertexOut VS(VertexIn vin)
{
	 VertexOut vout;
	 
	 //Already in NDC space.
	 vout.PosH = float4(vin.PosL, 1.f);
	 
	 //We store the index to the frustum corner in the normal x-coord slot.
	 vout.ToFarPlane = gFrustumCorners[vin.ToFarPlaneIndex.x].xyz;
	 
	 vout.Tex = vin.Tex;
	 
	 return vout;
}

//Determian how much the sample point q occludes the point p as a funciton of distZ.
float OcclusionFunciton(float distZ)
{
	//If depth(q) is "behind" depth(p), then q cannot occlude p. Moreover, if
	//depth(q) and depth(p) are sufficiently close, then ew also assume q cannot
	//occlude p because q needs to be in front of p by Epsilon to occlude p.
	
	//We use the following funciton to determine the occlusion.
	//
	//       1.0     -------------\
	//               |           |  \
	//               |           |    \
	//               |           |      \ 
	//               |           |        \
	//               |           |          \
	//               |           |            \
	//  ------|------|-----------|-------------|---------|--> zv
	//        0     Eps          z0            z1        
	//
	
	float occlusion = 0.f;
	if( distZ > gSurfaceEpsilon )
	{
		float fadeLength = gOcclusionFadeEnd - gOcclusionFadeStart;
		
		occlusion = saturate( (gOcclusionFadeEnd-distZ)/fadeLength);
	}
	return occlusion;
}

float4 PS(VertexOut pin, uniform int gSampleCount ) : SV_Target
{
	//p -- this point we are computing the ambient occlusion for.
	//n -- normal vector at p.
	//q -- a random offset from p.
	//r -- a potential occluder that might occlude p.
	
	//Get viewspace normal and z-coord of this pixel. the tex-coords for
	//this full screen quad we drew are already in uv-space.
	float4 normalDepth = gNormalDepthMap.SampleLevel(samNormalDepth, pin.Tex, 0.f );
	
	float3 n = normalDepth.xyz;
	float pz = normalDepth.w;
	
	//Reconstruct full view space position(x,y,z).
	//Find t such that p = t*pin.ToFarPlane.
	//p.z = t*pin.ToFarPlane.z
	//t = p.z / pin.ToFarPlane.z
	float3 p = (pz/pin.ToFarPlane.z)*pin.ToFarPlane;
	
	//Extract random vector and map from[0,1]-->[-1,+1]
	float3 randVec = 2.f * gRandomVecMap.SampleLevel(samRandomVec, 4.f*pin.Tex, 0.f ).rgb - 1.f;
	float occlusionSum = 0.f;
	
	//Sample neighboring points about p in the hemisphere oriented by n.
	[unroll]
	for( int i = 0; i < gSampleCount; ++i )
	{
		//Are offset vectors are fixed and uniformly distributed( so that our offset vector 
		//do not clump in the same direction). If we reflect them about a random vector
		//then we get a random uniform distribution of offset vectors.
		float3 offset = reflect(gOffsetVectors[i].xyz, randVec);
		
		//Flip offset vector if it is behind the plane defined by ( p, n ).
		float flip = sign( dot(offset, n) );
		
		//Sample a point near p within the occlusion radius.
		float3 q = flip * gOcclusionRadius * offset;
		
		//Project q and generate projective tex-coords.
		float4 projQ = mul(float4(q, 1.f), gViewToTexSpace);
		
		projQ /= projQ.w;
		
		//Find the nearest depth value along the ray from the eye to q( this is not 
		//the depth of q, as q is just arbitrary point near p and might occupy empty space).
		//To find the nearest depth we look it up in the depthmap.
		
		float rz = gNormalDepthMap.SampleLevel(samNormalDepth, projQ.xy, 0.0f).a;
		
		//Reconstruct full view space position r = ( rx, ry, rz).We know r 
		//lies on the ray of q, so there exists a t such that r = t*q.
		//r.z = t* q.z ==> t = r.z/q.z.
		
		float3 r = ( rz/q.z)*q;
		
		//Test whether r occludes p.
		// * The product dot(n, normalize(r-p)) measures how much in front of the
		//	 plane(p,n) the occluder point r is. The more in front it is, the more
		//	 occlusion weight we give it. This also prevent self shadowing where a
		//	 point r on an angled plane(p,n) could give a false occlusion since they		
		//	 have different depth values with respect to the eye.
		// * The weight of the occlusion is scaled based on how far the occluder is 
		//	 from the point we are computing the occlusion of. Is the occluder r is 
		//	 far away from p, then it does not occlude it.
		
		float distZ = p.z - r.z;
		float dp = max(dot(n,normalize(r-p)), 0.f);
		float occlusion = dp * OcclusionFunciton(distZ);
		
		occlusionSum += occlusion;
	}
	
	occlusionSum /= gSampleCount;
	float access = 1.f - occlusionSum;
	
	//Sharpen the constrat of the SSAO map to make the SSAO affect more dramatic.
	float4 ret = saturate(pow(access, 4.f));
	return ret; 
}

technique11 Ssao
{
    pass P0
    {
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(14) ) );
    }
}