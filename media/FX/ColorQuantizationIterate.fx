#include "ColorQuantizationDefine.fx"
cbuffer cbPerIteration
{
	int gPointSize = 0;
}
Texture2D gInput;
Texture2D gInputPallete;

struct VertexIn
{
	int2 Pos : POSITION;
};

struct VertexOut
{
	float4 Color 		: COLOR;
	int2 PalleteIndex : TEXCOORD; 
};
VertexOut VS( VertexIn vin )
{
	VertexOut vout;
	int col, row;
	int2 indexMin;
	float errorMin = 10000.f; //large value
	float4 texColor = gInput[vin.Pos.xy];
	for( row = 0; row < PALLETE_WIDTH; ++row )
	{
		for( col = 0; col < PALLETE_WIDTH; ++col )
		{
			int2 index = int2(col,row);
			float4 colDelta = gInputPallete[index.xy] - texColor;
			float err = dot( colDelta, colDelta);
			if( err < errorMin )
			{
				indexMin = int2(col,row);
				errorMin = err;
			}
		}
	}
	vout.Color = texColor;
	vout.PalleteIndex = indexMin;
	return vout;
}


struct GeoOut
{
	float4 PosH : SV_POSITION;
	float4 Color : COLOR;
};

[maxvertexcount(64)]
void GS(point VertexOut gin[1],
		inout PointStream<GeoOut> pointStream)
{
	GeoOut gout;
	gout.Color = gin[0].Color;
	for( int row = -gPointSize; row <= gPointSize; ++row )
	{
		for( int col = -gPointSize; col <= gPointSize; ++col )
		{
			//directx11 homogeneous coordinate visible range : x:(-1,1] ; y:[-1,1).
			gout.PosH = float4( float( (gin[0].PalleteIndex.x + 1 + col ) * 2 ) / PALLETE_WIDTH - 1, 
								float( (gin[0].PalleteIndex.y + row) * 2 ) / PALLETE_WIDTH - 1, 
								0, 1);
								
			pointStream.Append(gout);
		}
	}
}

struct PixelOut
{
	float4 Color0 :SV_Target0;
	float4 Color1 :SV_Target1;
};

PixelOut PS(GeoOut pin)
{
	PixelOut pout;
	pout.Color0 = pin.Color;
	//pout.Color0 = float4(1,0,0,1);
	pout.Color1 = float4(1,0,0,0);
	return pout;
}

BlendState AccumulatedBlendState
{
	BlendEnable[0] = TRUE;
	SrcBlend[0] = ONE;
	DestBlend[0] = ONE;
	BlendOp[0] = ADD;
	SrcBlendAlpha[0] = ONE;
	DestBlendAlpha[0] = ONE;
	BlendOpAlpha[0] = ADD;
	
	BlendEnable[1] = TRUE;
	SrcBlend[1] = ONE;
	DestBlend[1] = ONE;
	BlendOp[1] = ADD;
	SrcBlendAlpha[1] = ONE;
	DestBlendAlpha[1] = ONE;
	BlendOpAlpha[1] =  ADD;
};

technique11 Iterate
{
	pass p0
	{
		SetBlendState( AccumulatedBlendState, float4(0.f,0.f,0.f,0.f), 0xffffffff );
		SetVertexShader( CompileShader( vs_5_0, VS()) );
		SetGeometryShader( CompileShader( gs_5_0, GS() ) );
		SetPixelShader( CompileShader( ps_5_0, PS() ) );
	}
}