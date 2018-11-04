#include "Effects.h"
#include <d3dcompiler.h>
#pragma region Effect
Effect::Effect(ID3D11Device* device, const std::wstring& filename)
	: mFX(NULL)
{
	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
	ID3D10Blob* compiledShader = NULL;
	ID3D10Blob* compilationMsgs = NULL;
	HRESULT hr = D3DCompileFromFile(filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, 0, "fx_5_0", shaderFlags, 0, &compiledShader, &compilationMsgs);
	if (compilationMsgs != NULL)
	{
		MessageBoxA(0, (char*)compilationMsgs->GetBufferPointer(), 0, 0);
		ReleaseCOM(compilationMsgs);
	}

	HR(D3DX11CreateEffectFromMemory(compiledShader->GetBufferPointer(),compiledShader->GetBufferSize(),
		0, device, &mFX));
}

Effect::~Effect()
{
	ReleaseCOM(mFX);
}
#pragma endregion

#pragma region BasicEffect
BasicEffect::BasicEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	Light1Tech = mFX->GetTechniqueByName("Light1");
	Light2Tech = mFX->GetTechniqueByName("Light2");
	Light3Tech = mFX->GetTechniqueByName("Light3");

	Light0TexTech = mFX->GetTechniqueByName("Light0Tex");
	Light1TexTech = mFX->GetTechniqueByName("Light1Tex");
	Light2TexTech = mFX->GetTechniqueByName("Light2Tex");
	Light3TexTech = mFX->GetTechniqueByName("Light3Tex");

	Light0TexAlphaClipTech = mFX->GetTechniqueByName("Light0TexAlphaClip");
	Light1TexAlphaClipTech = mFX->GetTechniqueByName("Light1TexAlphaClip");
	Light2TexAlphaClipTech = mFX->GetTechniqueByName("Light2TexAlphaClip");
	Light3TexAlphaClipTech = mFX->GetTechniqueByName("Light3TexAlphaClip");

	Light1FogTech = mFX->GetTechniqueByName("Light1Fog");
	Light2FogTech = mFX->GetTechniqueByName("Light2Fog");
	Light3FogTech = mFX->GetTechniqueByName("Light3Fog");

	Light0TexFogTech = mFX->GetTechniqueByName("Light0TexFog");
	Light1TexFogTech = mFX->GetTechniqueByName("Light1TexFog");
	Light2TexFogTech = mFX->GetTechniqueByName("Light2TexFog");
	Light3TexFogTech = mFX->GetTechniqueByName("Light3TexFog");

	Light0TexAlphaClipFogTech = mFX->GetTechniqueByName("Light0TexAlphaClipFog");
	Light1TexAlphaClipFogTech = mFX->GetTechniqueByName("Light1TexAlphaClipFog");
	Light2TexAlphaClipFogTech = mFX->GetTechniqueByName("Light2TexAlphaClipFog");
	Light3TexAlphaClipFogTech = mFX->GetTechniqueByName("Light3TexAlphaClipFog");

	Light1ReflectTech = mFX->GetTechniqueByName("Light1Reflect");
	Light2ReflectTech = mFX->GetTechniqueByName("Light2Reflect");
	Light3ReflectTech = mFX->GetTechniqueByName("Light3Reflect");

	Light0TexReflectTech = mFX->GetTechniqueByName("Light0TexReflect");
	Light1TexReflectTech = mFX->GetTechniqueByName("Light1TexReflect");
	Light2TexReflectTech = mFX->GetTechniqueByName("Light2TexReflect");
	Light3TexReflectTech = mFX->GetTechniqueByName("Light3TexReflect");

	Light0TexAlphaClipReflectTech = mFX->GetTechniqueByName("Light0TexAlphaClipReflect");
	Light1TexAlphaClipReflectTech = mFX->GetTechniqueByName("Light1TexAlphaClipReflect");
	Light2TexAlphaClipReflectTech = mFX->GetTechniqueByName("Light2TexAlphaClipReflect");
	Light3TexAlphaClipReflectTech = mFX->GetTechniqueByName("Light3TexAlphaClipReflect");

	Light1FogReflectTech = mFX->GetTechniqueByName("Light1FogReflect");
	Light2FogReflectTech = mFX->GetTechniqueByName("Light2FogReflect");
	Light3FogReflectTech = mFX->GetTechniqueByName("Light3FogReflect");

	Light0TexFogReflectTech = mFX->GetTechniqueByName("Light0TexFogReflect");
	Light1TexFogReflectTech = mFX->GetTechniqueByName("Light1TexFogReflect");
	Light2TexFogReflectTech = mFX->GetTechniqueByName("Light2TexFogReflect");
	Light3TexFogReflectTech = mFX->GetTechniqueByName("Light3TexFogReflect");

	Light0TexAlphaClipFogReflectTech = mFX->GetTechniqueByName("Light0TexAlphaClipFogReflect");
	Light1TexAlphaClipFogReflectTech = mFX->GetTechniqueByName("Light1TexAlphaClipFogReflect");
	Light2TexAlphaClipFogReflectTech = mFX->GetTechniqueByName("Light2TexAlphaClipFogReflect");
	Light3TexAlphaClipFogReflectTech = mFX->GetTechniqueByName("Light3TexAlphaClipFogReflect");

	WorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	World = mFX->GetVariableByName("gWorld")->AsMatrix();
	WorldInvTranspose = mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	TexTransform = mFX->GetVariableByName("gTexTransform")->AsMatrix();
	EyePosW = mFX->GetVariableByName("gEyePosW")->AsVector();
	FogColor = mFX->GetVariableByName("gFogColor")->AsVector();
	FogStart = mFX->GetVariableByName("gFogStart")->AsScalar();
	FogRange = mFX->GetVariableByName("gFogRange")->AsScalar();
	DirLights = mFX->GetVariableByName("gDirLights");
	Mat = mFX->GetVariableByName("gMaterial");
	DiffuseMap = mFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
	CubeMap = mFX->GetVariableByName("gCubeMap")->AsShaderResource();
}

BasicEffect::~BasicEffect()
{
}
#pragma endregion

#pragma region SkyEffect
SkyEffect::SkyEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	SkyTech = mFX->GetTechniqueByName("SkyTech");
	WorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	CubeMap = mFX->GetVariableByName("gCubeMap")->AsShaderResource();
}

SkyEffect::~SkyEffect()
{
}
#pragma endregion

#pragma region NormalMapEffect
NormalMapEffect::NormalMapEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	Light1Tech = mFX->GetTechniqueByName("Light1");
	Light2Tech = mFX->GetTechniqueByName("Light2");
	Light3Tech = mFX->GetTechniqueByName("Light3");

	Light0TexTech = mFX->GetTechniqueByName("Light0Tex");
	Light1TexTech = mFX->GetTechniqueByName("Light1Tex");
	Light2TexTech = mFX->GetTechniqueByName("Light2Tex");
	Light3TexTech = mFX->GetTechniqueByName("Light3Tex");

	Light0TexAlphaClipTech = mFX->GetTechniqueByName("Light0TexAlphaClip");
	Light1TexAlphaClipTech = mFX->GetTechniqueByName("Light1TexAlphaClip");
	Light2TexAlphaClipTech = mFX->GetTechniqueByName("Light2TexAlphaClip");
	Light3TexAlphaClipTech = mFX->GetTechniqueByName("Light3TexAlphaClip");

	Light1FogTech = mFX->GetTechniqueByName("Light1Fog");
	Light2FogTech = mFX->GetTechniqueByName("Light2Fog");
	Light3FogTech = mFX->GetTechniqueByName("Light3Fog");

	Light0TexFogTech = mFX->GetTechniqueByName("Light0TexFog");
	Light1TexFogTech = mFX->GetTechniqueByName("Light1TexFog");
	Light2TexFogTech = mFX->GetTechniqueByName("Light2TexFog");
	Light3TexFogTech = mFX->GetTechniqueByName("Light3TexFog");

	Light0TexAlphaClipFogTech = mFX->GetTechniqueByName("Light0TexAlphaClipFog");
	Light1TexAlphaClipFogTech = mFX->GetTechniqueByName("Light1TexAlphaClipFog");
	Light2TexAlphaClipFogTech = mFX->GetTechniqueByName("Light2TexAlphaClipFog");
	Light3TexAlphaClipFogTech = mFX->GetTechniqueByName("Light3TexAlphaClipFog");

	Light1ReflectTech = mFX->GetTechniqueByName("Light1Reflect");
	Light2ReflectTech = mFX->GetTechniqueByName("Light2Reflect");
	Light3ReflectTech = mFX->GetTechniqueByName("Light3Reflect");

	Light0TexReflectTech = mFX->GetTechniqueByName("Light0TexReflect");
	Light1TexReflectTech = mFX->GetTechniqueByName("Light1TexReflect");
	Light2TexReflectTech = mFX->GetTechniqueByName("Light2TexReflect");
	Light3TexReflectTech = mFX->GetTechniqueByName("Light3TexReflect");

	Light0TexAlphaClipReflectTech = mFX->GetTechniqueByName("Light0TexAlphaClipReflect");
	Light1TexAlphaClipReflectTech = mFX->GetTechniqueByName("Light1TexAlphaClipReflect");
	Light2TexAlphaClipReflectTech = mFX->GetTechniqueByName("Light2TexAlphaClipReflect");
	Light3TexAlphaClipReflectTech = mFX->GetTechniqueByName("Light3TexAlphaClipReflect");

	Light1FogReflectTech = mFX->GetTechniqueByName("Light1FogReflect");
	Light2FogReflectTech = mFX->GetTechniqueByName("Light2FogReflect");
	Light3FogReflectTech = mFX->GetTechniqueByName("Light3FogReflect");

	Light0TexFogReflectTech = mFX->GetTechniqueByName("Light0TexFogReflect");
	Light1TexFogReflectTech = mFX->GetTechniqueByName("Light1TexFogReflect");
	Light2TexFogReflectTech = mFX->GetTechniqueByName("Light2TexFogReflect");
	Light3TexFogReflectTech = mFX->GetTechniqueByName("Light3TexFogReflect");

	Light0TexAlphaClipFogReflectTech = mFX->GetTechniqueByName("Light0TexAlphaClipFogReflect");
	Light1TexAlphaClipFogReflectTech = mFX->GetTechniqueByName("Light1TexAlphaClipFogReflect");
	Light2TexAlphaClipFogReflectTech = mFX->GetTechniqueByName("Light2TexAlphaClipFogReflect");
	Light3TexAlphaClipFogReflectTech = mFX->GetTechniqueByName("Light3TexAlphaClipFogReflect");

	WorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	World = mFX->GetVariableByName("gWorld")->AsMatrix();
	WorldInvTranspose = mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	TexTransform = mFX->GetVariableByName("gTexTransform")->AsMatrix();
	EyePosW = mFX->GetVariableByName("gEyePosW")->AsVector();
	FogColor = mFX->GetVariableByName("gFogColor")->AsVector();
	FogStart = mFX->GetVariableByName("gFogStart")->AsScalar();
	FogRange = mFX->GetVariableByName("gFogRange")->AsScalar();
	DirLights = mFX->GetVariableByName("gDirLights");
	Mat = mFX->GetVariableByName("gMaterial");
	DiffuseMap = mFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
	CubeMap = mFX->GetVariableByName("gCubeMap")->AsShaderResource();
	NormalMap = mFX->GetVariableByName("gNormalMap")->AsShaderResource();
}

NormalMapEffect::~NormalMapEffect()
{
}
#pragma endregion

#pragma region DisplacementMapEffect
DisplacementMapEffect::DisplacementMapEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	Light1Tech = mFX->GetTechniqueByName("Light1");
	Light2Tech = mFX->GetTechniqueByName("Light2");
	Light3Tech = mFX->GetTechniqueByName("Light3");

	Light0TexTech = mFX->GetTechniqueByName("Light0Tex");
	Light1TexTech = mFX->GetTechniqueByName("Light1Tex");
	Light2TexTech = mFX->GetTechniqueByName("Light2Tex");
	Light3TexTech = mFX->GetTechniqueByName("Light3Tex");

	Light0TexAlphaClipTech = mFX->GetTechniqueByName("Light0TexAlphaClip");
	Light1TexAlphaClipTech = mFX->GetTechniqueByName("Light1TexAlphaClip");
	Light2TexAlphaClipTech = mFX->GetTechniqueByName("Light2TexAlphaClip");
	Light3TexAlphaClipTech = mFX->GetTechniqueByName("Light3TexAlphaClip");

	Light1FogTech = mFX->GetTechniqueByName("Light1Fog");
	Light2FogTech = mFX->GetTechniqueByName("Light2Fog");
	Light3FogTech = mFX->GetTechniqueByName("Light3Fog");

	Light0TexFogTech = mFX->GetTechniqueByName("Light0TexFog");
	Light1TexFogTech = mFX->GetTechniqueByName("Light1TexFog");
	Light2TexFogTech = mFX->GetTechniqueByName("Light2TexFog");
	Light3TexFogTech = mFX->GetTechniqueByName("Light3TexFog");

	Light0TexAlphaClipFogTech = mFX->GetTechniqueByName("Light0TexAlphaClipFog");
	Light1TexAlphaClipFogTech = mFX->GetTechniqueByName("Light1TexAlphaClipFog");
	Light2TexAlphaClipFogTech = mFX->GetTechniqueByName("Light2TexAlphaClipFog");
	Light3TexAlphaClipFogTech = mFX->GetTechniqueByName("Light3TexAlphaClipFog");

	Light1ReflectTech = mFX->GetTechniqueByName("Light1Reflect");
	Light2ReflectTech = mFX->GetTechniqueByName("Light2Reflect");
	Light3ReflectTech = mFX->GetTechniqueByName("Light3Reflect");

	Light0TexReflectTech = mFX->GetTechniqueByName("Light0TexReflect");
	Light1TexReflectTech = mFX->GetTechniqueByName("Light1TexReflect");
	Light2TexReflectTech = mFX->GetTechniqueByName("Light2TexReflect");
	Light3TexReflectTech = mFX->GetTechniqueByName("Light3TexReflect");

	Light0TexAlphaClipReflectTech = mFX->GetTechniqueByName("Light0TexAlphaClipReflect");
	Light1TexAlphaClipReflectTech = mFX->GetTechniqueByName("Light1TexAlphaClipReflect");
	Light2TexAlphaClipReflectTech = mFX->GetTechniqueByName("Light2TexAlphaClipReflect");
	Light3TexAlphaClipReflectTech = mFX->GetTechniqueByName("Light3TexAlphaClipReflect");

	Light1FogReflectTech = mFX->GetTechniqueByName("Light1FogReflect");
	Light2FogReflectTech = mFX->GetTechniqueByName("Light2FogReflect");
	Light3FogReflectTech = mFX->GetTechniqueByName("Light3FogReflect");

	Light0TexFogReflectTech = mFX->GetTechniqueByName("Light0TexFogReflect");
	Light1TexFogReflectTech = mFX->GetTechniqueByName("Light1TexFogReflect");
	Light2TexFogReflectTech = mFX->GetTechniqueByName("Light2TexFogReflect");
	Light3TexFogReflectTech = mFX->GetTechniqueByName("Light3TexFogReflect");

	Light0TexAlphaClipFogReflectTech = mFX->GetTechniqueByName("Light0TexAlphaClipFogReflect");
	Light1TexAlphaClipFogReflectTech = mFX->GetTechniqueByName("Light1TexAlphaClipFogReflect");
	Light2TexAlphaClipFogReflectTech = mFX->GetTechniqueByName("Light2TexAlphaClipFogReflect");
	Light3TexAlphaClipFogReflectTech = mFX->GetTechniqueByName("Light3TexAlphaClipFogReflect");

	ViewProj = mFX->GetVariableByName("gViewProj")->AsMatrix();
	WorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	World = mFX->GetVariableByName("gWorld")->AsMatrix();
	WorldInvTranspose = mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	TexTransform = mFX->GetVariableByName("gTexTransform")->AsMatrix();
	EyePosW = mFX->GetVariableByName("gEyePosW")->AsVector();
	FogColor = mFX->GetVariableByName("gFogColor")->AsVector();
	FogStart = mFX->GetVariableByName("gFogStart")->AsScalar();
	FogRange = mFX->GetVariableByName("gFogRange")->AsScalar();
	DirLights = mFX->GetVariableByName("gDirLights");
	Mat = mFX->GetVariableByName("gMaterial");
	HeightScale = mFX->GetVariableByName("gHeightScale")->AsScalar();
	MaxTessDistance = mFX->GetVariableByName("gMaxTessDistance")->AsScalar();
	MinTessDistance = mFX->GetVariableByName("gMinTessDistance")->AsScalar();
	MinTessFactor = mFX->GetVariableByName("gMinTessFactor")->AsScalar();
	MaxTessFactor = mFX->GetVariableByName("gMaxTessFactor")->AsScalar();
	DiffuseMap = mFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
	CubeMap = mFX->GetVariableByName("gCubeMap")->AsShaderResource();
	NormalMap = mFX->GetVariableByName("gNormalMap")->AsShaderResource();
}

DisplacementMapEffect::~DisplacementMapEffect()
{
}
#pragma endregion

#pragma region TerrainEffect
TerrainEffect::TerrainEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	Light0Tech = mFX->GetTechniqueByName("Light0");
	Light1Tech = mFX->GetTechniqueByName("Light1");
	Light2Tech = mFX->GetTechniqueByName("Light2");
	Light3Tech = mFX->GetTechniqueByName("Light3");
	Light1FogTech = mFX->GetTechniqueByName("Light1Fog");
	Light2FogTech = mFX->GetTechniqueByName("Light2Fog");
	Light3FogTech = mFX->GetTechniqueByName("Light3Fog");

	ViewProj = mFX->GetVariableByName("gViewProj")->AsMatrix();
	EyePosW = mFX->GetVariableByName("gEyePosW")->AsVector();
	FogColor = mFX->GetVariableByName("gFogColor")->AsVector();
	FogStart = mFX->GetVariableByName("gFogStart")->AsScalar();
	FogRange = mFX->GetVariableByName("gFogRange")->AsScalar();
	DirLights = mFX->GetVariableByName("gDirLights");
	Mat = mFX->GetVariableByName("gMaterial");

	MinDist = mFX->GetVariableByName("gMinDist")->AsScalar();
	MaxDist = mFX->GetVariableByName("gMaxDist")->AsScalar();
	MinTess = mFX->GetVariableByName("gMinTess")->AsScalar();
	MaxTess = mFX->GetVariableByName("gMaxTess")->AsScalar();
	TexelCellSpaceU = mFX->GetVariableByName("gTexelCellSpaceU")->AsScalar();
	TexelCellSpaceV = mFX->GetVariableByName("gTexelCellSpaceV")->AsScalar();
	WorldCellSpace = mFX->GetVariableByName("gWorldCellSpace")->AsScalar();
	WorldFrustumPlanes = mFX->GetVariableByName("gWorldFrustumPlanes")->AsVector();

	LayerMapArray = mFX->GetVariableByName("gLayerMapArray")->AsShaderResource();
	BlendMap = mFX->GetVariableByName("gBlendMap")->AsShaderResource();
	HeightMap = mFX->GetVariableByName("gHeightMap")->AsShaderResource();
}

TerrainEffect::~TerrainEffect()
{
}
#pragma endregion

#pragma region Effects

BasicEffect* Effects::BasicFX = NULL;
SkyEffect*   Effects::SkyFX = NULL;
TerrainEffect* Effects::TerrainFX = NULL;

void Effects::InitAll(ID3D11Device* device)
{
	BasicFX = new BasicEffect(device, L"FX/Basic.fx");
	SkyFX = new SkyEffect(device, L"FX/Sky.fx");
	TerrainFX = new TerrainEffect(device, L"FX/Terrain.fx");
}

void Effects::DestroyAll()
{
	SafeDelete(BasicFX);
	SafeDelete(TerrainFX);
	SafeDelete(SkyFX);
}
#pragma endregion