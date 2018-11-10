#include "Effects.h"
#include <d3dcompiler.h>


#pragma region Effect
Effect::Effect(ID3D11Device* device, const std::wstring& filename)
	: mFX(0)
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
#pragma endregion

#pragma region AmbientOcclusionEffect
AmbientOcclusionEffect::AmbientOcclusionEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	AmbientOcclusionTech = mFX->GetTechniqueByName("AmbientOcclusion");

	WorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
}

AmbientOcclusionEffect::~AmbientOcclusionEffect()
{
}
#pragma endregion

#pragma region Effects

AmbientOcclusionEffect* Effects::AmbientOcclusionFX = NULL;

void Effects::InitAll(ID3D11Device* device)
{
	AmbientOcclusionFX = new AmbientOcclusionEffect(device, L"FX/AmbientOcclusion.fx");
}

void Effects::DestroyAll()
{
	SafeDelete(AmbientOcclusionFX);
}
#pragma endregion