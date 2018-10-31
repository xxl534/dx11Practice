#include "Vertex.h"
#include "Effects.h"
#pragma region InputLayoutDesc

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::Basic32[3] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::Pos[1] =
{
	{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
};
#pragma endregion

#pragma region InputLayouts

ID3D11InputLayout* InputLayouts::Basic32 = NULL;
ID3D11InputLayout* InputLayouts::Pos = NULL;

void InputLayouts::InitAll(ID3D11Device* device)
{
	D3DX11_PASS_DESC passDesc;

	//
	// Basic32
	//

	Effects::BasicFX->Light1Tech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(InputLayoutDesc::Basic32, 3, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &Basic32));

	Effects::IcosahedronTessellationFX->TessTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(InputLayoutDesc::Pos, 1, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &Pos));

}

void InputLayouts::DestroyAll()
{
	ReleaseCOM(Basic32);
	ReleaseCOM(Pos);
}

#pragma endregion
