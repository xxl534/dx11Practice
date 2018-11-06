#pragma once

#include "d3dUtil.h"

class Camera;
struct DirectionalLight;

class Terrain
{
public:
	struct InitInfo
	{
		std::wstring HeightMapFileName;
		std::wstring LayerMapFileNames[5];
		std::wstring BlendMapFileName;
		float HeightScale;
		UINT HeightMapWidth;
		UINT HeightMapHeight;
		float CellSpacing;
	};

public:
	Terrain();
	~Terrain();

	float GetWidth() const;
	float GetDepth() const;
	float GetHeight(float x, float z) const;

	XMMATRIX GetWorld() const;
	void SetWorld(CXMMATRIX M);
	void Init(ID3D11Device* device, ID3D11DeviceContext* dc, const InitInfo& initInfo);
	void Draw(ID3D11DeviceContext* dc, const Camera& cam, DirectionalLight lights[3]);
private:
	void LoadHeightMap();
	void Smooth();
	bool InBounds(int i, int j);
	float Average(int i, int j);
	void CalcAllPatchBoundsY();
	void CalcPatchBoundsY(UINT i, UINT j);
	void BuildQuadPatchVB(ID3D11Device* device);
	void BuildQuadPatchIB(ID3D11Device* device);
	void BuildHeightmapSRV(ID3D11Device* device);
private:
	static const int CellsPerPatch = 64;

	ID3D11Buffer* mQuadPatchVB;
	ID3D11Buffer* mQuadPatchIB;

	ID3D11ShaderResourceView* mLayerMapArraySRV;
	ID3D11ShaderResourceView* mBlendMapSRV;
	ID3D11ShaderResourceView* mHeightMapSRV;

	InitInfo mInfo;

	UINT mNumPatchVertices;
	UINT mNumPatchQuadFaces;

	UINT mNumPatchVertRows;
	UINT mNumPatchVertCols;

	XMFLOAT4X4 mWorld;
	Material  mMat;

	std::vector<XMFLOAT2> mPatchBoundsY;
	std::vector<float> mHeightMap;
};