#include "ColorQuantizationDefine.fx"

Texture2D gInput;
Texture2D gPallete;
RWTexture2D<float4> gOutputPalleteUsed;
RWTexture2D<float4> gOutput;

[numthreads(32,32,1)]
void DrawCS(int3 dispatchThreadID : SV_DispatchThreadID )
{
	if( dispatchThreadID.x < (int)gInput.Length.x && dispatchThreadID.y < (int)gInput.Length.y )
	{
		float4 texColor = gInput[dispatchThreadID.xy];
		float errorMin = 1e4;
		float4 outColor;
		int2 usedIndex;
		int row, col;
		for( row = 0; row < PALLETE_WIDTH; ++row )
		{
			for( col = 0; col < PALLETE_WIDTH; ++col )
			{
				int2 index = int2(col,row);
				float4 palleteColor = gPallete[index.xy];
				float4 colDelta = palleteColor - texColor;
				float err = dot( colDelta, colDelta);
				if( err < errorMin )
				{
					usedIndex = index;
					outColor = palleteColor;
					errorMin = err;
				}
			}
		}
		gOutput[dispatchThreadID.xy] = outColor;
		gOutputPalleteUsed[usedIndex.xy] = float4(1,1,1,1);
	}
}

technique11 Output
{
	pass P0
	{
		SetVertexShader(NULL);
		SetPixelShader(NULL);
		SetComputeShader( CompileShader(cs_5_0, DrawCS()) );
	}
}