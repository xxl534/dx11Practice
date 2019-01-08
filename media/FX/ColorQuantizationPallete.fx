#include "ColorQuantizationDefine.fx"

cbuffer cbFixed
{
	static const float2 seed = float2( 	
										23.14069263277926,  // e^pi(Gelfond's constant)
										2.665144142690225  	// 2^sqrt(2)(Gelfond&Schneider constant)
										);
	static const float3 seed3 = float3( 	
										23.14069263277926,  // e^pi(Gelfond's constant)
										2.665144142690225,  	// 2^sqrt(2)(Gelfond&Schneider constant)
										15.15426224147927 	// e^e
										);
}
float random( float2 p )
{
	int unused;
	return modf( cos( fmod( 123456789, 1e-7 + 256 * dot(seed,p))), unused);
}
float random( float3 p )
{
	int unused;
	return modf( cos( fmod( 123456789, 1e-7 + 256 * dot(seed3,p))), unused);
}
RWTexture2D<float4> gPallete;
RWTexture2D<float4> gPalleteCount;

[numthreads(PALLETE_WIDTH,PALLETE_WIDTH,1)]
void InitCS(int3 dispatchThreadID : SV_DispatchThreadID)
{
	float4 color = float4( 	
							random(float3(dispatchThreadID.xy,1)),
							random(float3(dispatchThreadID.xy,2)),
							random(float3(dispatchThreadID.xy,3)),
							random(float3(dispatchThreadID.xy,4))
							);
    //color = float4(1,1,1,1);								
	gPallete[dispatchThreadID.xy] = color;
	gPalleteCount[dispatchThreadID.xy] = float4( 0, 0, 0, 0);
}

[numthreads(PALLETE_WIDTH,PALLETE_WIDTH,1)]
void AmendColorCS(	int3 groupThreadID : SV_GroupThreadID,
			int3 dispatchThreadID : SV_DispatchThreadID )
{
	float count = gPalleteCount[dispatchThreadID.xy].r;
	if( count > 0 )
	{
		gPallete[dispatchThreadID.xy] = gPallete[dispatchThreadID.xy] / count;
	}
	gPalleteCount[dispatchThreadID.xy] = float4( 0, 0, 0, 0);
}

technique11 Init
{
	pass P0
	{
		SetVertexShader(NULL);
		SetPixelShader(NULL);
		SetComputeShader( CompileShader(cs_5_0, InitCS()) );
	}
}

technique11 CalcColor
{
	pass P0
	{
		SetVertexShader(NULL);
		SetPixelShader(NULL);
		SetComputeShader( CompileShader(cs_5_0, AmendColorCS()) );
	}
}