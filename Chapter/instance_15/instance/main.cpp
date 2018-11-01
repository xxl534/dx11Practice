#include "d3dApp.h"
#include "Waves.h"
#include "Effects.h"
#include "Vertex.h"
#include "RenderStates.h"
#include "DDSTextureLoader.h"
#include "GeometryGenerator.h"
#include "Camera.h"
#include "xnacollision.h"

struct InstancedData
{
	XMFLOAT4X4 World;
	XMFLOAT4X4 WorldInvTranspose;
	XMFLOAT4 Color;
};
class InstanceApp :public D3DApp
{
public:
	InstanceApp(HINSTANCE hInstance);
	~InstanceApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildSkullGeometryBuffers();
	void BuildInstanceBuffer();
private:

	ID3D11Buffer* mSkullVB;
	ID3D11Buffer* mSkullIB;
	ID3D11Buffer* mInstancedBuffer;

	XNA::AxisAlignedBox mSkullBox;
	XNA::Frustum mCamFrustum;

	UINT mVisibleObjectCount;
	std::vector<InstancedData> mInstancedData;

	DirectionalLight mDirLights[3];
	Material mSkullMat;

	XMFLOAT4X4 mSkullWorld;

	UINT mSkullIndexCount;

	XMFLOAT2 mWaterTexOffset;

	float mPhi;

	POINT mLastMousePos;
	Camera* mCamera;
	float mMoveSpeed;

	bool mFrustumCullingEnabled;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	InstanceApp theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

InstanceApp::InstanceApp(HINSTANCE hInstance)
	:D3DApp(hInstance)
	,mSkullVB(NULL)
	,mSkullIB(NULL)
	,mInstancedBuffer(NULL)
	,mWaterTexOffset(0.f,0.f)
	,mSkullIndexCount(0)
	,mPhi(0.4f*MathHelper::Pi)
	,mCamera(NULL)
	,mMoveSpeed(100.f)
	, mFrustumCullingEnabled(true)
{
	mMainWndCaption = L"Instance Demo";
	mEnable4xMsaa = false;

	mCamera = new Camera();
	mCamera->LookAt(XMFLOAT3(50.f, 30.f, -50.f), XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT3(0.f, 1.f, 0.f));

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;


	XMMATRIX skullScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
	XMMATRIX skullOffset = XMMatrixTranslation(0.f, 1.f, 0.f);
	XMStoreFloat4x4(&mSkullWorld, skullScale*skullOffset);

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

	mSkullMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mSkullMat.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mSkullMat.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);
}

InstanceApp::~InstanceApp()
{
	md3dImmediateContext->ClearState();
	ReleaseCOM(mSkullVB);
	ReleaseCOM(mSkullIB);
	ReleaseCOM(mInstancedBuffer);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();

	SafeDelete(mCamera);
}

bool InstanceApp::Init()
{
	if (!D3DApp::Init())
	{
		return false;
	}

	Effects::InitAll(md3dDevice);
	InputLayouts::InitAll(md3dDevice);
	RenderStates::InitAll(md3dDevice);

	BuildSkullGeometryBuffers();
	BuildInstanceBuffer();

	return true;
}

void InstanceApp::OnResize()
{
	D3DApp::OnResize();

	mCamera->SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);

	ComputeFrustumFromProjection(&mCamFrustum, &mCamera->Proj());
}

void InstanceApp::UpdateScene(float dt)
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

	if (GetAsyncKeyState('1') & 0x8000)
		mFrustumCullingEnabled = true;

	if (GetAsyncKeyState('2') & 0x8000)
		mFrustumCullingEnabled = false;


	mCamera->UpdateViewMatrix();

	mVisibleObjectCount = 0;
	if (mFrustumCullingEnabled)
	{
		XMVECTOR detView = XMMatrixDeterminant(mCamera->View());
		XMMATRIX invView = XMMatrixInverse(&detView, mCamera->View());

		D3D11_MAPPED_SUBRESOURCE mappedData;
		md3dImmediateContext->Map(mInstancedBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);

		InstancedData* dataView = reinterpret_cast<InstancedData*>(mappedData.pData);

		for (UINT i = 0; i < mInstancedData.size(); ++i)
		{

			XMMATRIX invWorld = MathHelper::InverseMatrix( XMLoadFloat4x4(&mInstancedData[i].World));

			XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);

			XMVECTOR scale;
			XMVECTOR rotQuat;
			XMVECTOR translation;
			XMMatrixDecompose(&scale, &rotQuat, &translation, toLocal);
			
			float scaleF = MathHelper::Min(MathHelper::Min(abs(XMVectorGetX(scale)), abs(XMVectorGetY(scale))), abs(XMVectorGetZ(scale)));
			XNA::Frustum localspaceFrustum;
			XNA::TransformFrustum(&localspaceFrustum, &mCamFrustum, scaleF, rotQuat, translation);

			if (XNA::IntersectAxisAlignedBoxFrustum(&mSkullBox, &localspaceFrustum))
			{
				dataView[mVisibleObjectCount++] = mInstancedData[i];
			}
		}
		md3dImmediateContext->Unmap(mInstancedBuffer,0);
	}
	else
	{
		D3D11_MAPPED_SUBRESOURCE mappedData;
		md3dImmediateContext->Map(mInstancedBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);

		InstancedData* dataView = reinterpret_cast<InstancedData*>(mappedData.pData);

		for (UINT i = 0; i < mInstancedData.size(); ++i)
		{
			dataView[mVisibleObjectCount++] = mInstancedData[i];
		}
		md3dImmediateContext->Unmap(mInstancedBuffer, 0);
	}

	std::wostringstream outs;
	outs.precision(6);
	outs << L"Instancing and Culling Demo" <<
		L"    " << mVisibleObjectCount <<
		L" objects visible out of " << mInstancedData.size();
	mMainWndCaption = outs.str();
}

void InstanceApp::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	md3dImmediateContext->IASetInputLayout(InputLayouts::InstancedBasic32);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride[2] = { sizeof(Vertex::Basic32), sizeof(InstancedData) };
	UINT offset[2] = { 0,0 };

	XMMATRIX view = mCamera->View();
	XMMATRIX proj = mCamera->Proj();
	XMMATRIX viewProj = mCamera->ViewProj();
	XMFLOAT3 eyePos = mCamera->GetPosition();

	Effects::InstancedBasicFX->SetDirLights(mDirLights);
	Effects::InstancedBasicFX->SetEyePosW(eyePos);
	Effects::InstancedBasicFX->SetFogColor(Colors::Silver);
	Effects::InstancedBasicFX->SetFogStart(15.f);
	Effects::InstancedBasicFX->SetFogRange(175.f);

	
	D3DX11_TECHNIQUE_DESC techDesc;
	ID3DX11EffectTechnique* tech = Effects::InstancedBasicFX->Light3Tech;
	tech->GetDesc(&techDesc);

	ID3D11Buffer* vbs[2] = { mSkullVB, mInstancedBuffer };
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = tech->GetPassByIndex(p);

		md3dImmediateContext->IASetVertexBuffers(0, 2, vbs, stride, offset);
		md3dImmediateContext->IASetIndexBuffer(mSkullIB, DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX world = XMLoadFloat4x4(&mSkullWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;

		Effects::InstancedBasicFX->SetWorld(world);
		Effects::InstancedBasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::InstancedBasicFX->SetViewProj(viewProj);
		Effects::InstancedBasicFX->SetMaterial(mSkullMat);

		pass->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexedInstanced(mSkullIndexCount, mVisibleObjectCount, 0, 0, 0);

	}
	HR(mSwapChain->Present(0, 0));
}

void InstanceApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void InstanceApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void InstanceApp::OnMouseMove(WPARAM btnState, int x, int y)
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

void InstanceApp::BuildSkullGeometryBuffers()
{
	std::ifstream fin("Models/skull.txt");

	if (!fin)
	{
		MessageBox(0, L"Models/skull.txt not found.", 0, 0);
		return;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	XMFLOAT3 vMinf3(+MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity);
	XMFLOAT3 vMaxf3(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);

	XMVECTOR vMin = XMLoadFloat3(&vMinf3);
	XMVECTOR vMax = XMLoadFloat3(&vMaxf3);
	std::vector<Vertex::Basic32> vertices(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;

		XMVECTOR P = XMLoadFloat3(&vertices[i].Pos);

		vMin = XMVectorMin(vMin, P);
		vMax = XMVectorMax(vMax, P);
	}

	XMStoreFloat3(&mSkullBox.Center, 0.5f*(vMin + vMax));
	XMStoreFloat3(&mSkullBox.Extents, 0.5f*(vMax - vMin));

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	mSkullIndexCount = 3 * tcount;
	std::vector<UINT> indices(mSkullIndexCount);
	for (UINT i = 0; i < tcount; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}

	fin.close();

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * vcount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mSkullVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * mSkullIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mSkullIB));
}

void InstanceApp::BuildInstanceBuffer()
{
	const int n = 5;
	mInstancedData.resize(n*n*n);

	float width = 200.0f;
	float height = 200.0f;
	float depth = 200.0f;

	float x = -0.5f*width;
	float y = -0.5f*height;
	float z = -0.5f*depth;
	float dx = width / (n - 1);
	float dy = height / (n - 1);
	float dz = depth / (n - 1);
	for (int k = 0; k < n; ++k)
	{
		for (int i = 0; i < n; ++i)
		{
			for (int j = 0; j < n; ++j)
			{
				// Position instanced along a 3D grid.
				UINT idx = k * n * n + i *n + j;

				mInstancedData[idx].World = XMFLOAT4X4(
					1.0f, 0.0f, 0.0f, 0.0f,
					0.0f, 1.0f, 0.0f, 0.0f,
					0.0f, 0.0f, 1.0f, 0.0f,
					x + j*dx, y + i*dy, z + k*dz, 1.0f);

				XMStoreFloat4x4(&mInstancedData[idx].WorldInvTranspose, MathHelper::InverseTranspose(XMLoadFloat4x4(&mInstancedData[idx].World)));
				// Random color.
				mInstancedData[idx].Color.x = MathHelper::RandF(0.0f, 1.0f);
				mInstancedData[idx].Color.y = MathHelper::RandF(0.0f, 1.0f);
				mInstancedData[idx].Color.z = MathHelper::RandF(0.0f, 1.0f);
				mInstancedData[idx].Color.w = 1.0f;
			}
		}
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.ByteWidth = sizeof(InstancedData) * mInstancedData.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	HR(md3dDevice->CreateBuffer(&vbd, 0, &mInstancedBuffer));
}