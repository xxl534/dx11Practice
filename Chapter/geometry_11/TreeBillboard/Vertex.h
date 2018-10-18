#pragma once

#include "d3dUtil.h"

namespace Vertex
{
	// Basic 32-byte vertex structure.
	struct Basic32
	{
		Basic32() :Pos(0.f,0.f,0.f),Normal(0.f,0.f,0.f),Tex(0.f,0.f){}
		Basic32(const XMFLOAT3& p, const XMFLOAT3 n, const XMFLOAT2 t)
			:Pos(p), Normal(n), Tex(t) {}
		Basic32(float px, float py, float pz, float nx,float ny,float nz, float tx, float ty)
			:Pos(px,py,pz),Normal(nx,ny,nz),Tex(tx,ty){}
		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
		XMFLOAT2 Tex;
	};

	struct TreePointSprite
	{
		XMFLOAT3 Pos;
		XMFLOAT2 Size;
	};
}

class InputLayoutDesc
{
public:
	// Init like const int A::a[4] = {0, 1, 2, 3}; in .cpp file.
	static const D3D11_INPUT_ELEMENT_DESC Basic32[3];
	static const D3D11_INPUT_ELEMENT_DESC TreePointSprite[2];
};

class InputLayouts
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static ID3D11InputLayout* Basic32;
	static ID3D11InputLayout* TreePointSprite;
};