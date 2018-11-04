#include "d3dApp.h"
#include "Waves.h"
#include "Effects.h"
#include "Vertex.h"
#include "RenderStates.h"
#include "DDSTextureLoader.h"
#include "GeometryGenerator.h"
#include "Camera.h"
#include "xnacollision.h"
#include "Sky.h"
#include "Terrain.h"

class TerrainApp :public D3DApp
{
public:
	TerrainApp(HINSTANCE hInstance);
	~TerrainApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);
private:
	Sky* mSky;

	Terrain mTerrain;

	DirectionalLight mDirLights[3];

	bool mWalkCamMode;

	float mPhi;

	POINT mLastMousePos;
	Camera* mCamera;
	float mMoveSpeed;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	TerrainApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

TerrainApp::TerrainApp(HINSTANCE hInstance)
	:D3DApp(hInstance)
	, mSky(NULL)
	, mPhi(0.4f*MathHelper::Pi)
	, mCamera(NULL)
	, mMoveSpeed(10.f)
	, mWalkCamMode(true)
{
	mMainWndCaption = L"Terrain Demo";
	mEnable4xMsaa = false;

	mCamera = new Camera();
	mCamera->LookAt(XMFLOAT3(0.f, 2.f, -15.f), XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT3(0.f, 1.f, 0.f));

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

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
}

TerrainApp::~TerrainApp()
{
	md3dImmediateContext->ClearState();
	SafeDelete(mSky);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();

	SafeDelete(mCamera);
}

bool TerrainApp::Init()
{
	if (!D3DApp::Init())
	{
		return false;
	}

	Effects::InitAll(md3dDevice);
	InputLayouts::InitAll(md3dDevice);
	RenderStates::InitAll(md3dDevice);

	mSky = new Sky(md3dDevice, L"Textures/grasscube1024.dds", 5000.0f);

	Terrain::InitInfo tii;
	tii.HeightMapFileName = L"Textures/terrain.raw";
	tii.LayerMapFileNames[0] = L"Textures/grass.dds";
	tii.LayerMapFileNames[1] = L"Textures/darkdirt.dds";
	tii.LayerMapFileNames[2] = L"Textures/stone.dds";
	tii.LayerMapFileNames[3] = L"Textures/lightdirt.dds";
	tii.LayerMapFileNames[4] = L"Textures/snow.dds";
	tii.BlendMapFileName = L"Textures/blend.dds";
	tii.HeightScale = 50.0f;
	tii.HeightMapWidth = 2049;
	tii.HeightMapHeight = 2049;
	tii.CellSpacing = 0.5f;

	mTerrain.Init(md3dDevice, md3dImmediateContext, tii);
	return true;
}

void TerrainApp::OnResize()
{
	D3DApp::OnResize();

	mCamera->SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);

}

void TerrainApp::UpdateScene(float dt)
{
	
	if (GetAsyncKeyState('W') & 0x8000)
	{
		mCamera->Walk(mMoveSpeed*dt);
	}
	if (GetAsyncKeyState('S') & 0x8000)
	{
		mCamera->Walk(-mMoveSpeed*dt);
	}
	if (GetAsyncKeyState('A') & 0x8000)
	{
		mCamera->Strafe(-mMoveSpeed*dt);
	}
	if (GetAsyncKeyState('D') & 0x8000)
	{
		mCamera->Strafe(mMoveSpeed*dt);
	}

	if (GetAsyncKeyState('2') & 0x8000)
		mWalkCamMode = true;
	if (GetAsyncKeyState('3') & 0x8000)
		mWalkCamMode = false;

	if (mWalkCamMode)
	{
		XMFLOAT3 camPos = mCamera->GetPosition();
		float y = mTerrain.GetHeight(camPos.x, camPos.z);
		mCamera->SetPosition(camPos.x, y + 2.0f, camPos.z);
	}
	mCamera->UpdateViewMatrix();
}

void TerrainApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	if (GetAsyncKeyState('1') & 0x8000)
		md3dImmediateContext->RSSetState(RenderStates::WireframesRS);

	mTerrain.Draw(md3dImmediateContext, *mCamera, mDirLights);

	md3dImmediateContext->RSSetState(0);

	mSky->Draw(md3dImmediateContext, *mCamera);

	// restore default states, as the SkyFX changes them in the effect file.
	md3dImmediateContext->RSSetState(0);
	md3dImmediateContext->OMSetDepthStencilState(0, 0);

	HR(mSwapChain->Present(0, 0));
}

void TerrainApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void TerrainApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void TerrainApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
		float newPhi = MathHelper::Clamp(mPhi + dy, 0.1f, MathHelper::Pi - 0.1f);
		dy = newPhi - mPhi;

		mPhi = newPhi;

		mCamera->Pitch(dy);
		mCamera->RotateY(dx);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}
