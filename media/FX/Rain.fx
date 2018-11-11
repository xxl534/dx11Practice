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
	float3 gAccelW = { -1.0f, -9.8f, 0.f };
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

float3 RandVec3(float offset)
{
	float u = (gGameTime + offset);
	float3 v = gRandomTex.SampleLevel(samLinear, u, 0).xyz;
	
	return v;
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

[maxvertexcount(6)]
void StreamOutGS( point Particle gin[1],
				inout PointStream<Particle> ptStream )
{
	gin[0].Age += gTimeStep;
	
	if( gin[0].Type == PT_EMITTER )
	{
		if(gin[0].Age > 0.002f )
		{
			for( int i = 0; i < 5; ++i )
			{
				float3 vRandom = 35.0f * RandVec3((float)i/5.f);
				vRandom.y = 20.f;
				
				Particle p;
				p.InitialPosW = gEmitPosW.xyz + vRandom;
				p.InitialVelW = float3(0.f,0.f,0.f);
				p.SizeW 	  = float2(1.f,1.f);
				p.Age 			= 0.f;
				p.Type 			= PT_FLARE;
				
				ptStream.Append(p);
			}
			
			
			gin[0].Age = 0.0f;
		}
		
		// always keep emitters
		ptStream.Append(gin[0]);
	}
	else
	{
		if( gin[0].Age <= 3.0f )
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
	uint Type : TYPE;
};

VertexOut DrawVS( Particle vin )
{
	VertexOut vout;
	
	float t = vin.Age;
	
	vout.PosW = 0.5f * t * t * gAccelW + t * vin.InitialVelW + vin.InitialPosW;
	
	vout.Type = vin.Type;
	return vout;
}

struct GeoOut
{
	float4 PosH : SV_Position;
	float2 Tex 	: TEXCOORD;
};

[maxvertexcount(2)]
void DrawGS(point VertexOut gin[1],
			inout LineStream<GeoOut> lineStream )
{
	if( gin[0].Type != PT_EMITTER )
	{
		float3 p0 = gin[0].PosW;
		float3 p1 = gin[0].PosW + 0.07f*gAccelW;
		
		GeoOut v0;
		v0.PosH = mul(float4(p0,1.f),gViewProj);
		v0.Tex = float2(0.f,0.f);
		lineStream.Append(v0);
		
		GeoOut v1;
		v1.PosH = mul(float4(p1, 1.f),gViewProj);
		v1.Tex = float2(1.f,1.f);
		lineStream.Append(v1);	
	}
}

float4 DrawPS( GeoOut pin ) : SV_TARGET
{
	return gTexArray.Sample(samLinear, float3(pin.Tex, 0 ) );
}

technique11 DrawTech
{
	Pass P0
	{
		SetVertexShader(   CompileShader( vs_5_0, DrawVS() ) );
        SetGeometryShader( CompileShader( gs_5_0, DrawGS() ) );
        SetPixelShader(    CompileShader( ps_5_0, DrawPS() ) );
		
        SetDepthStencilState( NoDepthWrites, 0 );
	}
}