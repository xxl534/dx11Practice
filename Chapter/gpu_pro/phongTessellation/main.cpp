#include "d3dApp.h"
#include "Waves.h"
#include "Effect/Effects.h"
#include "RenderStates.h"
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"
#include "GeometryGenerator.h"
#include "Model/SkinnedModel.h"
#include "Camera.h"

class PhongTessellationApp :public D3DApp
{
public:
	PhongTessellationApp(HINSTANCE hInstance);
	~PhongTessellationApp();

	bool Init();
	void DrawScene();
	void OnResize();
	virtual void UpdateScene(float dt);

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);
private:
	TextureMgr mTexMgr;

	SkinnedModel* mSkinnedModel;
	SkinnedModelInstance* mSkinnedModelInst;

	DirectionalLight mDirLight;

	XMFLOAT4X4 mWorld2;
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

	PhongTessellationApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

PhongTessellationApp::PhongTessellationApp(HINSTANCE hInstance)
	:D3DApp(hInstance)
	,mSkinnedModel(NULL)
	,mSkinnedModelInst(NULL)
	,mMoveSpeed(3)
	,mPhi(0.5f*MathHelper::Pi)
	,mCamera(NULL)
{
	mMainWndCaption = L"Phong Tessellation";

	mCamera = new Camera();
	mCamera->LookAt(XMFLOAT3(0.f, 0.f, -7.f), XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT3(0.f, 1.f, 0.f));

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	mEnable4xMsaa = false;

	mDirLight.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLight.Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLight.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLight.Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);
}

PhongTessellationApp::~PhongTessellationApp()
{
	md3dImmediateContext->ClearState();

	SafeDelete(mCamera);

	SafeDelete(mSkinnedModelInst);

	SafeDelete(mSkinnedModel);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool PhongTessellationApp::Init()
{
	if (!D3DApp::Init())
	{
		return false;
	}

	Effects::InitAll(md3dDevice);
	InputLayouts::InitAll(md3dDevice);
	RenderStates::InitAll(md3dDevice);

	mTexMgr.Init(md3dDevice);

	mSkinnedModel = new SkinnedModel(md3dDevice, mTexMgr, "Models/Soldier.m3d", L"Textures/");
	mSkinnedModelInst = new SkinnedModelInstance();
	mSkinnedModelInst->Model = mSkinnedModel;
	mSkinnedModelInst->TimePos = 0.f;
	mSkinnedModelInst->ClipName = "Take1";
	mSkinnedModelInst->FinalTransforms.resize(mSkinnedModel->SkinnedData.BoneCount());
	mSkinnedModelInst->Update(0);
	XMMATRIX modelScale = XMMatrixScaling(0.25f, 0.25f, -0.25f);
	XMMATRIX modelRot = XMMatrixRotationY(MathHelper::Pi);
	XMMATRIX modelOffset = XMMatrixTranslation(-5.f, 0.0f, 0.0f);
	XMStoreFloat4x4(&mSkinnedModelInst->World, modelScale*modelRot*modelOffset);

	modelOffset = XMMatrixTranslation(5.f, 0.f, 0.f);
	XMStoreFloat4x4(&mWorld2, modelScale*modelRot*modelOffset);
	return true;
}

void PhongTessellationApp::DrawScene()
{
	ID3D11RenderTargetView* renderTargets[1] = { mRenderTargetView };
	md3dImmediateContext->OMSetRenderTargets(1, renderTargets, mDepthStencilView);

	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	md3dImmediateContext->IASetInputLayout(InputLayouts::Skinned);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

	XMMATRIX view = mCamera->View();
	XMMATRIX proj = mCamera->Proj();
	XMMATRIX viewProj = mCamera->ViewProj();
	XMFLOAT3 eyePos = mCamera->GetPosition();

	Effects::PhongTessSkinnedFX->SetEyePosW(eyePos);
	Effects::PhongTessSkinnedFX->SetMaxTessDistance(1.0f);
	Effects::PhongTessSkinnedFX->SetMinTessDistance(15.0f);
	Effects::PhongTessSkinnedFX->SetMinTessFactor(1.0f);
	Effects::PhongTessSkinnedFX->SetMaxTessFactor(5.0f);
	Effects::PhongTessSkinnedFX->SetDirLights(mDirLight);

	Effects::BasicSkinnedFX->SetEyePosW(eyePos);
	Effects::BasicSkinnedFX->SetDirLights(mDirLight);

	ID3DX11EffectTechnique* tech = Effects::PhongTessSkinnedFX->TriangleTexTech;
	D3DX11_TECHNIQUE_DESC techDesc;
	tech->GetDesc(&techDesc);

	md3dImmediateContext->RSSetState(RenderStates::CullBackfaceRS);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pPass = tech->GetPassByIndex(p);
		XMMATRIX world = XMLoadFloat4x4(&mSkinnedModelInst->World);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;

		Effects::PhongTessSkinnedFX->SetWorld(world);
		Effects::PhongTessSkinnedFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::PhongTessSkinnedFX->SetWorldViewProj(worldViewProj);
		Effects::PhongTessSkinnedFX->SetViewProj(view*proj);
		Effects::PhongTessSkinnedFX->SetTexTransform(XMMatrixScaling(1.0f, 1.0f, 1.0f));
		Effects::PhongTessSkinnedFX->SetBoneTransforms(
			&mSkinnedModelInst->FinalTransforms[0],
			mSkinnedModelInst->FinalTransforms.size());
		for (UINT subset = 0; subset < mSkinnedModelInst->Model->SubsetCount; ++subset)
		{
			Effects::PhongTessSkinnedFX->SetMaterial(mSkinnedModelInst->Model->Mat[subset]);
			Effects::PhongTessSkinnedFX->SetDiffuseMap(mSkinnedModelInst->Model->DiffuseMapSRV[subset]);
			Effects::PhongTessSkinnedFX->SetNormalMap(mSkinnedModelInst->Model->NormalMapSRV[subset]);

			pPass->Apply(0, md3dImmediateContext);
			mSkinnedModelInst->Model->ModelMesh.Draw(md3dImmediateContext, subset);
		}
	}

	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	tech = Effects::BasicSkinnedFX->TexTech;
	tech->GetDesc(&techDesc);
	md3dImmediateContext->RSSetState(RenderStates::CullBackfaceRS);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pPass = tech->GetPassByIndex(p);
		XMMATRIX world = XMLoadFloat4x4(&mWorld2);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;

		Effects::BasicSkinnedFX->SetWorld(world);
		Effects::BasicSkinnedFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicSkinnedFX->SetWorldViewProj(worldViewProj);
		Effects::BasicSkinnedFX->SetTexTransform(XMMatrixScaling(1.0f, 1.0f, 1.0f));
		Effects::BasicSkinnedFX->SetBoneTransforms(
			&mSkinnedModelInst->FinalTransforms[0],
			mSkinnedModelInst->FinalTransforms.size());
		for (UINT subset = 0; subset < mSkinnedModelInst->Model->SubsetCount; ++subset)
		{
			Effects::BasicSkinnedFX->SetMaterial(mSkinnedModelInst->Model->Mat[subset]);
			Effects::BasicSkinnedFX->SetDiffuseMap(mSkinnedModelInst->Model->DiffuseMapSRV[subset]);
			Effects::BasicSkinnedFX->SetNormalMap(mSkinnedModelInst->Model->NormalMapSRV[subset]);

			pPass->Apply(0, md3dImmediateContext);
			mSkinnedModelInst->Model->ModelMesh.Draw(md3dImmediateContext, subset);
		}
	}


	HR(mSwapChain->Present(0, 0));
}

void PhongTessellationApp::OnResize()
{
	D3DApp::OnResize();

	mCamera->SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
}

void PhongTessellationApp::UpdateScene(float dt)
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
	//mSkinnedModelInst->Update(dt);

	mCamera->UpdateViewMatrix();
}

void PhongTessellationApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void PhongTessellationApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void PhongTessellationApp::OnMouseMove(WPARAM btnState, int x, int y)
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
