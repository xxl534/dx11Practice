#pragma once

#include "d3dUtil.h"

#pragma region Effect
class Effect
{
public:
	Effect(ID3D11Device* device, const std::wstring& filename);
	virtual ~Effect();

	void UnbindViews(ID3D11DeviceContext* dc);
private:
	Effect(const Effect& rhs);
	Effect& operator=(const Effect& rhs);
protected:
	ID3DX11Effect* mFX;
};
#pragma endregion

#pragma region BasicEffect
class BasicEffect : public Effect
{
public:
	BasicEffect(ID3D11Device* device, const std::wstring& filename);
	~BasicEffect();

	void SetWorldViewProj(CXMMATRIX M) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorld(CXMMATRIX M) { World->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldInvTranspose(CXMMATRIX M) { WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetTexTransform(CXMMATRIX M) { TexTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetEyePosW(const XMFLOAT3& v) { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	void SetFogColor(const FXMVECTOR v) { FogColor->SetFloatVector(reinterpret_cast<const float*>(&v)); }
	void SetFogStart(float f) { FogStart->SetFloat(f); }
	void SetFogRange(float f) { FogRange->SetFloat(f); }
	void SetDirLights(const DirectionalLight* lights) { DirLights->SetRawValue(lights, 0, 3 * sizeof(DirectionalLight)); }
	void SetMaterial(const Material& mat) { Mat->SetRawValue(&mat, 0, sizeof(Material)); }
	void SetDiffuseMap(ID3D11ShaderResourceView* tex) { DiffuseMap->SetResource(tex); }

	ID3DX11EffectTechnique* PointTexTech;

	ID3DX11EffectTechnique* Light1Tech;
	ID3DX11EffectTechnique* Light2Tech;
	ID3DX11EffectTechnique* Light3Tech;

	ID3DX11EffectTechnique* Light0TexTech;
	ID3DX11EffectTechnique* Light1TexTech;
	ID3DX11EffectTechnique* Light2TexTech;
	ID3DX11EffectTechnique* Light3TexTech;

	ID3DX11EffectTechnique* Light0TexAlphaClipTech;
	ID3DX11EffectTechnique* Light1TexAlphaClipTech;
	ID3DX11EffectTechnique* Light2TexAlphaClipTech;
	ID3DX11EffectTechnique* Light3TexAlphaClipTech;

	ID3DX11EffectTechnique* Light1FogTech;
	ID3DX11EffectTechnique* Light2FogTech;
	ID3DX11EffectTechnique* Light3FogTech;

	ID3DX11EffectTechnique* Light0TexFogTech;
	ID3DX11EffectTechnique* Light1TexFogTech;
	ID3DX11EffectTechnique* Light2TexFogTech;
	ID3DX11EffectTechnique* Light3TexFogTech;

	ID3DX11EffectTechnique* Light0TexAlphaClipFogTech;
	ID3DX11EffectTechnique* Light1TexAlphaClipFogTech;
	ID3DX11EffectTechnique* Light2TexAlphaClipFogTech;
	ID3DX11EffectTechnique* Light3TexAlphaClipFogTech;

	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectMatrixVariable* World;
	ID3DX11EffectMatrixVariable* WorldInvTranspose;
	ID3DX11EffectMatrixVariable* TexTransform;
	ID3DX11EffectVectorVariable* EyePosW;
	ID3DX11EffectVectorVariable* FogColor;
	ID3DX11EffectScalarVariable* FogStart;
	ID3DX11EffectScalarVariable* FogRange;
	ID3DX11EffectVariable* DirLights;
	ID3DX11EffectVariable* Mat;

	ID3DX11EffectShaderResourceVariable* DiffuseMap;
};
#pragma endregion

#pragma region ColorQuantPalleteEffect
class ColorQuantPalleteEffect : public Effect
{
public:
	ColorQuantPalleteEffect(ID3D11Device* device, const std::wstring& filename);
	~ColorQuantPalleteEffect();

	void SetPalleteTex(ID3D11UnorderedAccessView* tex) { mPalleteMap->SetUnorderedAccessView(tex); }
	void SetPalleteCountTex(ID3D11UnorderedAccessView* tex) { mPalleteCountMap->SetUnorderedAccessView(tex); }

	ID3DX11EffectTechnique* InitTech;
	ID3DX11EffectTechnique* CalcColorTech;
private:
	ID3DX11EffectUnorderedAccessViewVariable* mPalleteMap;
	ID3DX11EffectUnorderedAccessViewVariable* mPalleteCountMap;
};
#pragma endregion

#pragma region ColorQuantIterateEffect
class ColorQuantIterateEffect : public Effect
{
public:
	ColorQuantIterateEffect(ID3D11Device* device, const std::wstring& filename);
	~ColorQuantIterateEffect();

	void SetPelleteTex(ID3D11ShaderResourceView* tex) { mPalleteMap->SetResource(tex); }
	void SetInputTex(ID3D11ShaderResourceView* tex) { mInputMap->SetResource(tex); }
	void SetPointSize(int nSize) { mPointSize->SetInt(nSize); }
	
	ID3DX11EffectTechnique* IterateTech;
private:
	ID3DX11EffectScalarVariable* mPointSize;	
	ID3DX11EffectShaderResourceVariable* mInputMap;
	ID3DX11EffectShaderResourceVariable* mPalleteMap;
};
#pragma endregion

#pragma region ColorQuantOutputEffect
class ColorQuantOutputEffect : public Effect
{
public:
	ColorQuantOutputEffect(ID3D11Device* device, const std::wstring& filename);
	~ColorQuantOutputEffect();

	void SetPelleteTex(ID3D11ShaderResourceView* tex) { mPalleteMap->SetResource(tex); }
	void SetInputTex(ID3D11ShaderResourceView* tex) { mInputMap->SetResource(tex); }
	void SetOutputTex(ID3D11UnorderedAccessView* tex) { mOutputMap->SetUnorderedAccessView(tex); }
	void SetOutputPalleteUsedTex(ID3D11UnorderedAccessView* tex) { mOutputPalleteUsedMap->SetUnorderedAccessView(tex); }

	ID3DX11EffectTechnique* OutputTech;
private:
	ID3DX11EffectShaderResourceVariable* mInputMap;
	ID3DX11EffectShaderResourceVariable* mPalleteMap;
	ID3DX11EffectUnorderedAccessViewVariable* mOutputMap;
	ID3DX11EffectUnorderedAccessViewVariable* mOutputPalleteUsedMap;
};
#pragma endregion

#pragma region Effects
class Effects
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static BasicEffect* BasicFX;
	static ColorQuantPalleteEffect* ColorQuantPalleteFX;
	static ColorQuantIterateEffect* ColorQuantIterateFX;
	static ColorQuantOutputEffect* ColorQuantOutputFX;
};
#pragma endregion



