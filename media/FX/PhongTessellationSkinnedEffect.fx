#define VERTEX_TANGENT
#define SKINNED_MESH
#include "PhongTessellationVertex.fx"
#include "BasicLightPixel.fx"

technique11 TriangleTex
{
	Pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, PhongTessVS() ) );
		SetHullShader( CompileShader( hs_5_0, TriHS() ));
		SetDomainShader( CompileShader( ds_5_0, PhongTessTriDS()));
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, BasicLightPS(0,true,false,false,false)));
	}
}