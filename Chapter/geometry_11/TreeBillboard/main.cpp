#include "d3dApp.h"
#include "Waves.h"
#include "Effects.h"
#include "Vertex.h"
#include "RenderStates.h"
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"
#include "GeometryGenerator.h"
enum RenderOptions
{
	Lighting = 0,
	Textures = 1,
	TexturesAndFog = 2
};

#define  TREE_COUNT 20

class BlendApp :public D3DApp
{
public:
	BlendApp(HINSTANCE hInstance);
	~BlendApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	float GetHillHeight(float x, float z)const;
	XMFLOAT3 GetHillNormal(float x, float z)const;
	void BuildLandGeometryBuffers();
	void BuildWaveGeometryBuffers();
	void BuildCrateGeometryBuffers();
	void BuildTreeSpriteBuffer();
	void DrawTreeSprites(CXMMATRIX viewProj);
private:
	ID3D11Buffer* mLandVB;
	ID3D11Buffer* mLandIB;

	ID3D11Buffer* mWavesVB;
	ID3D11Buffer* mWavesIB;

	ID3D11Buffer* mBoxVB;
	ID3D11Buffer* mBoxIB;

	ID3D11Buffer* mTreeSpriteVB;

	ID3D11ShaderResourceView* mGrassMapSRV;
	ID3D11ShaderResourceView* mWavesMapSRV;
	ID3D11ShaderResourceView* mBoxMapSRV;
	ID3D11ShaderResourceView* mTreeTextureMapArraySRV;

	Waves mWaves;

	DirectionalLight mDirLights[3];
	Material mLandMat;
	Material mWavesMat;
	Material mBoxMat;
	Material mTreeMat;

	XMFLOAT4X4 mGrassTexTransform;
	XMFLOAT4X4 mWaterTexTransform;
	XMFLOAT4X4 mLandWorld;
	XMFLOAT4X4 mWavesWorld;
	XMFLOAT4X4 mBoxWorld;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	UINT mLandIndexCount;

	XMFLOAT2 mWaterTexOffset;

	RenderOptions mRenderOptions;

	XMFLOAT3 mEyePosW;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;

	bool mAlphaToCoverageOn;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	BlendApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

BlendApp::BlendApp(HINSTANCE hInstance)
	:D3DApp(hInstance)
	, mLandVB(NULL)
	, mLandIB(NULL)
	, mWavesVB(NULL)
	, mWavesIB(NULL)
	, mBoxVB(NULL)
	, mBoxIB(NULL)
	, mGrassMapSRV(NULL)
	, mWavesMapSRV(NULL)
	, mBoxMapSRV(NULL)
	, mWaterTexOffset(0.f, 0.f)
	, mEyePosW(0.f, 0.f, 0.f)
	, mLandIndexCount(0)
	, mRenderOptions(RenderOptions::Textures)
	,mTheta(1.3f*MathHelper::Pi)
	,mPhi(0.4f*MathHelper::Pi)
	,mRadius(80.f)
	,mAlphaToCoverageOn(true)
{
	mMainWndCaption = L"Blend Demo";
	mEnable4xMsaa = false;

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mLandWorld, I);
	XMStoreFloat4x4(&mWavesWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	XMMATRIX boxScale = XMMatrixScaling(25.f, 25.f, 25.f);
	XMMATRIX boxOffset = XMMatrixTranslation(0.f, 0.f, 0.f);
	XMStoreFloat4x4(&mBoxWorld, boxScale*boxOffset);

	XMMATRIX grassTexScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);
	XMStoreFloat4x4(&mGrassTexTransform, grassTexScale);

	mDirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

	mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Diffuse = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
	mDirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

	mLandMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mLandMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mLandMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);

	mWavesMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mWavesMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
	mWavesMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 32.0f);

	mBoxMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mBoxMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
	
	mTreeMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mTreeMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mTreeMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
}

BlendApp::~BlendApp()
{
	md3dImmediateContext->ClearState();
	ReleaseCOM(mLandVB);
	ReleaseCOM(mLandIB);
	ReleaseCOM(mWavesVB);
	ReleaseCOM(mWavesIB);
	ReleaseCOM(mBoxVB);
	ReleaseCOM(mBoxIB);
	ReleaseCOM(mTreeSpriteVB);
	ReleaseCOM(mGrassMapSRV);
	ReleaseCOM(mWavesMapSRV);
	ReleaseCOM(mBoxMapSRV);
	ReleaseCOM(mTreeTextureMapArraySRV);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool BlendApp::Init()
{
	if (!D3DApp::Init())
	{
		return false;
	}

	mWaves.Init(160, 160, 1.f, 0.03f, 5.f, 0.3f);

	Effects::InitAll(md3dDevice);
	InputLayouts::InitAll(md3dDevice);
	RenderStates::InitAll(md3dDevice);

	HR(CreateDDSTextureFromFile(md3dDevice,
		L"Textures/grass.dds", NULL, &mGrassMapSRV));

	HR(CreateDDSTextureFromFile(md3dDevice,
		L"Textures/water2.dds", NULL, &mWavesMapSRV));

	HR(CreateDDSTextureFromFile(md3dDevice,
		L"Textures/WireFence.dds", NULL, &mBoxMapSRV));

	wchar_t szBuffer[512];
	std::vector<std::wstring> arrayTreePath;
	for (UINT i = 0; i < 4; ++i)
	{
		wsprintf(szBuffer, L"Textures/tree%d.dds", i);
		arrayTreePath.push_back(szBuffer);
	}
	mTreeTextureMapArraySRV = d3dHelper::CreateDDSTexture2DArraySRV(md3dDevice, md3dImmediateContext, arrayTreePath);

	BuildLandGeometryBuffers();
	BuildWaveGeometryBuffers();
	BuildCrateGeometryBuffers();
	BuildTreeSpriteBuffer();

	return true;
}

void BlendApp::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void BlendApp::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius*sinf(mPhi)*cosf(mTheta);
	float z = mRadius*sinf(mPhi)*sinf(mTheta);
	float y = mRadius*cosf(mPhi);

	mEyePosW = XMFLOAT3(x, y, z);

	XMVECTOR pos = XMVectorSet(x, y, z, 1.f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.f, 1.f, 0.f, 0.f);

	XMMATRIX vLookAt = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, vLookAt);

	static float t_base = 0.f;
	if (mTimer.TotalTime() - t_base > 0.1f)
	{
		t_base += 0.1f;
		
		UINT i = 5 + rand() % (mWaves.RowCount() - 10);
		UINT j = 5 + rand() % (mWaves.ColumnCount() - 10);

		float r = MathHelper::RandF(0.5f, 1.f);
		mWaves.Disturb(i, j, r);
	}
	mWaves.Update(dt);

	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(md3dImmediateContext->Map(mWavesVB, NULL, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	Vertex::Basic32* pV = reinterpret_cast<Vertex::Basic32*>(mappedData.pData);
	for (UINT i = 0; i < mWaves.VertexCount(); ++i)
	{
		pV[i].Pos = mWaves[i];
		pV[i].Normal = mWaves.Normal(i);

		pV[i].Tex.x = 0.5f + mWaves[i].x / mWaves.Width();
		pV[i].Tex.y = 0.5f + mWaves[i].z / mWaves.Depth();
	}
	md3dImmediateContext->Unmap(mWavesVB, 0);

	XMMATRIX wavesScale = XMMatrixScaling(5.f, 5.f, 0.f);

	mWaterTexOffset.y += 0.05f*dt;
	mWaterTexOffset.x += 0.1f*dt;
	XMMATRIX wavesOffset = XMMatrixTranslation(mWaterTexOffset.x, mWaterTexOffset.y, 0.f);

	XMStoreFloat4x4(&mWaterTexTransform, wavesScale*wavesOffset);

	if (GetAsyncKeyState('1') & 0x8000)
	{
		mRenderOptions = RenderOptions::Lighting;
	}
	if (GetAsyncKeyState('2') & 0x8000)
	{
		mRenderOptions = RenderOptions::Textures;
	}
	if (GetAsyncKeyState('3') & 0x8000)
	{
		mRenderOptions = RenderOptions::TexturesAndFog;
	}
	if (GetAsyncKeyState('R') & 0x8000)
	{
		mAlphaToCoverageOn = true;
	}
	if (GetAsyncKeyState('T') & 0x8000)
	{
		mAlphaToCoverageOn = false;
	}
}

void BlendApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Black));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX viewProj = view*proj;

	float fogStart = 15.f;
	float fogRange = 175.f;
	XMVECTOR FogColor(Colors::Silver);

	Effects::BasicFX->SetDirLights(mDirLights);
	Effects::BasicFX->SetEyePosW(mEyePosW);
	Effects::BasicFX->SetFogColor(FogColor);
	Effects::BasicFX->SetFogStart(fogStart);
	Effects::BasicFX->SetFogRange(fogRange);

	Effects::TreeSpriteFX->SetDirLights(mDirLights);
	Effects::TreeSpriteFX->SetEyePosW(mEyePosW);
	Effects::TreeSpriteFX->SetFogColor(FogColor);
	Effects::TreeSpriteFX->SetFogStart(fogStart);
	Effects::TreeSpriteFX->SetFogRange(fogRange);

	ID3DX11EffectTechnique* boxTech;
	ID3DX11EffectTechnique* landAndWaveTech;
	ID3DX11EffectTechnique* cylinderTech = Effects::BasicFX->Light0TexAlphaClipTech;
	switch (mRenderOptions)
	{
	case Lighting:
		boxTech = Effects::BasicFX->Light3Tech;
		landAndWaveTech = Effects::BasicFX->Light3Tech;
		break;
	case Textures:
		boxTech = Effects::BasicFX->Light3TexAlphaClipTech;
		landAndWaveTech = Effects::BasicFX->Light3TexTech;
		break;
	case TexturesAndFog:
		boxTech = Effects::BasicFX->Light3TexAlphaClipFogTech;
		landAndWaveTech = Effects::BasicFX->Light3TexFogTech;
		break;
	}

	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;
	float blendFactor[] = { 0.f, 0.f, 0.f, 0.f };
	D3DX11_TECHNIQUE_DESC techDesc;
	//draw box
	boxTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX world = XMLoadFloat4x4(&mBoxWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
		Effects::BasicFX->SetMaterial(mBoxMat);
		Effects::BasicFX->SetDiffuseMap(mBoxMapSRV);

		md3dImmediateContext->RSSetState(RenderStates::NoCullRS);
		boxTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(36, 0, 0);

		md3dImmediateContext->RSSetState(NULL);
	}

	
	//draw land 
	landAndWaveTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mLandVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mLandIB, DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX world = XMLoadFloat4x4(&mLandWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&mGrassTexTransform));
		Effects::BasicFX->SetMaterial(mLandMat);
		Effects::BasicFX->SetDiffuseMap(mGrassMapSRV);

		landAndWaveTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mLandIndexCount, 0, 0);
	}

	//draw tree
	DrawTreeSprites(viewProj);

	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//draw water
	landAndWaveTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mWavesVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mWavesIB, DXGI_FORMAT_R32_UINT, 0);

		// Set per object constants.
		XMMATRIX world = XMLoadFloat4x4(&mWavesWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&mWaterTexTransform));
		Effects::BasicFX->SetMaterial(mWavesMat);
		Effects::BasicFX->SetDiffuseMap(mWavesMapSRV);

		md3dImmediateContext->OMSetBlendState(RenderStates::TransparentBS, blendFactor, 0xffffffff);
		md3dImmediateContext->OMSetDepthStencilState(RenderStates::NoDepthWriteDSS, 0);
		landAndWaveTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(3 * mWaves.TriangleCount(), 0, 0);

		// Restore default blend state
		md3dImmediateContext->OMSetBlendState(0, blendFactor, 0xffffffff);
		md3dImmediateContext->OMSetDepthStencilState(NULL, 0);
	}

	HR(mSwapChain->Present(0, 0));
}

void BlendApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void BlendApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void BlendApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box.
		mTheta += dx;
		mPhi += dy;

		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.01 unit in the scene.
		float dx = 0.1f*static_cast<float>(x - mLastMousePos.x);
		float dy = 0.1f*static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 20.0f, 500.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

float BlendApp::GetHillHeight(float x, float z) const
{
	return 0.3f*(z*sinf(0.1f*x) + x*cosf(0.1f*z));
}

DirectX::XMFLOAT3 BlendApp::GetHillNormal(float x, float z) const
{
	XMFLOAT3 n(
		-0.03f*z*cosf(0.1f*x) - 0.3f*cosf(0.1f*z),
		1.0f,
		-0.3f*sinf(0.1f*x) + 0.03f*x*sinf(0.1f*z));

	XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
	XMStoreFloat3(&n, unitNormal);

	return n;
}

void BlendApp::BuildLandGeometryBuffers()
{
	GeometryGenerator::MeshData grid;
	GeometryGenerator geoGen;
	geoGen.CreateGrid(160.f, 160.f, 50, 50, grid);
	mLandIndexCount = grid.Indices.size();

	std::vector<Vertex::Basic32> vertices(grid.Vertices.size());
	for (size_t i = 0; i < grid.Vertices.size(); ++i)
	{
		XMFLOAT3 p = grid.Vertices[i].Position;
		p.y = GetHillHeight(p.x, p.z);
		
		vertices[i].Pos = p;
		vertices[i].Normal = GetHillNormal(p.x, p.z);
		vertices[i].Tex = grid.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32)*grid.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vInitData;
	vInitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vInitData, &mLandVB));

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT)*mLandIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iInitData;
	iInitData.pSysMem = &grid.Indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iInitData, &mLandIB));
}

void BlendApp::BuildWaveGeometryBuffers()
{
	D3D11_BUFFER_DESC vdb;
	vdb.Usage = D3D11_USAGE_DYNAMIC;
	vdb.ByteWidth = sizeof(Vertex::Basic32) * mWaves.VertexCount();
	vdb.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vdb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vdb.MiscFlags = 0;
	HR(md3dDevice->CreateBuffer(&vdb, 0, &mWavesVB));

	std::vector<UINT> indices(3 * mWaves.TriangleCount());

	UINT m = mWaves.RowCount();
	UINT n = mWaves.ColumnCount();

	int k = 0;
	for (UINT i = 0; i < m - 1; ++i)
	{
		for (UINT j = 0; j < n - 1; ++j)
		{
			indices[k] = i*n + j;
			indices[k + 1] = i*n + j + 1;
			indices[k + 2] = (i + 1)*n + j;

			indices[k + 3] = (i + 1)*n + j;
			indices[k + 4] = i*n + j + 1;
			indices[k + 5] = (i + 1)*n + j + 1;

			k += 6;
		}
	}

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iInitData;
	iInitData.pSysMem = &indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iInitData, &mWavesIB));
}

void BlendApp::BuildCrateGeometryBuffers()
{
	GeometryGenerator::MeshData box;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.f, 1.f, 1.f, box);

	std::vector<Vertex::Basic32> vertices(box.Vertices.size());

	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		vertices[i].Pos = box.Vertices[i].Position;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].Tex = box.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * box.Vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vInitData;
	vInitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vInitData, &mBoxVB));

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT)*box.Indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iInitData;
	iInitData.pSysMem = &box.Indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iInitData, &mBoxIB));

}

void BlendApp::BuildTreeSpriteBuffer()
{
	Vertex::TreePointSprite v[TREE_COUNT];

	for (UINT i = 0; i < TREE_COUNT; ++i)
	{
		float x = MathHelper::RandF(-70.f, 70.f);
		float z = MathHelper::RandF(-70.f, 70.f);
		float y = GetHillHeight(x, z);

		float h = MathHelper::RandF(15.f, 30.f);
		y += h * 0.4f;

		v[i].Pos = XMFLOAT3(x, y, z);
		v[i].Size = XMFLOAT2(h, h);
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::TreePointSprite) * TREE_COUNT;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vInitData;
	vInitData.pSysMem = v;
	HR(md3dDevice->CreateBuffer(&vbd, &vInitData, &mTreeSpriteVB));
}

void BlendApp::DrawTreeSprites(CXMMATRIX viewProj)
{
	Effects::TreeSpriteFX->SetViewProj(viewProj);
	Effects::TreeSpriteFX->SetMaterial(mTreeMat);
	Effects::TreeSpriteFX->SetTreeTextureMapArray(mTreeTextureMapArraySRV);

	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	md3dImmediateContext->IASetInputLayout(InputLayouts::TreePointSprite);
	UINT stride = sizeof(Vertex::TreePointSprite);
	UINT offset = 0;
	float blendFactor[4] = { 0.f,0.f,0.f,0.f };
	ID3DX11EffectTechnique* treeTech;
	switch (mRenderOptions)
	{
	case Lighting:
		treeTech = Effects::TreeSpriteFX->Light3Tech;
		break;
	case Textures:
		treeTech = Effects::TreeSpriteFX->Light3TexAlphaClipTech;
		break;
	case TexturesAndFog:
		treeTech = Effects::TreeSpriteFX->Light3TexAlphaClipFogTech;
		break;
	}

	D3DX11_TECHNIQUE_DESC techDesc;
	treeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = treeTech->GetPassByIndex(p);

		md3dImmediateContext->IASetVertexBuffers(0, 1, &mTreeSpriteVB, &stride, &offset);
		if (mAlphaToCoverageOn)
		{
			md3dImmediateContext->OMSetBlendState(RenderStates::AlphaToCoverageBS, blendFactor, 0xffffffff);
		}
		else
		{
			md3dImmediateContext->OMSetBlendState(RenderStates::TransparentBS, blendFactor, 0xffffffff);
		}
		pass->Apply(0, md3dImmediateContext);
		md3dImmediateContext->Draw(TREE_COUNT, 0);
		md3dImmediateContext->OMSetBlendState(NULL, blendFactor, 0xffffffff);
	}
}
