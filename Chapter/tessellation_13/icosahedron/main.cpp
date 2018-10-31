#include "d3dApp.h"
#include "Waves.h"
#include "Effects.h"
#include "Vertex.h"
#include "RenderStates.h"
#include "DDSTextureLoader.h"
#include "GeometryGenerator.h"

class BasicTessellating :public D3DApp
{
public:
	BasicTessellating(HINSTANCE hInstance);
	~BasicTessellating();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildIcosahedronBuffer();
private:
	ID3D11Buffer* mIcosahedronVB;
	ID3D11Buffer* mIcosahedronIB;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	XMFLOAT3 mEyePosW;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	BasicTessellating theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

BasicTessellating::BasicTessellating(HINSTANCE hInstance)
	:D3DApp(hInstance)
	,mIcosahedronVB(NULL)
	,mIcosahedronIB(NULL)
	,mEyePosW(0.f,0.f,0.f)
	,mTheta(1.3f*MathHelper::Pi)
	,mPhi(0.4f*MathHelper::Pi)
	,mRadius(80.f)
{
	mMainWndCaption = L"Blend Demo";
	mEnable4xMsaa = false;

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);
}

BasicTessellating::~BasicTessellating()
{
	md3dImmediateContext->ClearState();
	ReleaseCOM(mIcosahedronVB);
	ReleaseCOM(mIcosahedronIB);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool BasicTessellating::Init()
{
	if (!D3DApp::Init())
	{
		return false;
	}

	Effects::InitAll(md3dDevice);
	InputLayouts::InitAll(md3dDevice);
	RenderStates::InitAll(md3dDevice);

	BuildIcosahedronBuffer();

	return true;
}

void BasicTessellating::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void BasicTessellating::UpdateScene(float dt)
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
}

void BasicTessellating::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	md3dImmediateContext->IASetInputLayout(InputLayouts::Pos);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

	UINT stride = sizeof(Vertex::Pos);
	UINT offset = 0;

	Effects::IcosahedronTessellationFX->SetEyePosW(mEyePosW);
	Effects::IcosahedronTessellationFX->SetFogColor(Colors::Silver);
	Effects::IcosahedronTessellationFX->SetFogStart(15.f);
	Effects::IcosahedronTessellationFX->SetFogRange(175.f);

	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX viewProj = view*proj;

	D3DX11_TECHNIQUE_DESC techDesc;
	Effects::IcosahedronTessellationFX->TessTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = Effects::IcosahedronTessellationFX->TessTech->GetPassByIndex(p);

		md3dImmediateContext->IASetVertexBuffers(0, 1, &mIcosahedronVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mIcosahedronIB, DXGI_FORMAT_R32_UINT, 0);
		XMMATRIX world = XMMatrixScaling(10.f,15.f,10.f);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world*view*proj;

		Effects::IcosahedronTessellationFX->SetWorld(world);
		Effects::IcosahedronTessellationFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::IcosahedronTessellationFX->SetWorldViewProj(worldViewProj);
		Effects::IcosahedronTessellationFX->SetTexTransform(XMMatrixIdentity());
		//Effects::TessellationFX->SetMaterial(NULL);
		Effects::IcosahedronTessellationFX->SetDiffuseMap(NULL);

		pass->Apply(0, md3dImmediateContext);

		md3dImmediateContext->RSSetState(RenderStates::WireframesRS);
		md3dImmediateContext->DrawIndexed(60,0,0);
	}

	HR(mSwapChain->Present(0, 0));
}

void BasicTessellating::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void BasicTessellating::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void BasicTessellating::OnMouseMove(WPARAM btnState, int x, int y)
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

void BasicTessellating::BuildIcosahedronBuffer()
{
	GeometryGenerator::MeshData data;
	GeometryGenerator geoGen;
	geoGen.CreateIcosahedron(1.f, data);

	std::vector<XMFLOAT3> vertices;

	for (UINT i = 0; i < data.Vertices.size(); ++i)
	{
		vertices.push_back(data.Vertices[i].Position);
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(XMFLOAT3) * vertices.size();
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mIcosahedronVB));

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * data.Indices.size();
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iInitData;
	iInitData.pSysMem = &data.Indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iInitData, &mIcosahedronIB));
}
