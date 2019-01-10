#include "Effects.h"
#include <d3dcompiler.h>
#pragma region Effect
Effect::Effect(ID3D11Device* device, const std::wstring& filename)
	: mFX(NULL)
{
	if (filename.size() >= 4)
	{
		bool preCompiled = false;
		std::wstring postFix = filename.substr(filename.size() - 4, 4);
		preCompiled = postFix == L".fxo";

		if (preCompiled)
		{
			std::ifstream fin(filename, std::ios::binary);

			fin.seekg(0, std::ios_base::end);
			int size = (int)fin.tellg();
			fin.seekg(0, std::ios_base::beg);
			std::vector<char> data(size);

			fin.read(&data[0], size);
			fin.close();

			HR(D3DX11CreateEffectFromMemory(&data[0], size,
				0, device, &mFX));

		}
		else
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

			HR(hr);

			HR(D3DX11CreateEffectFromMemory(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(),
				0, device, &mFX));

			ReleaseCOM(compiledShader);
		}
	}
}

Effect::~Effect()
{
	ReleaseCOM(mFX);
}

ID3D11ShaderResourceView* nullSRVs[8] = { NULL };
ID3D11UnorderedAccessView* nullUAVs[8] = { NULL };
void Effect::UnbindViews(ID3D11DeviceContext* dc)
{
	dc->VSSetShaderResources(0, 8, nullSRVs);
	dc->GSSetShaderResources(0, 8, nullSRVs);
	dc->HSSetShaderResources(0, 8, nullSRVs);
	dc->DSSetShaderResources(0, 8, nullSRVs);
	dc->PSSetShaderResources(0, 8, nullSRVs);
	dc->CSSetShaderResources(0, 8, nullSRVs);
	dc->CSSetUnorderedAccessViews(0, 8, nullUAVs, NULL);
}
#pragma endregion

#pragma region BasicEffect
BasicEffect::BasicEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	PointTexTech = mFX->GetTechniqueByName("PointTex");

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
}

BasicEffect::~BasicEffect()
{
}
#pragma endregion

#pragma region PhongTessSkinnedEffect
PhongTessSkinnedEffect::PhongTessSkinnedEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	TriangleTexTech = mFX->GetTechniqueByName("TriangleTex");

	WorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	ViewProj = mFX->GetVariableByName("gViewProj")->AsMatrix();
	World = mFX->GetVariableByName("gWorld")->AsMatrix();
	WorldInvTranspose = mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	TexTransform = mFX->GetVariableByName("gTexTransform")->AsMatrix();
	EyePosW = mFX->GetVariableByName("gEyePosition")->AsVector();
	BoneTransforms = mFX->GetVariableByName("gBoneTransforms")->AsMatrix();
	Mat = mFX->GetVariableByName("gMaterial");
	MaxTessDistance = mFX->GetVariableByName("gMaxTessDistance")->AsScalar();
	MinTessDistance = mFX->GetVariableByName("gMinTessDistance")->AsScalar();
	MinTessFactor = mFX->GetVariableByName("gMinTessFactor")->AsScalar();
	MaxTessFactor = mFX->GetVariableByName("gMaxTessFactor")->AsScalar();
	DiffuseMap = mFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
	NormalMap = mFX->GetVariableByName("gNormalMap")->AsShaderResource();
	DirLights = mFX->GetVariableByName("gDirLights");
}

PhongTessSkinnedEffect::~PhongTessSkinnedEffect()
{
}
#pragma endregion

#pragma region BasicSkinnedEffect
BasicSkinnedEffect::BasicSkinnedEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	TexTech = mFX->GetTechniqueByName("Tex");

	WorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	World = mFX->GetVariableByName("gWorld")->AsMatrix();
	WorldInvTranspose = mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	TexTransform = mFX->GetVariableByName("gTexTransform")->AsMatrix();
	EyePosW = mFX->GetVariableByName("gEyePosition")->AsVector();
	BoneTransforms = mFX->GetVariableByName("gBoneTransforms")->AsMatrix();
	Mat = mFX->GetVariableByName("gMaterial");
	DiffuseMap = mFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
	NormalMap = mFX->GetVariableByName("gNormalMap")->AsShaderResource();
	DirLights = mFX->GetVariableByName("gDirLights");
}

BasicSkinnedEffect::~BasicSkinnedEffect()
{
}
#pragma endregion

#pragma region Effects

BasicEffect* Effects::BasicFX = NULL;
PhongTessSkinnedEffect* Effects::PhongTessSkinnedFX = NULL;

BasicSkinnedEffect* Effects::BasicSkinnedFX = NULL;

void Effects::InitAll(ID3D11Device* device)
{
	BasicSkinnedFX = new BasicSkinnedEffect(device, L"FX/BasicSkinnedEffect.fx");
	PhongTessSkinnedFX = new PhongTessSkinnedEffect(device, L"FX/PhongTessellationSkinnedEffect.fx");
	BasicFX = new BasicEffect(device, L"FX/Basic.fxo");
}

void Effects::DestroyAll()
{
	SafeDelete(BasicFX);
	SafeDelete(PhongTessSkinnedFX);
	SafeDelete(BasicSkinnedFX);
}
#pragma endregion
