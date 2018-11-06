cbuffer cbPerFrame
{
	float3 gEyePosW;
	
	float3 gEmitPosW;
	float3 gEmitDirW;
	
	float gGameTime;
	float gTimeStep;
	float4x4 gViewProj;
};

cbuffer cbFixed
{
	float3 gAccelW = { 0.f, 7.8f, 0.f };
	
	float2 gQuadTexC[4] = 
	{
		float2(0.f, 1.f),
		float2(1.f, 1.f),
		float2(0.f, 0.f),
		float2(1.f, 0.f)
	};
};

Texture2DArray gTexArray;

Texture1D gRandomTex;

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

DepthStencilState DisableDepth
{
	DepthEnable = FALSE;
	DepthWriteMask = ZERO;
};

DepthStencilState NoDepthWrites
{
	DepthEnable = TRUE;
	DepthWriteMask = ZERO;
};

BlendState AdditiveBlending
{
	AlphaToCoverageEnable = FALSE;
	BlendEnable[0] = TRUE;
	SrcBlend = SRC_ALPHA;
	DestBlend = ONE;
	BlendOp = ADD;
	SrcBlendAlpha = ZERO;
	DestBlendAlpha = ZERO;
	BlendOpAlpha = ADD;
	RenderTargetWriteMask[0] = 0x0F;
};

float3 RandUnitVec3(float offset)
{
	float u = (gGameTime + offset);
	float3 v = gRandomTex.SampleLevel(samLinear, u, 0).xyz;
	
	return normalize(v);
}

#define PT_EMITTER 0
#define PT_FLARE 1

struct Particle
{
	float3 InitialPosW : POSITION;
	float3 InitialVelW : VELOCITY;
	float2 SizeW 		: SIZE;
	float Age 			: AGE;
	uint Type 			: TYPE;
};

Particle StreamOutVS(Particle vin)
{
	return vin;
}

[maxvertexcount(2)]
void StreamOutGS( point Particle gin[1],
				inout PointStream<Particle> ptStream )
{
	gin[0].Age += gTimeStep;
	
	if( gin[0].Type == PT_EMITTER )
	{
		if(gin[0].Age > 0.005f )
		{
			float3 vRandom = RandUnitVec3(0.f);
			vRandom.x *= 0.5f;
			vRandom.z *= 0.5f;
			
			Particle p;
			p.InitialPosW = gEmitPosW.xyz;
			p.InitialVelW = 4.f * vRandom;
			p.SizeW 	  = float2(3.f,3.f);
			p.Age 			= 0.f;
			p.Type 			= PT_FLARE;
			
			ptStream.Append(p);
			
			gin[0].Age = 0.0f;
		}
		
		// always keep emitters
		ptStream.Append(gin[0]);
	}
	else
	{
		if( gin[0].Age <= 1.f )
		{
			ptStream.Append(gin[0]);
		}
	}
}

GeometryShader gsStreamOut = ConstructGSWithSO(
	CompileShader( gs_5_0, StreamOutGS()),
	"POSITION.xyz; VELOCITY.xyz; SIZE.xy; AGE.x; TYPE.x");
	
technique11 StreamOutTech
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, StreamOutVS() ) );
		SetGeometryShader( gsStreamOut );
		
		//disable pixel shader and the depth buffer for stream-out only
		SetPixelShader( NULL ); 
		SetDepthStencilState( DisableDepth, 0 );
	}
}

struct VertexOut
{
	float3 PosW : POSITION;
	float2 SizeW : SIZE;
	float4 Color : COLOR;
	uint Type : TYPE;
};

VertexOut DrawVS( Particle vin )
{
	VertexOut vout;
	
	float t = vin.Age;
	
	vout.PosW = 0.5f * t * t * gAccelW + t * vin.InitialVelW + vin.InitialPosW;
	
	float opacity = 1.f - smoothstep( 0.f, 1.f, t/1.f);
	vout.Color = float4( 1.f, 1.f, 1.f, opacity );
	
	vout.SizeW = vin.SizeW;
	vout.Type = vin.Type;
	return vout;
}

struct GeoOut
{
	float4 PosH : SV_Position;
	float4 Color : COLOR;
	float2 Tex 	: TEXCOORD;
};

[maxvertexcount(4)]
void DrawGS(point VertexOut gin[1],
			inout TriangleStream<GeoOut> triStream)
{
	if( gin[0].Type != PT_EMITTER )
	{
		float3 look  = normalize(gEyePosW.xyz - gin[0].PosW);
		float3 right = normalize(cross(float3(0,1,0), look));
		float3 up    = cross(look, right);
		
		//
		// Compute triangle strip vertices (quad) in world space.
		//
		float halfWidth  = 0.5f*gin[0].SizeW.x;
		float halfHeight = 0.5f*gin[0].SizeW.y;
	
		float4 v[4];
		v[0] = float4(gin[0].PosW + halfWidth*right - halfHeight*up, 1.0f);
		v[1] = float4(gin[0].PosW + halfWidth*right + halfHeight*up, 1.0f);
		v[2] = float4(gin[0].PosW - halfWidth*right - halfHeight*up, 1.0f);
		v[3] = float4(gin[0].PosW - halfWidth*right + halfHeight*up, 1.0f);
		
		//
		// Transform quad vertices to world space and output 
		// them as a triangle strip.
		//
		GeoOut gout;
		[unroll]
		for(int i = 0; i < 4; ++i)
		{
			gout.PosH  = mul(v[i], gViewProj);
			gout.Tex   = gQuadTexC[i];
			gout.Color = gin[0].Color;
			triStream.Append(gout);
		}	
	}
}

float4 DrawPS( GeoOut pin ) : SV_TARGET
{
	return gTexArray.Sample(samLinear, float3(pin.Tex, 0 ) )*pin.Color;
}

technique11 DrawTech
{
	Pass P0
	{
		SetVertexShader(   CompileShader( vs_5_0, DrawVS() ) );
        SetGeometryShader( CompileShader( gs_5_0, DrawGS() ) );
        SetPixelShader(    CompileShader( ps_5_0, DrawPS() ) );
        
        SetBlendState(AdditiveBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff);
        SetDepthStencilState( NoDepthWrites, 0 );
	}
}