#define VERTEX_TANGENT
#define SKINNED_MESH
#include "BasicVertex.fx"
#include "BasicLightPixel.fx"

technique11 Tex
{
	Pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, BasicVS() ) );
		SetHullShader( NULL );
		SetDomainShader( NULL );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, BasicLightPS(1,true,false,false,false)));
	}
}