cbuffer cbPerObject
{
	float4x4 gWorldView;
	float4x4 gWorldInvTransposeView;
	float4x4 gWorldViewProj;
	float4x4 gTexTransform;
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