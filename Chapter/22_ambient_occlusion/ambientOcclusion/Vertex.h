#pragma once
#include "d3dUtil.h"

namespace Vertex
{
	struct AmbientOcclusion
	{
		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
		XMFLOAT2 Tex;
		float AmbientAccess;
	};
}

class InputLayoutDesc
{
public:
	static const D3D11_INPUT_ELEMENT_DESC AmbientOcclusion[4];
};

class InputLayouts
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static ID3D11InputLayout* AmbientOcclusion;
};